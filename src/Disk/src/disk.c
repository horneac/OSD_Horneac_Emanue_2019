#include "disk_base.h"
#include "disk.h"
#include "mbr.h"
#include "disk_dispatch.h"

static
STATUS
_DiskInitialize(
    IN      PDRIVER_OBJECT          DriverObject,
    IN      PDEVICE_OBJECT          HardDiskControllerDevice
    );

static
STATUS
_DiskRetrievePartitionsFromDisk(
    INOUT   PDEVICE_OBJECT          DiskDevice
    );

static
STATUS
_DiskRetrievePartitionsFromDiskStartingAtOffset(
    INOUT   PDEVICE_OBJECT              DiskDevice,
    IN      DWORD                       DiskOffset,
    IN      BOOLEAN                     ExtendedPartition,
    IN      DWORD                       FirstExtendedPartitionLBA,
    INOUT   DWORD*                      PartitionCount,
    OUT_OPT PDISK_LAYOUT_INFORMATION    DiskLayoutInformation
    );

STATUS
(__cdecl DiskDriverEntry)(
    INOUT       PDRIVER_OBJECT      DriverObject
    )
{
    STATUS status;
    DWORD disksFound;
    PDEVICE_OBJECT* pHardDiskControllerDevices;
    DWORD numberOfDevices;
    DWORD i;

    LOG_FUNC_START;

    ASSERT(NULL != DriverObject);

    status = STATUS_SUCCESS;
    disksFound = 0;
    pHardDiskControllerDevices = NULL;
    numberOfDevices = 0;

    DriverObject->DispatchFunctions[IRP_MJ_READ] = DiskDispatchReadWrite;
    DriverObject->DispatchFunctions[IRP_MJ_WRITE] = DiskDispatchReadWrite;
    DriverObject->DispatchFunctions[IRP_MJ_DEVICE_CONTROL] = DiskDispatchDeviceControl;

    status = IoGetDevicesByType(DeviceTypeHarddiskController, &pHardDiskControllerDevices, &numberOfDevices);
    if (!SUCCEEDED(status))
    {
        LOG_FUNC_ERROR("IoGetDeviceByType", status);
        return status;
    }
    ASSERT(numberOfDevices == 0 || pHardDiskControllerDevices != NULL);

    for (i = 0; i < numberOfDevices; ++i)
    {
        ASSERT(pHardDiskControllerDevices[i] != NULL);

        status = _DiskInitialize(DriverObject,
                                 pHardDiskControllerDevices[i]
                                 );
        if (!SUCCEEDED(status))
        {
            LOG_FUNC_ERROR("_DiskInitialize", status );
            continue;
        }

        // this should be done only at the end of the function,
        // else we may fail and the diskFound would not actually be found :)
        disksFound = disksFound + 1;
    }

    if (NULL != pHardDiskControllerDevices)
    {
        IoFreeTemporaryData(pHardDiskControllerDevices);
        pHardDiskControllerDevices = NULL;
    }

    if( disksFound > 0 )
    {
        LOG("Found %d disks\n", disksFound);
        status = STATUS_SUCCESS;
    }

    LOG_FUNC_END;

    return status;
}

static
STATUS
_DiskInitialize(
    IN      PDRIVER_OBJECT          DriverObject,
    IN      PDEVICE_OBJECT          HardDiskControllerDevice
    )
{
    STATUS status;
    PDEVICE_OBJECT pDiskDevice;
    PDISK_OBJECT pDiskData;
    GET_LENGTH_INFORMATION lengthInformation;
    PIRP pIrp;

    ASSERT(NULL != DriverObject);
    ASSERT(NULL != HardDiskControllerDevice);

    pDiskDevice = NULL;
    pDiskData = NULL;
    memzero(&lengthInformation, sizeof(GET_LENGTH_INFORMATION));
    pIrp = NULL;
    status = STATUS_SUCCESS;

    __try
    {
        // send an IOCTL to see device description
        pIrp = IoBuildDeviceIoControlRequest(IOCTL_DISK_GET_LENGTH_INFO,
                                             HardDiskControllerDevice,
                                             NULL,
                                             0,
                                             &lengthInformation,
                                             sizeof(GET_LENGTH_INFORMATION)
        );
        ASSERT(NULL != pIrp);

        status = IoCallDriver(HardDiskControllerDevice, pIrp);
        if (!SUCCEEDED(status))
        {
            LOG_FUNC_ERROR("IoCallDriver", status);
            __leave;
        }

        if (!SUCCEEDED(pIrp->IoStatus.Status))
        {
            LOG_FUNC_ERROR("IoCallDriver", status);
            __leave;
        }

        LOG("Succesfully called device with IOCTL IOCTL_DISK_GET_LENGTH_INFO\n");

        pDiskDevice = IoCreateDevice(DriverObject, sizeof(DISK_OBJECT), DeviceTypeDisk);
        if (NULL == pDiskDevice)
        {
            LOG_FUNC_ERROR_ALLOC("IoCreateDevice", sizeof(DISK_OBJECT));
            status = STATUS_DEVICE_COULD_NOT_BE_CREATED;
            __leave;
        }
        pDiskDevice->DeviceAlignment = HardDiskControllerDevice->DeviceAlignment;

        pDiskData = IoGetDeviceExtension(pDiskDevice);
        ASSERT(NULL != pDiskData);

        IoAttachDevice(pDiskDevice, HardDiskControllerDevice);

        pDiskData->NumberOfSectors = lengthInformation.Length / SECTOR_SIZE;
        pDiskData->DiskDeviceController = HardDiskControllerDevice;

        status = _DiskRetrievePartitionsFromDisk(pDiskDevice);
        if (!SUCCEEDED(status))
        {
            LOG_FUNC_ERROR("_DiskRetrievePartitionsFromDisk", status);
            __leave;
        }
        LOGL("_DiskRetrievePartitionsFromDisk succceded\n");

        pDiskDevice = NULL;
        pDiskData = NULL;
    }
    __finally
    {
        if (NULL != pDiskDevice)
        {
            IoDeleteDevice(pDiskDevice);
            pDiskDevice = NULL;
        }

        if (NULL != pIrp)
        {
            IoFreeIrp(pIrp);
            pIrp = NULL;
        }
    }

    return status;
}

static
STATUS
_DiskRead(
    IN                                          PDEVICE_OBJECT  DiskDevice,
    IN                                          QWORD           SectorIndex,
    IN                                          WORD            SectorCount,
    OUT_WRITES_BYTES(SectorCount*SECTOR_SIZE)   PVOID           Buffer,
    OUT                                         WORD*           SectorsRead,
    IN                                          BOOLEAN         Asynchronous
    )
{
    STATUS status;
    PIRP pIrp;
    PIO_STACK_LOCATION pStackLocation;
    QWORD byteOffset;

    if (NULL == DiskDevice)
    {
        return STATUS_INVALID_PARAMETER1;
    }

    LOG_FUNC_START;

    status = STATUS_SUCCESS;
    pIrp = NULL;
    pStackLocation = NULL;
    byteOffset = 0;

    LOGL("Disk read with sector index 0x%X, sector count 0x%x\n", SectorIndex, SectorCount);

    __try
    {
        if (SectorIndex >= MAX_QWORD / SECTOR_SIZE)
        {
            status = STATUS_DEVICE_SECTOR_OFFSET_EXCEEDED;
            __leave;
        }

        pIrp = IoAllocateIrp(DiskDevice->StackSize);
        ASSERT(NULL != pIrp);

        // setup next stack location
        pStackLocation = IoGetNextIrpStackLocation(pIrp);

        pStackLocation->MajorFunction = IRP_MJ_READ;
        pStackLocation->Parameters.ReadWrite.Length = SectorCount * SECTOR_SIZE;

        pStackLocation->Parameters.ReadWrite.Offset = SectorIndex * SECTOR_SIZE;

        pIrp->Buffer = Buffer;
        pIrp->Flags.Asynchronous = Asynchronous;

        status = IoCallDriver(DiskDevice, pIrp);
        if (!SUCCEEDED(status))
        {
            LOG_FUNC_ERROR("IoCallDriver", status);
            __leave;
        }

        *SectorsRead = (WORD)(pIrp->IoStatus.Information / SECTOR_SIZE);
        status = pIrp->IoStatus.Status;
    }
    __finally
    {
        if (NULL != pIrp)
        {
            IoFreeIrp(pIrp);
            pIrp = NULL;
        }

        LOG_FUNC_END;
    }

    return status;
}

static
STATUS
_DiskRetrievePartitionsFromDisk(
    INOUT   PDEVICE_OBJECT          DiskDevice
    )
{
    STATUS status;
    DWORD partitionCount;
    PDISK_LAYOUT_INFORMATION pDiskInformation;
    DWORD structureSize;
    PDISK_OBJECT pDisk;

    ASSERT(NULL != DiskDevice);

    LOG_FUNC_START;

    status = STATUS_SUCCESS;
    partitionCount = 0;
    pDiskInformation = NULL;
    structureSize = 0;
    pDisk = IoGetDeviceExtension(DiskDevice);

    ASSERT(NULL != pDisk);

    status = _DiskRetrievePartitionsFromDiskStartingAtOffset(DiskDevice, 0, FALSE, 0, &partitionCount, NULL);
    if (!SUCCEEDED(status))
    {
        LOG_FUNC_ERROR("_DiskRetrievePartitionsFromDiskStartingAtOffset", status);
        return status;
    }

    // now that we now the partition count we can allocate the structure
    structureSize = sizeof(DISK_LAYOUT_INFORMATION) + partitionCount * sizeof(PARTITION_INFORMATION);

    LOG("Allocate structure for %d partitions\n", partitionCount);

    pDiskInformation = ExAllocatePoolWithTag(PoolAllocateZeroMemory, structureSize, HEAP_DISK_TAG, 0);
    if (NULL == pDiskInformation)
    {
        status = STATUS_HEAP_NO_MORE_MEMORY;
        LOG_FUNC_ERROR_ALLOC("HeapAllocatePoolWithTag", structureSize);
        return status;
    }

    partitionCount = 0;

    LOG("Will now call _DiskRetrievePartitionsFromDiskStartingAtOffset to complete pDiskInformation structure\n");

    status = _DiskRetrievePartitionsFromDiskStartingAtOffset(DiskDevice, 0, FALSE, 0, &partitionCount, pDiskInformation);
    if (!SUCCEEDED(status))
    {
        LOG_FUNC_ERROR("_DiskRetrievePartitionsFromDiskStartingAtOffset", status);
        return status;
    }
    pDiskInformation->NumberOfPartitions = partitionCount;

    pDisk->DiskLayout = pDiskInformation;

    LOG_FUNC_END;

    return status;
}

static
STATUS
_DiskRetrievePartitionsFromDiskStartingAtOffset(
    INOUT   PDEVICE_OBJECT              DiskDevice,
    IN      DWORD                       DiskOffset,
    IN      BOOLEAN                     ExtendedPartition,
    IN      DWORD                       FirstExtendedPartitionLBA,
    INOUT   DWORD*                      PartitionCount,
    OUT_OPT PDISK_LAYOUT_INFORMATION    DiskLayoutInformation
    )
{
    STATUS status;
    PMBR pMbr;
    WORD sectorsRead;
    DWORD currentPartition;
    PMBR_PARTITION_ENTRY pCurPartitionEntry;
    BOOLEAN extendedPartitionEntry;
    DWORD entriesToIterateOver;
    DWORD firstPartitionLBA;
    PDISK_OBJECT pDiskObject;
    DWORD partitionCount;

    LOG_FUNC_START;

    ASSERT(NULL != DiskDevice);

    ASSERT_INFO(ExtendedPartition ^ (0 == FirstExtendedPartitionLBA), "If we're in an extended partition the LBA to the first extended partition cannot be 0\n");

    ASSERT(NULL != PartitionCount);

    status = STATUS_SUCCESS;
    pMbr = NULL;
    sectorsRead = 0;
    currentPartition = 0;
    pCurPartitionEntry = NULL;
    extendedPartitionEntry = FALSE;
    entriesToIterateOver = ExtendedPartition ? MBR_EXT_PARTITION_NO_OF_ENTRIES : MBR_NO_OF_PARTITIONS;
    firstPartitionLBA = 0;
    partitionCount = *PartitionCount;
    pDiskObject = IoGetDeviceExtension(DiskDevice);
    ASSERT(NULL != pDiskObject);

    __try
    {
        pMbr = ExAllocatePoolWithTag(PoolAllocateZeroMemory, sizeof(MBR), HEAP_TEMP_TAG, 0);
        if (NULL == pMbr)
        {
            LOG_FUNC_ERROR_ALLOC("HeapAllocatePoolWithTag", sizeof(MBR));
            status = STATUS_HEAP_INSUFFICIENT_RESOURCES;
            __leave;
        }

        // read the MBR
        status = _DiskRead(DiskDevice, DiskOffset, 1, pMbr, &sectorsRead, FALSE);
        if (!SUCCEEDED(status))
        {
            LOG_FUNC_ERROR("DiskRead", status);
            __leave;
        }
        ASSERT_INFO(1 == sectorsRead, "Number of sectors read: %d\n", sectorsRead);

        // we read the MBR, time to parse it
        if (MBR_BOOT_SIGNATURE != pMbr->BootSignature)
        {
            LOG_WARNING("There is no MBR signature to be found\n");
            status = STATUS_DISK_MBR_NOT_PRESENT;
            __leave;
        }

        for (currentPartition = 0; currentPartition < entriesToIterateOver; ++currentPartition)
        {
            pCurPartitionEntry = &pMbr->Partitions[currentPartition];

            if (ExtendedPartition)
            {
                // if this entry contains only zero bytes => it was the last entry
                if (sizeof(MBR_PARTITION_ENTRY) == memscan(pCurPartitionEntry, sizeof(MBR_PARTITION_ENTRY), 0))
                {
                    // we finished our iteration
                    status = STATUS_SUCCESS;
                    __leave;
                }
            }

            if ((MBR_STATUS_ACTIVE != pCurPartitionEntry->Status) &&
                (MBR_STATUS_INACTIVE != pCurPartitionEntry->Status)
                )
            {
                // this partition is not valid
                continue;
            }

            // see if this is an extended partition entry or not
            extendedPartitionEntry = (MBR_PARTITION_EXTENDED_LBA == pCurPartitionEntry->PartitionType) ||
                (MBR_PARTITION_EXTENDED_CHS == pCurPartitionEntry->PartitionType);

            if (extendedPartitionEntry)
            {
                // we have an extended partition entry

                // partition entry, see what is the offsetToTheFirstPartitionEntry
                DWORD offsetToFirstExtendedPartition = ExtendedPartition ? FirstExtendedPartitionLBA : pCurPartitionEntry->FirstSectorLBA;

                // the extended entries are relative to the first extended partition
                firstPartitionLBA = pCurPartitionEntry->FirstSectorLBA + FirstExtendedPartitionLBA;

                // start parsing this extended partition
                status = _DiskRetrievePartitionsFromDiskStartingAtOffset(DiskDevice, firstPartitionLBA, TRUE, offsetToFirstExtendedPartition, &partitionCount, DiskLayoutInformation);
                if (!SUCCEEDED(status))
                {
                    LOG_FUNC_ERROR("_DiskRetrieveVolumesForDiskStartingAtOffset", status);
                }

                // nothing can be found after an extended entry
                __leave;
            }
            else
            {
                // this is not an extended partition entry

                // if we're parsing an extended partition all offsets are relative to this partition
                firstPartitionLBA = pCurPartitionEntry->FirstSectorLBA + DiskOffset;

                // found another partition
                if (NULL != DiskLayoutInformation)
                {
                    // add partition to list

                    DiskLayoutInformation->Partitions[partitionCount].Bootable = MBR_STATUS_ACTIVE == pCurPartitionEntry->Status;
                    DiskLayoutInformation->Partitions[partitionCount].OffsetInDisk = firstPartitionLBA;
                    DiskLayoutInformation->Partitions[partitionCount].PartitionSize = pCurPartitionEntry->NumberOfSectors;
                    DiskLayoutInformation->Partitions[partitionCount].PartitionType = pCurPartitionEntry->PartitionType;
                }

                partitionCount = partitionCount + 1;
            }
        }
    }
    __finally
    {
        *PartitionCount = partitionCount;

        if (NULL != pMbr)
        {
            ExFreePoolWithTag(pMbr, HEAP_TEMP_TAG);
            pMbr = NULL;
        }

        LOG_FUNC_END;
    }

    return status;
}