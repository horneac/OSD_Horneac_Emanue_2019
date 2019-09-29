#include "ata_base.h"
#include "ata_dispatch.h"
#include "ata_operations.h"

#define LBA48_MAX_VALUE                 0x0000'FFFF'FFFF'FFFFULL

__forceinline
static
STATUS
_AtaCheckAlignment(
    IN                                          QWORD           Size,
    IN                                          QWORD           Offset
    )
{
    if (!IsAddressAligned(Size, SECTOR_SIZE))
    {
        return STATUS_DEVICE_DATA_ALIGNMENT_ERROR;
    }

    if (!IsAddressAligned(Offset, SECTOR_SIZE))
    {
        return STATUS_DEVICE_DATA_ALIGNMENT_ERROR;
    }

    return STATUS_SUCCESS;
}

__forceinline
static
STATUS
_AtaCheckIOParameters(
    IN                                          PATA_DEVICE     Device,
    IN                                          QWORD           SectorIndex,
    IN                                          WORD            SectorCount
    )
{
    ASSERT(NULL != Device);

    if (!Device->Initialized)
    {
        return STATUS_DEVICE_NOT_INITIALIZED;
    }

    if (SectorIndex >= Device->TotalSectors)
    {
        // how can we read at an index higher than our total sector count?
        return STATUS_DEVICE_SECTOR_OFFSET_EXCEEDED;
    }

    if (SectorIndex >= LBA48_MAX_VALUE)
    {
        // sector index is only a 48-bit value
        return STATUS_DEVICE_SECTOR_OFFSET_EXCEEDED;
    }

    if (Device->TotalSectors - SectorIndex < SectorCount)
    {
        // sorry, we really don't have that much
        return STATUS_DEVICE_SECTOR_COUNT_EXCEEDED;
    }

    return STATUS_SUCCESS;
}

STATUS
(__cdecl AtaDispatchReadWrite)(
    INOUT       PDEVICE_OBJECT      DeviceObject,
    INOUT       PIRP                Irp
    )
{
    PATA_DEVICE pAtaDevice;
    QWORD sectorIndex;
    QWORD sectorCount;
    PIO_STACK_LOCATION pStackLocation;
    STATUS status;
    QWORD sizeInBytes;
    QWORD offset;
    WORD sectorsRead;
    BOOLEAN writeOperation;

    ASSERT(NULL != DeviceObject);
    ASSERT(NULL != Irp);

    pAtaDevice = NULL;
    sectorIndex = 0;
    sectorCount = 0;
    pStackLocation = NULL;
    status = STATUS_SUCCESS;
    sizeInBytes = 0;
    offset = 0;
    sectorsRead = 0;
    writeOperation = FALSE;

    pAtaDevice = IoGetDeviceExtension(DeviceObject);
    ASSERT(NULL != pAtaDevice);

    pStackLocation = IoGetCurrentIrpStackLocation(Irp);

    ASSERT(IRP_MJ_READ == pStackLocation->MajorFunction || IRP_MJ_WRITE == pStackLocation->MajorFunction);
    writeOperation = IRP_MJ_WRITE == pStackLocation->MajorFunction;

    sizeInBytes = pStackLocation->Parameters.ReadWrite.Length;
    offset = pStackLocation->Parameters.ReadWrite.Offset;

    LOG_TRACE_STORAGE("Offset: 0x%X\n", offset);
    LOG_TRACE_STORAGE("Size in bytes: 0x%x\n", sizeInBytes);

    __try
    {
        status = _AtaCheckAlignment(sizeInBytes, offset);
        if (!SUCCEEDED(status))
        {
            __leave;
        }

        LOG_TRACE_STORAGE("Sizes are properly aligned to sector size\n");

        sectorIndex = offset / SECTOR_SIZE;
        sectorCount = sizeInBytes / SECTOR_SIZE;

        LOG_TRACE_STORAGE("Sector index, Sector count: 0x%X, 0x%X\n", sectorIndex, sectorCount);

        if (sectorCount > MAX_WORD)
        {
            status = STATUS_DEVICE_SECTOR_COUNT_EXCEEDED;
            __leave;
        }

        status = _AtaCheckIOParameters(pAtaDevice, sectorIndex, (WORD)sectorCount);
        if (!SUCCEEDED(status))
        {
            __leave;
        }

        LOG_TRACE_STORAGE("IO parameters are valid\n");
        LOG_TRACE_STORAGE("Sector index: 0x%X\n", sectorIndex);
        LOG_TRACE_STORAGE("Sector count: 0x%X\n", sectorCount);

        ASSERT(AtaTransferStateFree == _InterlockedCompareExchange(&pAtaDevice->CurrentTransfer.State, AtaTransferStateInProgress, AtaTransferStateFree));

        status = AtaReadWriteSectors(pAtaDevice, sectorIndex, (WORD)sectorCount, Irp->Buffer, &sectorsRead, (BOOLEAN)Irp->Flags.Asynchronous, writeOperation);

        _InterlockedExchange(&pAtaDevice->CurrentTransfer.State, AtaTransferStateFree);
    }
    __finally
    {
        Irp->IoStatus.Status = status;
        Irp->IoStatus.Information = sectorsRead * SECTOR_SIZE;

        // complete IRP
        IoCompleteIrp(Irp);
        Irp = NULL;
    }

    return STATUS_SUCCESS;
}

STATUS
(__cdecl AtaDispatchDeviceControl)(
    INOUT       PDEVICE_OBJECT      DeviceObject,
    INOUT       PIRP                Irp
    )
{
    STATUS status;
    PIO_STACK_LOCATION pStackLocation;
    DWORD information;
    PATA_DEVICE pAtaDevice;

    ASSERT(NULL != DeviceObject);
    ASSERT(NULL != Irp);

    status = STATUS_SUCCESS;
    pStackLocation = IoGetCurrentIrpStackLocation(Irp);
    information = 0;
    pAtaDevice = IoGetDeviceExtension(DeviceObject);
    ASSERT(NULL != pAtaDevice);

    ASSERT(IRP_MJ_DEVICE_CONTROL == pStackLocation->MajorFunction);

    switch (pStackLocation->Parameters.DeviceControl.IoControlCode)
    {
    case IOCTL_DISK_GET_LENGTH_INFO:
        {
            GET_LENGTH_INFORMATION result;

            information = sizeof(GET_LENGTH_INFORMATION);
            memzero(&result, information);

            if (pStackLocation->Parameters.DeviceControl.OutputBufferLength < information)
            {
                status = STATUS_BUFFER_TOO_SMALL;
                break;
            }

            result.Length = pAtaDevice->TotalSectors * SECTOR_SIZE;

            // copy result
            memcpy(pStackLocation->Parameters.DeviceControl.OutputBuffer, &result, information);
        }
        break;
    default:
        status = STATUS_UNSUPPORTED;
    }

    Irp->IoStatus.Status = status;
    Irp->IoStatus.Information = information;

    return STATUS_SUCCESS;
}