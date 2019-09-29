#include "fat32_base.h"
#include "fat_operations.h"
#include "fat_utils.h"


static
void
_FatPopulateFileInformationFromFatEntry(
    IN      PFAT_DATA               FatData,
    IN      PDIR_ENTRY              DirEntry,
    OUT     PFILE_INFORMATION       FileInformation
);

STATUS
FatInitVolume(
    INOUT          PFAT_DATA           FatData
)
{
    STATUS status;
    FAT_BPB bpb;
    PDEVICE_OBJECT pVolumeDevice;
    QWORD bytesToRead;
    DWORD fatSize;
    DWORD totalSectors;
    DWORD dataSectors;

    ASSERT(NULL != FatData);

    status = STATUS_SUCCESS;
    pVolumeDevice = FatData->VolumeDevice;
    bytesToRead = sizeof(FAT_BPB);
    ASSERT(NULL != pVolumeDevice);

    status = IoReadDevice(pVolumeDevice, &bpb, &bytesToRead, 0);
    if (!SUCCEEDED(status))
    {
        LOG_FUNC_ERROR("IoReadDevice", status);
        return status;
    }

    if (MBR_BOOT_SIGNATURE != ((WORD*)&bpb)[255])
    {
        LOG_ERROR("Invalid FAT Partition\n");
        return STATUS_DISK_MBR_NOT_PRESENT;
    }

    // Display basic information about the FAT structure
    LOG_TRACE_FILESYSTEM("OEM Name: %s\n", bpb.BS_OEMName);
    LOG_TRACE_FILESYSTEM("Bytes per sector: 0x%X\n", bpb.BPB_BytsPerSec);
    LOG_TRACE_FILESYSTEM("Sectors per cluster: 0x%X\n", bpb.BPB_SecPerClus);
    LOG_TRACE_FILESYSTEM("Reserved sectors: 0x%X\n", bpb.BPB_RsvdSecCnt);
    LOG_TRACE_FILESYSTEM("Number of FATs: 0x%X\n", bpb.BPB_NumFATs);

    // We start determining the type of FAT it is
    // FATSz = Size of a FAT
    if (0 != bpb.BPB_FATSz16)
    {
        fatSize = bpb.BPB_FATSz16;
    }
    else
    {
        fatSize = bpb.DiffOffset.FAT32_BPB.BPB_FATSz32;
    }

    LOG_TRACE_FILESYSTEM("Fat Size: 0x%X\n", fatSize);

    // TotalSectors
    if (0 != bpb.BPB_TotSec16)
    {
        totalSectors = bpb.BPB_TotSec16;
    }
    else
    {
        totalSectors = bpb.BPB_TotSec32;
    }

    LOG_TRACE_FILESYSTEM("Total sectors: 0x%X\n", totalSectors);

    // Data sectors
    dataSectors = totalSectors - (bpb.BPB_RsvdSecCnt + (bpb.BPB_NumFATs * fatSize));
    LOG_TRACE_FILESYSTEM("Data sectors: 0x%X\n", dataSectors);

    // Number of clusters
    FatData->CountOfClusters = dataSectors / bpb.BPB_SecPerClus;
    LOG_TRACE_FILESYSTEM("Count of clusters: %d\n", FatData->CountOfClusters);

    // Determining the type of FAT
    if (FatData->CountOfClusters < FAT12_MAX_CLUSTERS)
    {
        LOG_TRACE_FILESYSTEM("Volume is FAT12\n");
        LOG_TRACE_FILESYSTEM("We work only with FAT32\n");
        return STATUS_DEVICE_FILESYSTEM_UNSUPPORTED;
    }
    else
    {
        if (FatData->CountOfClusters < FAT16_MAX_CLUSTERS)
        {
            LOG_TRACE_FILESYSTEM("Volume is FAT16\n");
            LOG_TRACE_FILESYSTEM("We work only with FAT32\n");
            return STATUS_DEVICE_FILESYSTEM_UNSUPPORTED;
        }
        else
        {
            LOG_TRACE_FILESYSTEM("Volume is FAT32\n");
        }
    }


    // Sector in which data starts
    FatData->FirstDataSector = bpb.BPB_RsvdSecCnt + (bpb.BPB_NumFATs * fatSize);

    LOG_TRACE_FILESYSTEM("First data sector: 0x%X\n", FatData->FirstDataSector);
    LOG_TRACE_FILESYSTEM("Root directory: 0x%X\n", bpb.DiffOffset.FAT32_BPB.BPB_RootClus);

    // Number of sectors per cluster
    FatData->SectorsPerCluster = bpb.BPB_SecPerClus;

    // we set the value of the RootDirectoryStartSector
    status = FirstSectorOfCluster(FatData, bpb.DiffOffset.FAT32_BPB.BPB_RootClus, &(FatData->RootDirectoryStartSector));
    if (!SUCCEEDED(status))
    {
        LOG_FUNC_ERROR("_FirstSectorOfCluster", status);
        return status;
    }

    // Number of FAT entries on a sector
    FatData->EntriesPerSector = bpb.BPB_BytsPerSec / sizeof(DIR_ENTRY);

    // Number of reserved sectors
    FatData->ReservedSectors = bpb.BPB_RsvdSecCnt;

    // Number of bytes per sector
    FatData->BytesPerSector = bpb.BPB_BytsPerSec;

    FatData->AllocationSize = FatData->SectorsPerCluster * FatData->BytesPerSector;
    LOG_TRACE_FILESYSTEM("ALlocation size: %d\n", FatData->AllocationSize);

    ASSERT_INFO(FatData->AllocationSize >= pVolumeDevice->DeviceAlignment,
        "The FAT driver does not handle issues caused by greater device alignment needed by volume devices");

    return status;
}

STATUS
FatSearch(
    IN      PFAT_DATA               FatData,
    IN_Z    char*                   Name,
    OUT     QWORD*                  DirectorySector,
    IN      BYTE                    SearchType,
    OUT_OPT PFILE_INFORMATION       FileInformation,
    OUT     QWORD*                  ParentSector
)
{
    // name of current entry to search ( name between backslashes )
    char partialName[LONG_NAME_MAX_CHARS];

    // current index of partial name in the full path name
    DWORD startIndexInName;

    char* partialNameEnd;
    DWORD partialNameLength;

    // set to 1 when we reach the last directory
    BOOLEAN finishedParse;

    QWORD tempResult;
    QWORD currentSearchSector;

    // until the last instance we're searching only for directories
    BYTE currentSearchType;
    STATUS status;

    LOG_FUNC_START;

    ASSERT(NULL != FatData);
    ASSERT(NULL != Name);
    ASSERT(NULL != DirectorySector);
    ASSERT(NULL != ParentSector);

    memzero(partialName, sizeof(partialName));
    startIndexInName = 0;
    partialNameEnd = NULL;
    partialNameLength = 0;
    finishedParse = FALSE;
    tempResult = 0;
    currentSearchSector = 0;
    currentSearchType = ATTR_DIRECTORY;
    status = STATUS_SUCCESS;

    if (FAT_DELIMITER != Name[0])
    {
        // path must start with backslash
        return STATUS_INVALID_FILE_NAME;
    }

    LOG_TRACE_FILESYSTEM("Will search for file [%s], search type: [%x]\n", Name, SearchType);

    currentSearchSector = FatData->RootDirectoryStartSector;

    if (1 == strlen(Name))
    {
        LOG_TRACE_FILESYSTEM("Search for root directory\n");

        // we have the root directory
        if (ATTR_DIRECTORY != (SearchType & ATTR_DIRECTORY))
        {
            return STATUS_FILE_TYPE_INVALID;
        }

        // we already checked that Name[0] is '\\'
        *DirectorySector = currentSearchSector;
        *ParentSector = 0;

        if (NULL != FileInformation)
        {
            memzero(FileInformation, sizeof(FILE_INFORMATION));
            FileInformation->FileAttributes = FILE_ATTRIBUTE_VOLUME;
        }

        return STATUS_SUCCESS;
    }

    startIndexInName++;

    do
    {
        // we get a pointer to the first occurrence of a backslash after the
        // last found backslash
        partialNameEnd = (char*)strchr((Name + startIndexInName), FAT_DELIMITER);

        if (Name + startIndexInName == partialNameEnd)
        {
            // if we didn't find an occurrence => we arrived to the last
            // file
            partialNameLength = strlen(Name) - startIndexInName;
            currentSearchType = SearchType;
            finishedParse = TRUE; // we finished the search
        }
        else
        {
            partialNameLength = (DWORD)(partialNameEnd - (Name + startIndexInName));
        }

        if (partialNameLength >= sizeof(partialName) || partialNameLength == 0)
        {
            LOG_TRACE_FILESYSTEM("[ERROR]Partial name length is %u\n", partialNameLength);
            return STATUS_PATH_NOT_VALID;
        }

        // we copy the name to a new buffer so we can append a '\0' to it
        // (done automatically by str(n)cpy
        strncpy(partialName, (Name + startIndexInName), partialNameLength);
        startIndexInName += partialNameLength + 1; // we update the index in the name buffer

                                                   // We search for the partial name in the current directory sector
        status = FatSearchDirectoryEntry(FatData, currentSearchSector, partialName, currentSearchType, &tempResult, FileInformation, ParentSector);
        if (!SUCCEEDED(status))
        {
            // something bad happened
            LOG_TRACE_FILESYSTEM("[ERROR]SearchDirectoryEntry failed with status 0x%x\n", status);
            return status;
        }

        // we update the current directory sector
        currentSearchSector = tempResult;

    } while (!finishedParse);

    *DirectorySector = currentSearchSector;

    LOG_FUNC_END;

    return STATUS_SUCCESS;
}

STATUS
FatReadFile(
    IN      PFAT_DATA   FatData,
    IN      QWORD       BaseFileSector,
    IN      QWORD       SectorOffset,
    IN      QWORD       DirEntrySector,
    IN      PVOID       Buffer,
    IN      QWORD       SectorsToRead,
    OUT     QWORD*      SectorsRead,
    IN      BOOLEAN     Asynchronous
)
{
    STATUS status;
    QWORD currentSector;                // the sector in which the file is
    QWORD nextSector;
    QWORD sectorsRemaining;             // how much of the file we have parsed so far
    QWORD sectorsToRead;
    QWORD bytesToRead;
    PBYTE pData;
    QWORD sectorsTraversed;

    DIR_ENTRY  dirEntry = { 0 };
    QWORD dirEntryIndex = 0;
    DATETIME currentDateTime = { 0 };
    FATDATE fatDate = { 0 };
    FATTIME fatTime = { 0 };

    LOG_FUNC_START;

    ASSERT(NULL != FatData);
    ASSERT(NULL != Buffer);

    ASSERT(IsAddressAligned(BaseFileSector, FatData->SectorsPerCluster));

    status = STATUS_SUCCESS;
    currentSector = BaseFileSector;
    nextSector = 0;
    sectorsRemaining = SectorsToRead;
    sectorsTraversed = 0;
    sectorsToRead = 0;
    bytesToRead = 0;
    pData = (PBYTE)Buffer;

    LOG_TRACE_FILESYSTEM("Base file sector: [0x%x]\n", BaseFileSector);
    LOG_TRACE_FILESYSTEM("Sector offset: [0x%x]\n", SectorOffset);

    if (0 == sectorsRemaining)
    {
        *SectorsRead = 0;
        return STATUS_SUCCESS;
    }

    while (sectorsTraversed + FatData->SectorsPerCluster <= SectorOffset)
    {
        ASSERT(SectorOffset >= FatData->SectorsPerCluster);

        status = NextSectorInClusterChain(FatData, currentSector, &nextSector, FALSE);
        if (!SUCCEEDED(status))
        {
            LOG_FUNC_ERROR("NextSectorInClusterChain", status);
            return status;
        }

        currentSector = nextSector;
        sectorsTraversed = sectorsTraversed + FatData->SectorsPerCluster;
    }
    ASSERT(0 != currentSector);

    // we modify current sector to sectorToReach because
    // we do not need to read the whole cluster
    currentSector = currentSector + (SectorOffset % FatData->SectorsPerCluster);

    // it is possible that the first sector to read is in the middle of a cluster
    sectorsToRead = min(sectorsRemaining, FatData->SectorsPerCluster - (SectorOffset % FatData->SectorsPerCluster));
    bytesToRead = sectorsToRead * FatData->BytesPerSector;

    for (;;)
    {
        LOG_TRACE_FILESYSTEM("Will read [0x%x] sectors starting from sector [0x%x]\n", sectorsToRead, currentSector);

        status = IoReadDeviceEx(
            FatData->VolumeDevice,
            pData,
            &bytesToRead,
            currentSector * FatData->BytesPerSector,
            Asynchronous
        );
        if (!SUCCEEDED(status))
        {
            LOG_FUNC_ERROR("IoReadDeviceEx", status);
            return status;
        }
        ASSERT(bytesToRead == sectorsToRead * FatData->BytesPerSector);

        pData = pData + bytesToRead;

        sectorsRemaining = sectorsRemaining - sectorsToRead;

        if (0 == sectorsRemaining)
        {
            break;
        }

        // find next sector
        status = NextSectorInClusterChain(FatData, currentSector, &nextSector, FALSE);
        if (!SUCCEEDED(status))
        {
            LOG_FUNC_ERROR("NextSectorInClusterChain", status);
            return status;
        }

        if (0 == nextSector)
        {
            // reached EOC marker
            break;
        }

        currentSector = nextSector;

        sectorsToRead = min(FatData->SectorsPerCluster, sectorsRemaining);
        bytesToRead = sectorsToRead * FatData->BytesPerSector;
    }

    status = GetDirEntryFromSector(FatData, DirEntrySector, BaseFileSector, &dirEntryIndex, &dirEntry);
    if (!SUCCEEDED(status))
    {
        LOG_FUNC_ERROR("GetDirEntryFromSector", status);
        return status;
    }

    currentDateTime = IoGetCurrentDateTime();

    ConvertDateTimeToFatDateTime(&currentDateTime, &fatDate, &fatTime);

    dirEntry.DIR_LstAccDate = fatDate;

    status = WriteDirEntryToSector(FatData, DirEntrySector, dirEntryIndex, &dirEntry);
    if (!SUCCEEDED(status))
    {
        LOG_FUNC_ERROR("WriteDirEntryToSector", status);
        return status;
    }

    *SectorsRead = SectorsToRead - sectorsRemaining;

    return status;
}

STATUS
FatWriteFile(
    IN      PFAT_DATA   FatData,
    IN      QWORD       BaseFileSector,
    IN      QWORD       SectorOffset,
    IN      QWORD       DirEntrySector,
    IN      PVOID       Buffer,
    IN      QWORD       SectorsToWrite,
    OUT     QWORD*      SectorsWritten,
    IN      BOOLEAN     Asynchronous
)
{
    STATUS status;
    QWORD currentSector;                // the sector in which the file is
    QWORD nextSector;
    QWORD sectorsRemaining;             // how much of the file we have parsed so far
    QWORD sectorsToWrite;
    QWORD bytesToWrite;
    PBYTE pData;
    QWORD sectorsTraversed;
    DIR_ENTRY  dirEntry = { 0 };
    QWORD dirEntryIndex = 0;
    DATETIME currentDateTime = { 0 };
    FATDATE fatDate = { 0 };
    FATTIME fatTime = { 0 };
    DWORD nextByteToWrite = 0;

    LOG_FUNC_START;

    ASSERT(NULL != FatData);
    ASSERT(NULL != Buffer);

    ASSERT(IsAddressAligned(BaseFileSector, FatData->SectorsPerCluster));

    status = STATUS_SUCCESS;
    currentSector = BaseFileSector;
    nextSector = 0;
    sectorsRemaining = SectorsToWrite;
    sectorsTraversed = 0;
    sectorsToWrite = 0;
    bytesToWrite = 0;
    pData = (PBYTE)Buffer;

    LOG_TRACE_FILESYSTEM("Base file sector: [0x%x]\n", BaseFileSector);
    LOG_TRACE_FILESYSTEM("Sector offset: [0x%x]\n", SectorOffset);

    if (0 == sectorsRemaining)
    {
        *SectorsWritten = 0;
        return STATUS_SUCCESS;
    }

    while (sectorsTraversed + FatData->SectorsPerCluster <= SectorOffset)
    {
        ASSERT(SectorOffset >= FatData->SectorsPerCluster);

        status = NextSectorInClusterChain(FatData, currentSector, &nextSector, TRUE);
        if (!SUCCEEDED(status))
        {
            LOG_FUNC_ERROR("NextSectorInClusterChain", status);
            return status;
        }

        currentSector = nextSector;
        sectorsTraversed = sectorsTraversed + FatData->SectorsPerCluster;
    }
    ASSERT(0 != currentSector);

    // we modify current sector to sectorToReach because
    // we do not need to write to the whole cluster
    currentSector = currentSector + (SectorOffset % FatData->SectorsPerCluster);

    // it is possible that the first sector to write to is in the middle of a cluster
    sectorsToWrite = min(sectorsRemaining, FatData->SectorsPerCluster - (SectorOffset % FatData->SectorsPerCluster));
    bytesToWrite = sectorsToWrite * FatData->BytesPerSector;

    for (;;)
    {
        LOG_TRACE_FILESYSTEM("Will write [0x%x] sectors starting from sector [0x%x]\n", sectorsToWrite, currentSector);

        status = IoWriteDeviceEx(
            FatData->VolumeDevice,
            pData,
            &bytesToWrite,
            currentSector * FatData->BytesPerSector,
            Asynchronous
        );
        if (!SUCCEEDED(status))
        {
            LOG_FUNC_ERROR("IoWriteDeviceEx", status);
            return status;
        }
        ASSERT(bytesToWrite == sectorsToWrite * FatData->BytesPerSector);

        pData = pData + bytesToWrite;

        sectorsRemaining = sectorsRemaining - sectorsToWrite;

        if (0 == sectorsRemaining)
        {
            break;
        }

        // find next sector, extend chain if necessary
        status = NextSectorInClusterChain(FatData, currentSector, &nextSector, TRUE);
        if (!SUCCEEDED(status))
        {
            LOG_FUNC_ERROR("NextSectorInClusterChain", status);
            return status;
        }

        currentSector = nextSector;

        sectorsToWrite = min(FatData->SectorsPerCluster, sectorsRemaining);
        bytesToWrite = sectorsToWrite * FatData->BytesPerSector;
    }

    status = GetDirEntryFromSector(FatData, DirEntrySector, BaseFileSector, &dirEntryIndex, &dirEntry);
    if (!SUCCEEDED(status))
    {
        LOG_FUNC_ERROR("GetDirEntryFromSector", status);
        return status;
    }

    nextByteToWrite = (DWORD)(SectorOffset + SectorsToWrite) * FatData->BytesPerSector;

    if (nextByteToWrite > dirEntry.DIR_FileSize)
    {
        dirEntry.DIR_FileSize = nextByteToWrite;
    }

    currentDateTime = IoGetCurrentDateTime();

    ConvertDateTimeToFatDateTime(&currentDateTime, &fatDate, &fatTime);

    dirEntry.DIR_LstAccDate = fatDate;
    dirEntry.DIR_WrtDate = fatDate;
    dirEntry.DIR_WrtTime = fatTime;

    status = WriteDirEntryToSector(FatData, DirEntrySector, dirEntryIndex, &dirEntry);
    if (!SUCCEEDED(status))
    {
        LOG_FUNC_ERROR("WriteDirEntryToSector", status);
        return status;
    }

    *SectorsWritten = SectorsToWrite;

    return status;
}

STATUS
FatSearchDirectoryEntry(
    IN      PFAT_DATA               FatData,
    IN      QWORD                   SectorToSearch,
    IN_Z    char*                   Name,
    IN      BYTE                    SearchType,
    OUT     QWORD*                  SearchResult,
    OUT_OPT PFILE_INFORMATION       FileInformation,
    OUT     QWORD*                  ParentSector
)
{
    STATUS status;
    LONG_DIR_ENTRY* pLongEntry;
    DIR_ENTRY* pEntry;
    QWORD sectorToParse;
    int index;
    QWORD bytesToRead;
    char normalizedShortName[SHORT_NAME_MAX_LENGTH] = { 0 };
    char normalizedLongName[LONG_NAME_MAX_CHARS + 1] = { 0 };
    DWORD requiredLength;

    LOG_FUNC_START;

    // checking pointers
    ASSERT(NULL != FatData);
    ASSERT(NULL != Name);
    ASSERT(NULL != SearchResult);
    ASSERT(NULL != ParentSector);

    // don't need to check pointer FileSize because it's ok to be NULL if the user
    // does not want any value to be returned => will be checked later when setting result
    LOG_TRACE_FILESYSTEM("Will search for file [%s], search type: [%x]\n", Name, SearchType);

    status = STATUS_SUCCESS;
    pLongEntry = NULL;
    pEntry = NULL;
    sectorToParse = SectorToSearch;
    index = 0;
    bytesToRead = FatData->BytesPerSector * FatData->SectorsPerCluster;
    requiredLength = 0;

    __try
    {
        do
        {
            if (0 == (index % (FatData->EntriesPerSector  * FatData->SectorsPerCluster)))
            {
                // we need to read the next SectorsPerCluster sectors because we finished
                // searching the current cluster
                if (NULL != pEntry)
                {
                    // if we have already read SectorsPerCluster sectors we can deallocate the memory
                    // because we won't need it
                    ExFreePoolWithTag(pEntry, HEAP_TEMP_TAG);
                    pEntry = NULL;

                    // we go to the next sector we need to parse
                    status = NextSectorInClusterChain(FatData, sectorToParse, &sectorToParse, FALSE);
                    if (!SUCCEEDED(status))
                    {
                        // something bad happened :(
                        LOG_FUNC_ERROR("NextSectorInClusterChain", status);
                        __leave;
                    }

                    if (0 == sectorToParse)
                    {
                        // we have reached the EOC marker if the sector value is 0
                        status = STATUS_FILE_NOT_FOUND;
                        __leave;
                    }
                }

                // we allocate space for the buffer where a cluster will be read
                ASSERT(bytesToRead <= MAX_DWORD);
                pEntry = (DIR_ENTRY*)ExAllocatePoolWithTag(PoolAllocateZeroMemory, (DWORD)bytesToRead, HEAP_TEMP_TAG, 0);
                if (NULL == pEntry)
                {
                    // malloc failed
                    LOG_FUNC_ERROR_ALLOC("HeapAllocatePoolWithTag", bytesToRead);
                    __leave;
                }

                status = IoReadDeviceEx(FatData->VolumeDevice,
                    pEntry,
                    &bytesToRead,
                    sectorToParse * FatData->BytesPerSector,
                    TRUE
                );
                if (!SUCCEEDED(status))
                {
                    LOG_FUNC_ERROR("IoReadDeviceEx", status);
                    __leave;
                }
                ASSERT(bytesToRead == FatData->BytesPerSector * FatData->SectorsPerCluster);

                index = 0;    // we reset the index
            }

            if ((FREE_ENTRY == pEntry[index].DIR_Name[0]) || (FREE_JAP_ENTRY == pEntry[index].DIR_Name[0]))
            {
                // this entry is empty
                index++;        // we increment the current index and we
                continue;        // continue to the next entry
            }

            pLongEntry = (LONG_DIR_ENTRY*)&(pEntry[index]);

            if ((pLongEntry->LDIR_Attr & ATTR_LONG_NAME_MASK) == ATTR_LONG_NAME)
            {
                status = WritePartialLongFatNameToName(pLongEntry, LONG_NAME_MAX_CHARS + 1, normalizedLongName);
                ASSERT(SUCCEEDED(status));

                index++;
                continue;
            }

            // short directory entry

            LOG_TRACE_FILESYSTEM("[%d]: [%s][%x]\n", index, pEntry[index].DIR_Name, pEntry[index].DIR_Attr);

            status = ConvertFatNameToName((char*)pEntry[index].DIR_Name, SHORT_NAME_MAX_LENGTH, normalizedShortName, &requiredLength);
            ASSERT(SUCCEEDED(status));


            LOG_TRACE_FILESYSTEM("Normalized long  name: [%s]\n", normalizedLongName);
            LOG_TRACE_FILESYSTEM("Normalized short name: [%s]\n", normalizedShortName);

            if (0 == stricmp(normalizedLongName, Name) ||
                0 == stricmp(normalizedShortName, Name))
            {
                // we found what we were looking for
                QWORD cluster;

                // now we have to check if it's a corresponding directory entry
                BYTE maskResult = pEntry[index].DIR_Attr & (ATTR_DIRECTORY | ATTR_VOLUME_ID);

                if (maskResult != SearchType)
                {
                    LOG_WARNING("Found file, but with different attributes, Requested: [0x%x], Found: [0x%x]\n", SearchType, pEntry[index].DIR_Attr);
                    status = STATUS_FILE_TYPE_INVALID;
                    __leave;
                }
                *ParentSector = sectorToParse;

                // we find the cluster to which the DIR_ENTRY belongs to
                cluster = WORDS_TO_DWORD(pEntry[index].DIR_FstClusHI, pEntry[index].DIR_FstClusLO);

                // we set the sector from the cluster value
                status = FirstSectorOfCluster(FatData, cluster, SearchResult);
                if (!SUCCEEDED(status))
                {
                    LOG_FUNC_ERROR("FirstSectorOfCluster", status);
                    __leave;
                }

                if (NULL != FileInformation)
                {
                    // if the user requested the file size we set it
                    _FatPopulateFileInformationFromFatEntry(FatData, &pEntry[index], FileInformation);
                }

                // we go to clean even if success or failure
                __leave;
            }

            // reset long name
            // in case the next short directory entry does not have associated long name entries
            // it must not use the long name from a previous entry
            normalizedLongName[0] = '\0';

            // we increment the index to search the next directory entry

            index++;


        } while (FREE_ALL != pEntry[index].DIR_Name[0]);


        status = STATUS_FILE_NOT_FOUND;
    }
    __finally
    {
        if (NULL != pEntry)
        {
            ExFreePoolWithTag(pEntry, HEAP_TEMP_TAG);
            pEntry = NULL;
        }

        LOG_FUNC_END;
    }

    return status;
}

static
void
_FatPopulateFileInformationFromFatEntry(
    IN      PFAT_DATA               FatData,
    IN      PDIR_ENTRY              DirEntry,
    OUT     PFILE_INFORMATION       FileInformation
)
{
    STATUS status;

    ASSERT(NULL != FatData);
    ASSERT(NULL != DirEntry);
    ASSERT(NULL != FileInformation);

    FileInformation->FileSize = AlignAddressUpper(DirEntry->DIR_FileSize, FatData->VolumeDevice->DeviceAlignment);
    if (IsBooleanFlagOn(DirEntry->DIR_Attr, ATTR_VOLUME_ID))
    {
        FileInformation->FileAttributes = FILE_ATTRIBUTE_VOLUME;
    }
    else if (IsBooleanFlagOn(DirEntry->DIR_Attr, ATTR_DIRECTORY))
    {
        FileInformation->FileAttributes = FILE_ATTRIBUTE_DIRECTORY;
    }
    else
    {
        FileInformation->FileAttributes = FILE_ATTRIBUTE_NORMAL;
    }

    LOG_TRACE_FILESYSTEM("Attributes: [0x%x]\n", DirEntry->DIR_Attr);

    // the root entry has no creation or write datetime
    if (FILE_ATTRIBUTE_VOLUME != FileInformation->FileAttributes)
    {
        status = ConvertFatDateTimeToDateTime(&DirEntry->DIR_CrtDate, &DirEntry->DIR_CrtTime, &FileInformation->CreationTime);
        ASSERT(SUCCEEDED(status));

        status = ConvertFatDateTimeToDateTime(&DirEntry->DIR_WrtDate, &DirEntry->DIR_WrtTime, &FileInformation->LastWriteTime);
        ASSERT(SUCCEEDED(status));
    }
}

STATUS
FatCreateDirectoryEntry(
    IN      PFAT_DATA       FatData,
    IN_Z    char*           Name,
    IN      BYTE            FileAttributes
)
{
    char fullPath[MAX_PATH] = { 0 };
    char newEntryName[SHORT_NAME_CHARS + 1];
    char* pPathToBackSlash;
    DWORD lastBackslashIndex = 0;
    QWORD parentSector;
    QWORD dummySector;
    STATUS status = STATUS_SUCCESS;
    QWORD tempCluster;
    QWORD currentClusterInChain;
    QWORD finalSector;
    QWORD sectorAllocated;
    DWORD index = 0;                // index of the DIR_ENTRY in the current cluster
    DIR_ENTRY* pEntry = NULL;        // pointer to DIR_ENTRY vector
    FSINFO* pFSinfo = NULL;            // pointer to FSInfo
    BOOLEAN found;                    // found = 1 if we find free space in last cluster in chain
    BYTE* pFAT = NULL;                // pointer to FAT
    DATETIME crtDateTime;            // date time read from CMOS
    FATTIME fatTime;                    // time converted for FAT representation
    FATDATE fatDate;                    // date converted for FAT representation

    QWORD fatEntrySector;            // used for calculating the sector to read for the FAT entry
    DWORD fatEntryOffset;            // offset in the FAT sector read
    QWORD parentDirEntrySector;
    QWORD bytesToRead;

    LOG_FUNC_START;
    // Step 0. Check pointers
    ASSERT(NULL != FatData);
    ASSERT(NULL != Name);

    LOG_TRACE_FILESYSTEM("[ERROR]Will create file [%s]\n", Name);

    memset(newEntryName, ' ', SHORT_NAME_CHARS);

    // fullPath contains the path until the new directory entry
    pPathToBackSlash = (char*)strrchr(Name, FAT_DELIMITER);
    bytesToRead = 0;

    if (Name == pPathToBackSlash && FAT_DELIMITER != Name[0])
    {
        // path is not specified correctly
        LOG_TRACE_FILESYSTEM("'%c' not found in [%s]\n", FAT_DELIMITER, Name);
        return STATUS_PATH_NOT_VALID;
    }

    lastBackslashIndex = (DWORD)(pPathToBackSlash - Name);

    // in fullPath we copy the parent directory path
    if (0 == lastBackslashIndex)
    {
        strcpy(fullPath, "\\"); // parent directory is root
    }
    else
    {
        strncpy(fullPath, Name, lastBackslashIndex);
    }

    // newEntryName contains only the name of the to be created entry
    strcpy(newEntryName, (Name + lastBackslashIndex + 1));

    LOG_TRACE_FILESYSTEM("Directory: [%s]\n", fullPath);
    LOG_TRACE_FILESYSTEM("Filename: [%s]\n", newEntryName);

    // we don't want a NULL terminated string
    newEntryName[strlen(newEntryName)] = ' ';

    // Step 1. Check if to be parent directory exists
    status = FatSearch(FatData, fullPath, &parentSector, ATTR_DIRECTORY, NULL, &parentDirEntrySector);
    if (!SUCCEEDED(status))
    {
        LOG_FUNC_ERROR("FatSearch", status);
        return status;
    }

    // Step 2. Check if there is already a file with the same name
    status = FatSearch(FatData, Name, &dummySector, ATTR_NORMAL, NULL, &parentDirEntrySector);
    if (SUCCEEDED(status) || (STATUS_FILE_TYPE_INVALID == status))
    {
        return STATUS_FILE_ALREADY_EXISTS;
    }

    // Step 3. Go to the EOC cluster of the parent directory entry
    /// TODO check if there is enough space before the end cluster
    // we find the cluster of the parent
    status = ClusterOfSector(FatData, parentSector, &currentClusterInChain);
    if (!SUCCEEDED(status))
    {
        LOG_FUNC_ERROR("ClusterOfSector", status);
        return status;
    }

    tempCluster = currentClusterInChain;

    // this loop will place in currentClusterInChain the last cluster belonging
    // to the parent directory
    while (!FAT32_EOC(tempCluster))
    {
        currentClusterInChain = tempCluster;

        status = NextClusterInChain(FatData, currentClusterInChain, &tempCluster);
        if (!SUCCEEDED(status))
        {
            LOG_FUNC_ERROR("NextClusterInChain", status);
            return status;
        }
    }

    // we found the sector of the last cluster of the parent
    status = FirstSectorOfCluster(FatData, currentClusterInChain, &finalSector);
    if (!SUCCEEDED(status))
    {
        LOG_FUNC_ERROR("FirstSectorOfCluster", status);
        return status;
    }

    // we allocate space for the last cluster
    bytesToRead = FatData->BytesPerSector * FatData->SectorsPerCluster;

    pEntry = (DIR_ENTRY*)ExAllocatePoolWithTag(PoolAllocateZeroMemory, (DWORD)bytesToRead, HEAP_TEMP_TAG, 0);
    if (NULL == pEntry)
    {
        status = STATUS_HEAP_NO_MORE_MEMORY;
        LOG_FUNC_ERROR_ALLOC("HeapAllocatePoolWithTag", bytesToRead);
        return status;
    }

    __try
    {
        status = IoReadDeviceEx(FatData->VolumeDevice,
            pEntry,
            &bytesToRead,
            finalSector * FatData->BytesPerSector,
            TRUE
        );
        if (!SUCCEEDED(status))
        {
            LOG_FUNC_ERROR("IoReadDeviceEx", status);
            __leave;
        }
        ASSERT(bytesToRead == FatData->BytesPerSector * FatData->SectorsPerCluster);

        if (finalSector == FatData->FirstDataSector)
        {
            // root directory => skip first 2 entries
            // (first 2 are reserved and we can't place anything there)
            index = 2;
        }

        // Step 4. Is there space in the current cluster?
        /// TODO check for enough space for long dir entries
        found = FALSE;
        while (index < (FatData->EntriesPerSector * FatData->SectorsPerCluster))
        {
            if ((FREE_ENTRY == pEntry[index].DIR_Name[0]) ||
                (FREE_ALL == pEntry[index].DIR_Name[0]) ||
                (FREE_JAP_ENTRY == pEntry[index].DIR_Name[0]))
            {
                // we found a place where we can place our new entry
                found = TRUE;
                break;
            }
            ++index;
        }

        // There is space => step 7
        // Else => step 5

        if (!found)
        {
            // this part needs to be done

            // we need to add a new cluster to the link because we didn't find any
            // empty space

            // Step 5. Update link to new Cluster from FSI


            // Step 6. Update FreeCount and NextFree info in FSI

            // ATM returns a disk full error so as not to overwrite any previous entry
            status = STATUS_DISK_FULL;
            __leave;
        }


        // Step 7. Add new directory entry in current cluster

        // we zero the memory
        memset(&(pEntry[index]), 0, sizeof(DIR_ENTRY));

        // we set the name of the new entry
        // use memcpy because we don't want NULL terminator afterwards
        memcpy((char*)pEntry[index].DIR_Name, newEntryName, SHORT_NAME_CHARS);

        // we search for the cluster we can use to allocate for the new entry
        if (NULL == pFSinfo)
        {
            bytesToRead = FatData->BytesPerSector;

            ASSERT(bytesToRead <= MAX_DWORD);

            pFSinfo = (FSINFO*)ExAllocatePoolWithTag(PoolAllocateZeroMemory, (DWORD)bytesToRead, HEAP_TEMP_TAG, 0);
            if (NULL == pFSinfo)
            {
                status = STATUS_HEAP_NO_MORE_MEMORY;
                LOG_FUNC_ERROR_ALLOC("HeapAllocatePoolWithTag", bytesToRead);
                __leave;
            }

            status = IoReadDevice(FatData->VolumeDevice,
                pFSinfo,
                &bytesToRead,
                1 * FatData->BytesPerSector
            );
            if (!SUCCEEDED(status))
            {
                LOG_FUNC_ERROR("IoReadDevice", status);
                __leave;
            }
            ASSERT(bytesToRead == FatData->BytesPerSector);
        }

        if (0 == pFSinfo->FSI_Free_Count)
        {
            // if there are no more free clusters we fail
            status = STATUS_DISK_FULL;
            __leave;
        }
        // HERE we should check the FSI_Free_Count for 0xFFFFFFFF and if so we should recompute the
        // free count and recheck it for 0

        // we set the directory cluster (should check against 0xFFFFFFFF) and if so we should
        // start looking for clusters starting form cluster 2

        // we set the cluster where the directory entry's data will be placed
        pEntry[index].DIR_FstClusHI = DWORD_HIGH(pFSinfo->FSI_Nxt_Free);
        pEntry[index].DIR_FstClusLO = DWORD_LOW(pFSinfo->FSI_Nxt_Free);

        currentClusterInChain = pFSinfo->FSI_Nxt_Free;

        // set the new file attributes
        pEntry[index].DIR_Attr = FileAttributes;
        pEntry[index].DIR_FileSize = 0; // redundant because of MemSet to 0

        // get current date time
        crtDateTime = IoGetCurrentDateTime();

        // we convert the date read to a date which can be written to the
        // DIR_ENTRY
        ConvertDateTimeToFatDateTime(&crtDateTime, &fatDate, &fatTime);

        // We set the creation, write and last access date's and time
        // to the current time
        pEntry[index].DIR_CrtDate = fatDate;
        pEntry[index].DIR_WrtDate = fatDate;
        pEntry[index].DIR_LstAccDate = fatDate;

        pEntry[index].DIR_CrtTime = fatTime;
        pEntry[index].DIR_WrtTime = fatTime;


        // Step 9. Update FSI information FreeCount and NextFree

        // decrement free count
        pFSinfo->FSI_Free_Count--;

        // update next free cluster(should check if cluster is actually free)
        pFSinfo->FSI_Nxt_Free++;

        // we write the changes made to pFSinfo and to the parent cluster of the new entry
        bytesToRead = FatData->BytesPerSector;
        status = IoWriteDevice(FatData->VolumeDevice,
            pFSinfo,
            &bytesToRead,
            1 * FatData->BytesPerSector
        );
        if (!SUCCEEDED(status))
        {
            LOG_FUNC_ERROR("IoWriteDevice", status);
            __leave;
        }
        ASSERT(bytesToRead == FatData->BytesPerSector);

        bytesToRead = FatData->BytesPerSector * FatData->SectorsPerCluster;
        status = IoWriteDevice(FatData->VolumeDevice,
            pEntry,
            &bytesToRead,
            finalSector * FatData->BytesPerSector
        );
        if (!SUCCEEDED(status))
        {
            LOG_FUNC_ERROR("IoWriteDevice", status);
            __leave;
        }
        ASSERT(bytesToRead == FatData->BytesPerSector * FatData->SectorsPerCluster);

        // zero the memory where the new cluster was placed
        ExFreePoolWithTag(pEntry, HEAP_TEMP_TAG);
        pEntry = NULL;

        // we allocate data for the new DIR_ENTRY
        bytesToRead = FatData->BytesPerSector * FatData->SectorsPerCluster;
        pEntry = (DIR_ENTRY*)ExAllocatePoolWithTag(PoolAllocateZeroMemory, (DWORD)bytesToRead, HEAP_TEMP_TAG, 0);
        if (NULL == pEntry)
        {
            status = STATUS_HEAP_NO_MORE_MEMORY;
            LOG_FUNC_ERROR_ALLOC("HeapAllocatePoolWithTag", bytesToRead);
            __leave;
        }

        // malloc already zeroes memory so no need to do it

        // Step 10. Is new entry a directory?
        // It is not => step 12
        if (ATTR_DIRECTORY == (FileAttributes & (ATTR_DIRECTORY | ATTR_VOLUME_ID)))
        {
            // If it is a directory => step 11

            // Step 11. Create "." and ".." entries
            int i;

            // Set the name of the "." and ".." entries
            memcpy((char*)pEntry[DOT_ENTRY_INDEX].DIR_Name, DOT_ENTRY_NAME, SHORT_NAME_CHARS);
            memcpy((char*)pEntry[DOT_DOT_ENTRY_INDEX].DIR_Name, DOT_DOT_ENTRY_NAME, SHORT_NAME_CHARS);

            // set their attributes and date and time
            for (i = 0; i < FAT_DIR_NO_DEFAULT_ENTRIES; ++i)
            {
                pEntry[i].DIR_Attr = ATTR_DIRECTORY;
                pEntry[i].DIR_CrtTime = pEntry[i].DIR_WrtTime = fatTime;
                pEntry[i].DIR_CrtDate = pEntry[i].DIR_WrtDate = pEntry[i].DIR_LstAccDate = fatDate;
            }


            if (FatData->FirstDataSector == parentSector)
            {
                // => parent is root directory => we need to set the cluster value to 0
                pEntry[DOT_DOT_ENTRY_INDEX].DIR_FstClusHI = pEntry[DOT_DOT_ENTRY_INDEX].DIR_FstClusLO = 0;
            }
            else
            {
                // if the parent is not root directory we need to set the actual cluster value
                status = ClusterOfSector(FatData, parentSector, &tempCluster);
                if (!SUCCEEDED(status))
                {
                    LOG_FUNC_ERROR("ClusterOfSector", status);
                    __leave;
                }

                // we set to the ".." entry a reference to it's parent
                pEntry[DOT_DOT_ENTRY_INDEX].DIR_FstClusHI = DWORD_HIGH(tempCluster);
                pEntry[DOT_DOT_ENTRY_INDEX].DIR_FstClusLO = DWORD_LOW(tempCluster);
            }

            // we set to the "." entry a reference to itself
            pEntry[DOT_ENTRY_INDEX].DIR_FstClusHI = DWORD_HIGH(currentClusterInChain);
            pEntry[DOT_ENTRY_INDEX].DIR_FstClusLO = DWORD_LOW(currentClusterInChain);
        }

        // we find the sector in which we added the new DIR_ENTRY contents
        status = FirstSectorOfCluster(FatData, currentClusterInChain, &sectorAllocated);
        if (!SUCCEEDED(status))
        {
            LOG_FUNC_ERROR("FirstSectorOfCluster", status);
            __leave;
        }

        status = IoWriteDevice(FatData->VolumeDevice,
            pEntry,
            &bytesToRead,
            sectorAllocated * FatData->BytesPerSector
        );
        if (!SUCCEEDED(status))
        {
            LOG_FUNC_ERROR("IoWriteDevice", status);
            __leave;
        }
        ASSERT(bytesToRead == FatData->BytesPerSector * FatData->SectorsPerCluster);

        // Step 12. Add EOC for new cluster used

        // we allocate space for the FAT
        bytesToRead = FatData->BytesPerSector;
        pFAT = (BYTE*)ExAllocatePoolWithTag(PoolAllocateZeroMemory, (DWORD)bytesToRead, HEAP_TEMP_TAG, 0);
        if (NULL == pFAT)
        {
            status = STATUS_HEAP_NO_MORE_MEMORY;
            LOG_FUNC_ERROR_ALLOC("HeapAllocatePoolWithTag", bytesToRead);
            __leave;
        }

        // we find the sector and the offset of the FAT
        fatEntrySector = FatData->ReservedSectors + (currentClusterInChain * sizeof(DWORD)) / FatData->BytesPerSector;
        fatEntryOffset = (currentClusterInChain * sizeof(DWORD)) % FatData->BytesPerSector;


        status = IoReadDevice(FatData->VolumeDevice,
            pFAT,
            &bytesToRead,
            fatEntrySector * FatData->BytesPerSector
        );
        if (!SUCCEEDED(status))
        {
            LOG_FUNC_ERROR("IoReadDevice", status);
            __leave;
        }
        ASSERT(bytesToRead == FatData->BytesPerSector);


        *((DWORD*)&(pFAT[fatEntryOffset])) = *((DWORD*)&(pFAT[fatEntryOffset])) & (~FAT32_CLUSTER_MASK);    // we need to preserve the 4 reserved bits
        *((DWORD*)&(pFAT[fatEntryOffset])) = *((DWORD*)&(pFAT[fatEntryOffset])) | FAT32_EOC_MARK;            // we add the EOC mark


        status = IoWriteDevice(FatData->VolumeDevice,
            pFAT,
            &bytesToRead,
            fatEntrySector * FatData->BytesPerSector
        );
        if (!SUCCEEDED(status))
        {
            LOG_FUNC_ERROR("IoWriteDevice", status);
            __leave;
        }
        ASSERT(bytesToRead == FatData->BytesPerSector);
    }
    __finally
    {
        if (NULL != pFAT)
        {
            ExFreePoolWithTag(pFAT, HEAP_TEMP_TAG);
            pFAT = NULL;
        }

        if (NULL != pFSinfo)
        {
            ExFreePoolWithTag(pFSinfo, HEAP_TEMP_TAG);
            pFSinfo = NULL;
        }

        if (NULL != pEntry)
        {
            ExFreePoolWithTag(pEntry, HEAP_TEMP_TAG);
            pEntry = NULL;
        }

        LOG_FUNC_END;
    }

    return status;
}

// warning C6101: Returning uninitialized memory '*DirectoryInformation'.  A successful path through the function does not set the named _Out_ parameter. 

STATUS
FatQueryDirectory(
    IN                                              PFAT_DATA                       FatData,
    IN                                              QWORD                           DirectorySector,
    IN                                              DWORD                           DirectoryInformationSize,
    OUT_WRITES_BYTES(DirectoryInformationSize)      FILE_DIRECTORY_INFORMATION      *DirectoryInformation,
    OUT                                             DWORD*                          RequiredDirectionInformationSize
)
{
    STATUS status;
    LONG_DIR_ENTRY* pLongEntry;
    DIR_ENTRY* pEntry;
    int index;
    QWORD sectorToParse;        // directory sector to parse
    QWORD bytesToRead;
    DWORD requiredSize;
    PFILE_DIRECTORY_INFORMATION pCurEntry;
    PFILE_DIRECTORY_INFORMATION pPrevEntry;

    char longName[LONG_NAME_MAX_CHARS + 1] = { 0 };
    DWORD longNameLength;
    char shortName[SHORT_NAME_MAX_LENGTH] = { 0 };
    DWORD shortNameLength;
    DWORD curStructureSize;

    ASSERT(NULL != FatData);
    ASSERT(NULL != DirectoryInformation || 0 == DirectoryInformationSize);
    ASSERT(NULL != RequiredDirectionInformationSize);

    status = STATUS_SUCCESS;
    pLongEntry = NULL;
    pEntry = NULL;
    index = 0;
    sectorToParse = DirectorySector;
    bytesToRead = 0;
    requiredSize = 0;
    pCurEntry = DirectoryInformation;
    pPrevEntry = NULL;

    longNameLength = 0;
    shortNameLength = 0;

    __try
    {
        do
        {
            if (0 == (index % (FatData->EntriesPerSector  * FatData->SectorsPerCluster)))
            {
                // we need to read from the next sector, we don't care about this one
                // anymore
                if (NULL != pEntry)
                {
                    ExFreePoolWithTag(pEntry, HEAP_TEMP_TAG);
                    pEntry = NULL;

                    // we go to the next sector by following the cluster chain
                    status = NextSectorInClusterChain(FatData, sectorToParse, &sectorToParse, FALSE);
                    if (!SUCCEEDED(status))
                    {
                        LOG_FUNC_ERROR("NextSectorInClusterChain", status);
                        __leave;
                    }

                    if (0 == sectorToParse)
                    {
                        // we have reached the EOC marker => we finished parsing the directory
                        __leave;
                    }
                }

                // we allocate a buffer where to read the current cluster in the chain
                bytesToRead = FatData->BytesPerSector * FatData->SectorsPerCluster;
                pEntry = (DIR_ENTRY*)ExAllocatePoolWithTag(PoolAllocateZeroMemory, (DWORD)bytesToRead, HEAP_TEMP_TAG, 0);
                if (NULL == pEntry)
                {
                    // malloc failed
                    status = STATUS_HEAP_NO_MORE_MEMORY;
                    LOG_FUNC_ERROR_ALLOC("HeapAllocatePoolWithTag", bytesToRead);
                    __leave;
                }

                // we read the next cluster
                status = IoReadDeviceEx(FatData->VolumeDevice,
                    pEntry,
                    &bytesToRead,
                    sectorToParse * FatData->BytesPerSector,
                    TRUE
                );
                if (!SUCCEEDED(status))
                {
                    LOG_FUNC_ERROR("IoReadDeviceEx", status);
                    __leave;
                }
                ASSERT(bytesToRead == FatData->BytesPerSector * FatData->SectorsPerCluster);


                // reseting the index in the DIR_ENTRY array
                index = 0;
            }


            if ((FREE_ENTRY == pEntry[index].DIR_Name[0]) || (FREE_JAP_ENTRY == pEntry[index].DIR_Name[0]))
            {
                // this entry is empty
                index++;        // we increment the current index and we
                continue;        // continue to the next entry
            }

            pLongEntry = (LONG_DIR_ENTRY*)&(pEntry[index]);

            if ((pLongEntry->LDIR_Attr & ATTR_LONG_NAME_MASK) == ATTR_LONG_NAME)
            {
                status = WritePartialLongFatNameToName(pLongEntry, LONG_NAME_MAX_CHARS + 1, longName);
                ASSERT(SUCCEEDED(status));

                index++;
                continue;
            }

            // short directory entry

            status = ConvertFatNameToName((char*)pEntry[index].DIR_Name, SHORT_NAME_MAX_LENGTH, shortName, &shortNameLength);
            ASSERT(SUCCEEDED(status));

            longNameLength = strlen_s(longName, LONG_NAME_MAX_CHARS);
            if (0 == longNameLength)
            {
                longNameLength = shortNameLength;
                strncpy(longName, shortName, shortNameLength);
            }

            curStructureSize = sizeof(FILE_DIRECTORY_INFORMATION) + longNameLength;

            requiredSize = requiredSize + curStructureSize;

            if (requiredSize <= DirectoryInformationSize)
            {
                if (NULL != pPrevEntry)
                {
                    // update previous entry offset
                    pPrevEntry->NextEntryOffset = (DWORD)((PBYTE)pCurEntry - (PBYTE)pPrevEntry);
                }

                LOG_TRACE_FILESYSTEM("[%d]: [%s][%x]\n", index, pEntry[index].DIR_Name, pEntry[index].DIR_Attr);
                LOG_TRACE_FILESYSTEM("File short name: [%s]\n", shortName);
                LOG_TRACE_FILESYSTEM("File long  name: [%s]\n", longName);

                // populate current entry

                // we set offset to 0 because we don't know if there are any entries
                // after us
                pCurEntry->NextEntryOffset = 0;

                _FatPopulateFileInformationFromFatEntry(FatData, &pEntry[index], &pCurEntry->BasicFileInformation);
                memcpy(pCurEntry->ShortFilename, shortName, shortNameLength);

                pCurEntry->FilenameLength = longNameLength;
                memcpy(pCurEntry->Filename, longName, longNameLength);

                // update previous and current entry
                pPrevEntry = pCurEntry;
                pCurEntry = (PFILE_DIRECTORY_INFORMATION)((PBYTE)pCurEntry + curStructureSize);
            }

            // reset long name
            // in case the next short directory entry does not have associated long name entries
            // it must not use the long name from a previous entry
            longName[0] = '\0';


            // we increase the index in the entry array
            index++;

        } while (FREE_ALL != pEntry[index].DIR_Name[0]);
    }
    __finally
    {
        if (NULL != pEntry)
        {
            ExFreePoolWithTag(pEntry, HEAP_TEMP_TAG);
            pEntry = NULL;
        }

        *RequiredDirectionInformationSize = requiredSize;
    }

    return status;
}
