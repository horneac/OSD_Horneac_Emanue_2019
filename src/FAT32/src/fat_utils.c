#include "fat32_base.h"
#include "fat_operations.h"
#include "fat_utils.h"

static
STATUS
_ConvertFatDateToDate(
    IN      FATDATE*    FatDate,
    OUT     DATE*       RealDate
    );

static
STATUS
_ConvertFatTimeToTime(
    IN      FATTIME*    FatTime,
    OUT     TIME*       RealTime
    );

static
void
_ConvertDateToFatDate(
    IN      DATE*       RealDate,
    OUT     FATDATE*    FatDate
    );

static
void
_ConvertTimeToFatTime(
    IN      TIME*       RealTime,
    OUT     FATTIME*    FatTime
    );

static
STATUS
_AddNewClusterToChain(
    IN      PFAT_DATA       FatData,
    IN      QWORD           LastClusterFromChain,
    OUT     QWORD*          ReservedCluster
);

STATUS
NextSectorInClusterChain(
    IN      PFAT_DATA       FatData,
    IN      QWORD           CurrentSector,
    OUT     QWORD*          NextSector,
    IN      BOOLEAN         ExtendChain
    )
{
    STATUS status;
    QWORD currentCluster;
    QWORD nextCluster;

    ASSERT(NULL != FatData);
    ASSERT(NULL != NextSector);

    status = STATUS_SUCCESS;
    currentCluster = 0;
    nextCluster = 0;

    // we get the cluster of the current sector
    status = ClusterOfSector(FatData, CurrentSector, &currentCluster);
    if (!SUCCEEDED(status))
    {
        LOG_FUNC_ERROR("ClusterOfSector", status);
        return status;
    }

    // we get the cluster following the current one
    status = NextClusterInChain(FatData, currentCluster, &nextCluster);
    if (!SUCCEEDED(status))
    {
        LOG_FUNC_ERROR("NextClusterInChain", status);
        return status;
    }

    // we check if we reached the end of chain
    if (FAT32_EOC(nextCluster))
    {
        if (!ExtendChain)
        {
        // arrived to the end of the cluster chain
        *NextSector = 0;
        return status;
    }

        status = _AddNewClusterToChain(FatData, currentCluster, &nextCluster);
        if (!SUCCEEDED(status))
        {
            LOG_FUNC_ERROR("AddNewClusterToChain", status);
            return status;
        }
    }

    // we get the sector belonging to the found cluster
    status = FirstSectorOfCluster(FatData, nextCluster, NextSector);
    if (!SUCCEEDED(status))
    {
        LOG_FUNC_ERROR("FirstSectorOfCluster", status);
        return status;
    }

    return status;
}

STATUS
NextClusterInChain(
    IN      PFAT_DATA       FatData,
    IN      QWORD           CurrentCluster,
    OUT     QWORD*          NextCluster
    )
{
    STATUS status;
    FAT32_ENTRY* pFat32Entries;        // pointer to a FAT table
    QWORD sectorNumber;
    QWORD indexInSector;
    QWORD bytesToReadWrite;
    QWORD offset;
    QWORD nextCluster;

    ASSERT(NULL != FatData);
    ASSERT(NULL != NextCluster);

    status = STATUS_UNSUCCESSFUL;
    pFat32Entries = NULL;
    nextCluster = 0;

    sectorNumber = FatData->ReservedSectors + (CurrentCluster * (sizeof(FAT32_ENTRY)) / FatData->BytesPerSector);
    indexInSector = CurrentCluster % (FatData->BytesPerSector / sizeof(FAT32_ENTRY));
    bytesToReadWrite = FatData->BytesPerSector;
    offset = sectorNumber * FatData->BytesPerSector;

    __try
    {
        pFat32Entries = (FAT32_ENTRY*)ExAllocatePoolWithTag(PoolAllocateZeroMemory, FatData->BytesPerSector, HEAP_TEMP_TAG, 0);
    if (NULL == pFat32Entries)
    {
            status = STATUS_HEAP_NO_MORE_MEMORY;
        // could not allocate memory for a FAT32 sector
            LOG_FUNC_ERROR_ALLOC("ExAllocatePoolWithTag", FatData->BytesPerSector);
            __leave;
    }

    // We read the sector
        status = IoReadDevice(
            FatData->VolumeDevice,
                          pFat32Entries,
            &bytesToReadWrite,
                          offset
                          );
    if (!SUCCEEDED(status))
    {
        LOG_FUNC_ERROR("IoReadDevice", status);
            __leave;
        }
        ASSERT(bytesToReadWrite == FatData->BytesPerSector);

        nextCluster = pFat32Entries[indexInSector] & FAT32_CLUSTER_MASK;

        if (0 == nextCluster)
        {
            LOG_TRACE_FILESYSTEM("Found zero in cluster chain");
            // has to be treated as EOC marker
            nextCluster = FAT32_EOC_MARK;

            // write EOC marker back to disk, such that the cluster is not treated as a free one
            pFat32Entries[indexInSector] &= ~FAT32_CLUSTER_MASK;    // we need to preserve the 4 reserved bits
            pFat32Entries[indexInSector] |= FAT32_EOC_MARK;            // we add the EOC mark

            status = IoWriteDevice(
                FatData->VolumeDevice,
                pFat32Entries,
                &bytesToReadWrite,
                offset
            );
            if (!SUCCEEDED(status))
            {
                LOG_FUNC_ERROR("IoWriteDevice", status);
                __leave;
            }
            ASSERT(bytesToReadWrite == FatData->BytesPerSector);
        }
        else if (FAT32_BAD_CLUSTER == nextCluster)
        {
            // maybe we should cut off the cluster chain, so it doesn't reach the bad cluster
            nextCluster = FAT32_EOC_MARK;
        }

        status = STATUS_SUCCESS;
    }
    __finally
    {
        *NextCluster = nextCluster;

        // We free the memory allocated
        if (NULL != pFat32Entries)
        {
            ExFreePoolWithTag(pFat32Entries, HEAP_TEMP_TAG);
            pFat32Entries = NULL;
        }
    }

        return status;
    }

static
STATUS
_FirstFreeCluster(
    IN      PFAT_DATA       FatData,
    IN      QWORD           FirstCheckedCluster,
    IN      BOOLEAN         MarkReserved,
    OUT     QWORD*          FreeCluster
)
{
    STATUS status = STATUS_UNSUCCESSFUL;
    FAT32_ENTRY* pFat32Entries = NULL;        // pointer to a FAT table
    QWORD firstCheckedCluster = FirstCheckedCluster;
    QWORD entriesPerFat = 0;
    QWORD sectorsPerFat = 0;
    QWORD firstFatSector = 0;
    QWORD currentSector = 0;
    QWORD bytesToReadWrite = 0;
    QWORD indexInSector = 0;
    QWORD clusterIndex = 0;

    __try
    {
        pFat32Entries = (FAT32_ENTRY*)ExAllocatePoolWithTag(PoolAllocateZeroMemory, FatData->BytesPerSector, HEAP_TEMP_TAG, 0);
        if (NULL == pFat32Entries)
        {
            status = STATUS_HEAP_NO_MORE_MEMORY;
            // could not allocate memory for a FAT32 sector
            LOG_FUNC_ERROR_ALLOC("ExAllocatePoolWithTag", FatData->BytesPerSector);
            __leave;
        }

        bytesToReadWrite = FatData->BytesPerSector; // read the FAT sector by sector

        firstFatSector = FatData->ReservedSectors;
        entriesPerFat = FatData->CountOfClusters + 2;
        sectorsPerFat = entriesPerFat * sizeof(FAT32_ENTRY) / FatData->BytesPerSector;

        // if search should begin after the last cluster
        if (entriesPerFat <= firstCheckedCluster)
        {
            status = STATUS_SUCCESS;
            __leave;
        }

        if (FAT32_UNKNOWN == firstCheckedCluster)
        {
            firstCheckedCluster = 2;
        }

        currentSector = firstFatSector + (firstCheckedCluster * sizeof(FAT32_ENTRY) / FatData->BytesPerSector);
        indexInSector = firstCheckedCluster % (FatData->BytesPerSector / sizeof(FAT32_ENTRY));
        // for each sector of the FAT
        for (; currentSector < sectorsPerFat; currentSector++)
        {
            // We read the sector
            status = IoReadDevice(
                FatData->VolumeDevice,
                pFat32Entries,
                &bytesToReadWrite,
                currentSector * FatData->BytesPerSector
            );
            if (!SUCCEEDED(status))
            {
                LOG_FUNC_ERROR("IoReadDevice", status);
                __leave;
            }
            ASSERT(bytesToReadWrite == FatData->BytesPerSector);

            // check all the entries in the current sector of the FAT
            for (; indexInSector < FatData->BytesPerSector / sizeof(FAT32_ENTRY); indexInSector++)
            {
                if (0 == (pFat32Entries[indexInSector] & FAT32_CLUSTER_MASK)) // found free cluster
                {
                    if (MarkReserved)
                    {
                        // write EOC marker back to disk, such that the cluster is reserved
                        pFat32Entries[indexInSector] &= ~FAT32_CLUSTER_MASK;    // we need to preserve the 4 reserved bits
                        pFat32Entries[indexInSector] |= FAT32_EOC_MARK;         // we add the EOC mark

                        status = IoWriteDevice(
                            FatData->VolumeDevice,
                            pFat32Entries,
                            &bytesToReadWrite,
                            currentSector * FatData->BytesPerSector
                        );
                        if (!SUCCEEDED(status))
                        {
                            LOG_FUNC_ERROR("IoWriteDevice", status);
                            __leave;
                        }
                        ASSERT(bytesToReadWrite == FatData->BytesPerSector);
                    }

                    // clusterIndex = Index of sector inside FAT * Nr of FAT entryies per sector + FAT entry index in current sector
                    clusterIndex = (currentSector - firstFatSector) * (FatData->BytesPerSector / sizeof(FAT32_ENTRY)) + indexInSector;

                    ASSERT(2 <= clusterIndex && clusterIndex < entriesPerFat);
                    ASSERT(FAT32_UNKNOWN != clusterIndex);

                    currentSector = sectorsPerFat; // break outer for loop
                    break;
                }
            }
            // reset entry index
            indexInSector = 0;
        }
    }
    __finally
    {
        *FreeCluster = clusterIndex;

    // We free the memory allocated
        if (NULL != pFat32Entries)
        {
    ExFreePoolWithTag(pFat32Entries, HEAP_TEMP_TAG);
            pFat32Entries = NULL;
        }
    }

    return status;
}

static
STATUS
_AddNewClusterToChain(
    IN      PFAT_DATA       FatData,
    IN      QWORD           LastClusterFromChain,
    OUT     QWORD*          ReservedCluster
)
{
    ASSERT(FatData != NULL);
    ASSERT(ReservedCluster != NULL);

    ASSERT(!FAT32_EOC(LastClusterFromChain));

    STATUS status = STATUS_UNSUCCESSFUL;
    FSINFO* pFsInfo = NULL;            // pointer to FSInfo
    FAT32_ENTRY* pFat32Entries = NULL;        // pointer to a FAT table
    QWORD bytesToReadWrite = 0;
    QWORD freeCluster = 0;
    QWORD reservedCluster = 0;
    QWORD sectorFromFat = 0;
    QWORD indexInSector = 0;

    __try
    {
        pFsInfo = (FSINFO*)ExAllocatePoolWithTag(PoolAllocateZeroMemory, sizeof(FSINFO), HEAP_TEMP_TAG, 0);
        if (NULL == pFsInfo)
        {
            status = STATUS_HEAP_NO_MORE_MEMORY;
            LOG_FUNC_ERROR_ALLOC("ExAllocatePoolWithTag", sizeof(FSINFO));
            __leave;
        }

        bytesToReadWrite = sizeof(FSINFO);
        status = IoReadDevice(
            FatData->VolumeDevice,
            pFsInfo,
            &bytesToReadWrite,
            1 * FatData->BytesPerSector
        );
        if (!SUCCEEDED(status))
        {
            LOG_FUNC_ERROR("IoReadDevice", status);
            __leave;
        }
        ASSERT(bytesToReadWrite == FatData->BytesPerSector);

        // if there are no more free clusters we fail
        if (0 == pFsInfo->FSI_Free_Count)
        {
            pFsInfo->FSI_Nxt_Free = FAT32_UNKNOWN;

            status = STATUS_DISK_FULL;
            __leave;
        }

        status = _FirstFreeCluster(FatData, pFsInfo->FSI_Nxt_Free, TRUE, &reservedCluster);
        if (!SUCCEEDED(status))
        {
            LOG_FUNC_ERROR("FirstFreeCluster", status);
            __leave;
        }

        if (0 == reservedCluster) // no free cluster found
        {
            // FSI_Free_Count was not zero, but there was no free cluster
            ASSERT(FAT32_UNKNOWN == pFsInfo->FSI_Free_Count);

            pFsInfo->FSI_Free_Count = 0;
            pFsInfo->FSI_Nxt_Free = FAT32_UNKNOWN;

            status = STATUS_DISK_FULL;
            __leave;
        }

        // a free cluster was reserved
        // write it to the end of the cluster chain
        pFat32Entries = (FAT32_ENTRY*)ExAllocatePoolWithTag(PoolAllocateZeroMemory, FatData->BytesPerSector, HEAP_TEMP_TAG, 0);
        if (NULL == pFat32Entries)
        {
            status = STATUS_HEAP_NO_MORE_MEMORY;
            // could not allocate memory for a FAT32 sector
            LOG_FUNC_ERROR_ALLOC("ExAllocatePoolWithTag", FatData->BytesPerSector);
            __leave;
        }

        sectorFromFat = FatData->ReservedSectors + (LastClusterFromChain * sizeof(FAT32_ENTRY) / FatData->BytesPerSector);
        indexInSector = LastClusterFromChain % (FatData->BytesPerSector / sizeof(FAT32_ENTRY));

        bytesToReadWrite = FatData->BytesPerSector;
        // We read the sector
        status = IoReadDevice(
            FatData->VolumeDevice,
            pFat32Entries,
            &bytesToReadWrite,
            sectorFromFat * FatData->BytesPerSector
        );
        if (!SUCCEEDED(status))
        {
            LOG_FUNC_ERROR("IoReadDevice", status);
            __leave;
        }
        ASSERT(bytesToReadWrite == FatData->BytesPerSector);

        pFat32Entries[indexInSector] &= ~FAT32_CLUSTER_MASK;    // we need to preserve the 4 reserved bits
        pFat32Entries[indexInSector] |= (DWORD)reservedCluster;

        status = IoWriteDevice(
            FatData->VolumeDevice,
            pFat32Entries,
            &bytesToReadWrite,
            sectorFromFat * FatData->BytesPerSector
        );
        if (!SUCCEEDED(status))
        {
            LOG_FUNC_ERROR("IoWriteDevice", status);
            __leave;
        }
        ASSERT(bytesToReadWrite == FatData->BytesPerSector);

        // Update the FSI_Nxt_Free and FSI_Free_Count

        // fatgen103.pdf:
        // Typically this value is set to the last cluster number that the driver allocated.
        pFsInfo->FSI_Nxt_Free = (DWORD)reservedCluster;

        if (FAT32_UNKNOWN == pFsInfo->FSI_Free_Count)
        {
            // calculate remaining free clusters
            pFsInfo->FSI_Free_Count = 0;

            freeCluster = reservedCluster;

            for(;;)
            {
                status = _FirstFreeCluster(FatData, freeCluster + 1, FALSE, &freeCluster);
                if (!SUCCEEDED(status))
                {
                    LOG_FUNC_ERROR("FirstFreeCluster", status);
                    __leave;
                }

                if (0 == freeCluster)
                {
                    break;
                }

                pFsInfo->FSI_Free_Count++;
            }
        }
        else // FSI_Free_Count != FAT32_UNKNOWN or 0
        {
            pFsInfo->FSI_Free_Count--;
        }

    }
    __finally
    {
        *ReservedCluster = reservedCluster;

        // Write the FsInfo back to the disk
        // (placed here in case of __leave when disk is full)
        bytesToReadWrite = sizeof(FSINFO);
        status = IoWriteDevice(
            FatData->VolumeDevice,
            pFsInfo,
            &bytesToReadWrite,
            1 * FatData->BytesPerSector
        );
        if (!SUCCEEDED(status))
        {
            LOG_FUNC_ERROR("IoWriteDevice", status);
        }
        ASSERT(bytesToReadWrite == FatData->BytesPerSector);

        // We free the memory allocated
        if (NULL != pFsInfo)
        {
            ExFreePoolWithTag(pFsInfo, HEAP_TEMP_TAG);
            pFsInfo = NULL;
        }

        // We free the memory allocated
        if (NULL != pFat32Entries)
        {
            ExFreePoolWithTag(pFat32Entries, HEAP_TEMP_TAG);
            pFat32Entries = NULL;
        }
    }

    return status;
}

STATUS
FirstSectorOfCluster(
    IN      PFAT_DATA   FatData,
    IN      QWORD       Cluster,
    OUT     QWORD*      Sector
    )
{
    ASSERT(NULL != FatData);
    ASSERT(NULL != Sector);

    if ((Cluster > FatData->CountOfClusters + 1) || (Cluster < 2))
    {
        // there is no such cluster
        LOG_TRACE_FILESYSTEM("Cluster: 0x%X\n", Cluster);
        LOG_TRACE_FILESYSTEM("Count of clusters: 0x%x\n", FatData->CountOfClusters);
        LOG_TRACE_FILESYSTEM("Sectors per cluster: 0x%x\n", FatData->SectorsPerCluster);
        LOG_TRACE_FILESYSTEM("FatData->FirstDataSector: 0x%x\n", FatData->FirstDataSector);

        return STATUS_DEVICE_CLUSTER_INVALID;
    }

    *Sector = (((Cluster - 2) * FatData->SectorsPerCluster) + FatData->FirstDataSector);

    return STATUS_SUCCESS;
}

STATUS
ClusterOfSector(
    IN      PFAT_DATA   FatData,
    IN      QWORD       Sector,
    OUT     QWORD*      Cluster
    )
{
    ASSERT(NULL != Cluster);

    *Cluster = (((Sector - FatData->FirstDataSector) / FatData->SectorsPerCluster) + 2);

    LOG_TRACE_FILESYSTEM("Sector: 0x%x\n", Sector);
    LOG_TRACE_FILESYSTEM("*Result: 0x%x\n", *Cluster);
    LOG_TRACE_FILESYSTEM("Count of clusters: 0x%x\n", FatData->CountOfClusters);
    LOG_TRACE_FILESYSTEM("Sectors per cluster: 0x%x\n", FatData->SectorsPerCluster);
    LOG_TRACE_FILESYSTEM("FatData->FirstDataSector: 0x%x\n", FatData->FirstDataSector);

    if ((*Cluster > FatData->CountOfClusters + 1) || (*Cluster < 2))
    {
        return STATUS_DEVICE_CLUSTER_INVALID;
    }

    return STATUS_SUCCESS;
}

STATUS
GetDirEntryFromSector(
    IN      PFAT_DATA   FatData,
    IN      QWORD       EntrySector,
    IN      QWORD       DirFirstSector, // search criterion
    OUT     QWORD*      EntryIndex,
    OUT     DIR_ENTRY*  DirEntry
)
{
    STATUS status = STATUS_UNSUCCESSFUL;
    LONG_DIR_ENTRY* pLongEntry = NULL;
    DIR_ENTRY* pDirEntryArray = NULL;
    QWORD index = 0;
    QWORD bytesToRead = 0;
    QWORD dirFstClusterToSearch = 0;

    __try
    {
        bytesToRead = FatData->BytesPerSector * FatData->SectorsPerCluster;

        pDirEntryArray = (DIR_ENTRY*)ExAllocatePoolWithTag(PoolAllocateZeroMemory, (DWORD)bytesToRead, HEAP_TEMP_TAG, 0);
        if (NULL == pDirEntryArray)
        {
            status = STATUS_HEAP_NO_MORE_MEMORY;
            // malloc failed
            LOG_FUNC_ERROR_ALLOC("HeapAllocatePoolWithTag", bytesToRead);
            __leave;
        }

        status = IoReadDevice(FatData->VolumeDevice,
            pDirEntryArray,
            &bytesToRead,
            EntrySector * FatData->BytesPerSector
        );
        if (!SUCCEEDED(status))
        {
            LOG_FUNC_ERROR("IoReadDevice", status);
            __leave;
        }
        ASSERT(bytesToRead == FatData->BytesPerSector * FatData->SectorsPerCluster);

        status = ClusterOfSector(FatData, DirFirstSector, &dirFstClusterToSearch);
        if (!SUCCEEDED(status))
        {
            LOG_FUNC_ERROR("ClusterOfSector", status);
            __leave;
        }

        for (index = 0; index < bytesToRead / sizeof(DIR_ENTRY); index++)
        {
            if ((FREE_ENTRY == pDirEntryArray[index].DIR_Name[0]) ||
                (FREE_JAP_ENTRY == pDirEntryArray[index].DIR_Name[0]))
            {
                // this entry is empty
                continue;
            }

            pLongEntry = (LONG_DIR_ENTRY*)&(pDirEntryArray[index]);

            if ((pLongEntry->LDIR_Attr & ATTR_LONG_NAME_MASK) == ATTR_LONG_NAME)
            {
                // might be used in the future

                continue;
            }

            if (dirFstClusterToSearch == WORDS_TO_DWORD(pDirEntryArray[index].DIR_FstClusHI, pDirEntryArray[index].DIR_FstClusLO))
            {
                memcpy(DirEntry, &pDirEntryArray[index], sizeof(DIR_ENTRY));
                *EntryIndex = index;

                status = STATUS_SUCCESS;
                __leave;
            }
        }

        *EntryIndex = MAX_DWORD;
        status = STATUS_ELEMENT_NOT_FOUND;
    }
    __finally
    {
        if (NULL != pDirEntryArray)
        {
            ExFreePoolWithTag(pDirEntryArray, HEAP_TEMP_TAG);
            pDirEntryArray = NULL;
        }
    }

    return status;
}

STATUS
WriteDirEntryToSector(
    IN      PFAT_DATA   FatData,
    IN      QWORD       EntrySector,
    IN      QWORD       EntryIndex,
    IN      DIR_ENTRY*  DirEntry
)
{
    STATUS status = STATUS_UNSUCCESSFUL;
    DIR_ENTRY* pDirEntryArray = NULL;
    QWORD bytesToReadWrite = 0;

    __try
    {
        bytesToReadWrite = FatData->BytesPerSector * FatData->SectorsPerCluster;

        pDirEntryArray = (DIR_ENTRY*)ExAllocatePoolWithTag(PoolAllocateZeroMemory, (DWORD)bytesToReadWrite, HEAP_TEMP_TAG, 0);
        if (NULL == pDirEntryArray)
        {
            status = STATUS_HEAP_NO_MORE_MEMORY;
            // malloc failed
            LOG_FUNC_ERROR_ALLOC("HeapAllocatePoolWithTag", bytesToReadWrite);
            __leave;
        }

        status = IoReadDevice(FatData->VolumeDevice,
            pDirEntryArray,
            &bytesToReadWrite,
            EntrySector * FatData->BytesPerSector
        );
        if (!SUCCEEDED(status))
        {
            LOG_FUNC_ERROR("IoReadDevice", status);
            __leave;
        }
        ASSERT(bytesToReadWrite == FatData->BytesPerSector * FatData->SectorsPerCluster);

        memcpy(&pDirEntryArray[EntryIndex], (PVOID)DirEntry, sizeof(DIR_ENTRY));

        status = IoWriteDevice(FatData->VolumeDevice,
            pDirEntryArray,
            &bytesToReadWrite,
            EntrySector * FatData->BytesPerSector
        );
        if (!SUCCEEDED(status))
        {
            LOG_FUNC_ERROR("IoWriteDevice", status);
            __leave;
        }
        ASSERT(bytesToReadWrite == FatData->BytesPerSector * FatData->SectorsPerCluster);

        status = STATUS_SUCCESS;
    }
    __finally
    {
        if (NULL != pDirEntryArray)
        {
            ExFreePoolWithTag(pDirEntryArray, HEAP_TEMP_TAG);
            pDirEntryArray = NULL;
        }
    }

    return status;
}

STATUS
ConvertFatDateTimeToDateTime(
    IN      FATDATE*    FatDate,
    IN      FATTIME*    FatTime,
    OUT     DATETIME*   DateTime
    )
{
    STATUS status;

    ASSERT(NULL != DateTime);

    status = STATUS_SUCCESS;

    status = _ConvertFatDateToDate(FatDate, &DateTime->Date);
    if (!SUCCEEDED(status))
    {
        LOG_FUNC_ERROR("ConvertFatDateToDate", status);
        return status;
    }

    status = _ConvertFatTimeToTime(FatTime, &DateTime->Time);
    if (!SUCCEEDED(status))
    {
        LOG_FUNC_ERROR("ConvertFatTimeToTime", status);
        return status;
    }

    return status;
}

void
ConvertDateTimeToFatDateTime(
    IN      PDATETIME   DateTime,
    OUT     FATDATE*    FatDate,
    OUT     FATTIME*    FatTime
    )
{
    ASSERT(NULL != DateTime);

    _ConvertDateToFatDate(&DateTime->Date, FatDate);
    _ConvertTimeToFatTime(&DateTime->Time, FatTime);
}

STATUS
ConvertFatNameToName(
    IN_READS(SHORT_NAME_CHARS)      char*       FatName,
    IN                              DWORD       BufferSize,
    OUT_WRITES(BufferSize)          char*       Buffer,
    OUT                             DWORD*      ActualNameLength
    )
{
    STATUS status;
    DWORD nameLen;
    char extName[4];
    char normalName[9];
    DWORD lastSpaceIndex;
    DWORD i;

    ASSERT(NULL != FatName);
    ASSERT(NULL != Buffer);
    ASSERT(NULL != ActualNameLength);

    status = STATUS_SUCCESS;
    nameLen = 0;
    lastSpaceIndex = SHORT_NAME_NAME;

    // retrieve normal name
    for (i = 0; i < SHORT_NAME_NAME; ++i)
    {
        normalName[i] = FatName[i];
        if (FatName[i] == ' ')
        {
            if (lastSpaceIndex > i)
            {
                lastSpaceIndex = i;
            }
        }
        else
        {
            lastSpaceIndex = SHORT_NAME_NAME;
        }
    }
    normalName[lastSpaceIndex] = '\0';

    // retrieve extension
    for (i = 0; i < SHORT_NAME_EXT; ++i)
    {
        extName[i] = FatName[i+SHORT_NAME_NAME];
        if (FatName[i + SHORT_NAME_NAME] == ' ')
        {
            // we found a space and it's over
            break;
        }
    }
    extName[i] = '\0';

    nameLen = strlen(normalName) + i + ( ( 0 != i ) ? 1 : 0 );
    *ActualNameLength = nameLen;
    if (nameLen >= BufferSize)
    {
        return STATUS_BUFFER_TOO_SMALL;
    }

    if (0 == i)
    {
        // there is no extension
        strcpy(Buffer, normalName);
    }
    else
    {
        // there is an extension
        status = snprintf(Buffer, BufferSize, "%s.%s", normalName, extName);
    }

    return status;
}

STATUS
WritePartialLongFatNameToName(
    IN                          LONG_DIR_ENTRY* LongDirEntry,
    IN                          DWORD           BufferSize,
    INOUT_UPDATES(BufferSize)   char*           Buffer
)
{
    ASSERT(NULL != LongDirEntry);
    ASSERT(0 != BufferSize);
    ASSERT(NULL != Buffer);

    STATUS status = STATUS_SUCCESS;
    DWORD entryOrdinal = (LongDirEntry->LDIR_Ord & ~LONG_NAME_ORD_END_MASK) - 1;
    BOOLEAN lastLongEntry = IsBooleanFlagOn(LongDirEntry->LDIR_Ord, LONG_NAME_ORD_END_MASK);
    DWORD bufferWriteIndex = 0;
    DWORD i = 0;
    DWORD j = 0;

    BYTE const* longNames[3] = { 0 };
    DWORD longNameLengths[3] = { 0 };

    longNames[0] = LongDirEntry->LDIR_Name1;
    longNames[1] = LongDirEntry->LDIR_Name2;
    longNames[2] = LongDirEntry->LDIR_Name3;

    longNameLengths[0] = LONG_NAME1_CHARS * sizeof(WCHAR);
    longNameLengths[1] = LONG_NAME2_CHARS * sizeof(WCHAR);
    longNameLengths[2] = LONG_NAME3_CHARS * sizeof(WCHAR);

    // calculate position of partial long name in the final name
    bufferWriteIndex = LONG_NAME_TOTAL_CHARS * entryOrdinal;

    // for each array of characters in the long directory entry
    for (j = 0; j < ARRAYSIZE(longNames); j++)
    {
        // for each ascii character
        for (i = 0; i < longNameLengths[j]; i += sizeof(WCHAR))
        {
            if ('\0' == longNames[j][i])
            {
                j = ARRAYSIZE(longNames); // break outer loop too
                break;
            }

            if (bufferWriteIndex >= BufferSize - 1)
            {
                return STATUS_BUFFER_TOO_SMALL;
            }

            Buffer[bufferWriteIndex] = longNames[j][i];

            bufferWriteIndex++;
        }
    }

    if (lastLongEntry)
    {
        Buffer[bufferWriteIndex] = '\0';
    }

    return status;
}

static
STATUS
_ConvertFatDateToDate(
    IN      FATDATE*    FatDate,
    OUT     DATE*       RealDate
    )
{
    ASSERT(NULL != RealDate);
    ASSERT(NULL != FatDate);

    RealDate->Day = (BYTE) FatDate->Day;

    if ((RealDate->Day < FAT_DAY_RANGE_MIN) || (RealDate->Day > FAT_DAY_RANGE_MAX))
    {
        // The day is not valid
        return STATUS_TIME_INVALID;
    }

    RealDate->Month = (BYTE) FatDate->Month;

    if ((RealDate->Month < FAT_MONTH_RANGE_MIN) || (RealDate->Month > FAT_MONTH_RANGE_MAX))
    {
        // the month is invalid
        return STATUS_TIME_INVALID;
    }

    RealDate->Year = FatDate->Year + FAT_START_YEAR;

    return STATUS_SUCCESS;
}

static
STATUS
_ConvertFatTimeToTime(
    IN      FATTIME*    FatTime,
    OUT     TIME*       RealTime
    )
{
    ASSERT(NULL != RealTime);

    RealTime->Second = (BYTE) FatTime->Second;

    if ((RealTime->Second < FAT_SECOND_RANGE_MIN) || (RealTime->Second > FAT_SECOND_RANGE_MAX))
    {
        // the seconds parameter is invalid
        return STATUS_TIME_INVALID;
    }

    RealTime->Second *= 2;

    RealTime->Minute = (BYTE) FatTime->Minute;

    if ((RealTime->Minute < FAT_MINUTE_RANGE_MIN) || (RealTime->Minute > FAT_MINUTE_RANGE_MAX))
    {
        // the minutes parameter is invalid
        return STATUS_TIME_INVALID;
    }

    RealTime->Hour = (BYTE) FatTime->Hour;

    if ((RealTime->Hour < FAT_HOUR_RANGE_MIN) || (RealTime->Hour > FAT_HOUR_RANGE_MAX))
    {
        // the hours parameter is invalid
        return STATUS_TIME_INVALID;
    }

    return STATUS_SUCCESS;
}

static
void
_ConvertDateToFatDate(
    IN      DATE*       RealDate,
    OUT     FATDATE*    FatDate
    )
{
    ASSERT(NULL != RealDate);
    ASSERT(NULL != FatDate);

    FatDate->Year = RealDate->Year - FAT_START_YEAR;
    FatDate->Month = RealDate->Month;
    FatDate->Day = RealDate->Day;
}

static
void
_ConvertTimeToFatTime(
    IN      TIME*       RealTime,
    OUT     FATTIME*    FatTime
    )
{
    ASSERT(NULL != RealTime);
    ASSERT(NULL != FatTime);

    FatTime->Hour = RealTime->Hour;
    FatTime->Minute = RealTime->Minute;
    FatTime->Second = RealTime->Second / 2;
}