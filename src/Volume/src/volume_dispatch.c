#include "volume_base.h"
#include "volume_dispatch.h"

__forceinline
static
STATUS
_VolCheckIOParameters(
    IN                                          PVOLUME         Volume,
    IN                                          QWORD           SectorIndex,
    IN                                          QWORD           SectorCount
    )
{
    ASSERT(NULL != Volume);

    if (SectorIndex >= Volume->PartitionInformation.PartitionSize)
    {
        // how can we read at an index higher than our total sector count?
        return STATUS_DEVICE_SECTOR_OFFSET_EXCEEDED;
    }

    if (Volume->PartitionInformation.PartitionSize - SectorIndex < SectorCount)
    {
        // sorry, we really don't have that much
        return STATUS_DEVICE_SECTOR_COUNT_EXCEEDED;
    }

    return STATUS_SUCCESS;
}

STATUS
(__cdecl VolDispatchReadWrite)(
    INOUT       PDEVICE_OBJECT      DeviceObject,
    INOUT       PIRP                Irp
    )
{
    STATUS status;
    PVOLUME pVolume;
    PIO_STACK_LOCATION pStackLocation;
    QWORD lengthInSectors;
    QWORD offsetInSectors;

    ASSERT(NULL != DeviceObject);
    ASSERT(NULL != Irp);

    status = STATUS_SUCCESS;
    pVolume = NULL;
    pStackLocation = NULL;
    lengthInSectors = 0;
    offsetInSectors = 0;

    pVolume = IoGetDeviceExtension(DeviceObject);
    ASSERT(NULL != pVolume);

    // validate parameters
    pStackLocation = IoGetCurrentIrpStackLocation(Irp);
    ASSERT(IRP_MJ_READ == pStackLocation->MajorFunction || IRP_MJ_WRITE == pStackLocation->MajorFunction);

    lengthInSectors = pStackLocation->Parameters.ReadWrite.Length / SECTOR_SIZE;
    offsetInSectors = pStackLocation->Parameters.ReadWrite.Offset / SECTOR_SIZE;

    __try
    {
        status = _VolCheckIOParameters(pVolume, offsetInSectors, lengthInSectors);
        if (!SUCCEEDED(status))
        {
            __leave;
        }

        // copy stack location
        // this function also advances the current stack location
        IoCopyCurrentStackLocationToNext(Irp);

        // we need to modify offset to be disk relative
        pStackLocation = IoGetNextIrpStackLocation(Irp);

        pStackLocation->Parameters.ReadWrite.Offset = (offsetInSectors + pVolume->PartitionInformation.OffsetInDisk) * SECTOR_SIZE;

        LOG_TRACE_STORAGE("Copied current IRP stack location\n");

        // call disk device
        status = IoCallDriver(pVolume->DiskDevice, Irp);
        if (!SUCCEEDED(status))
        {
            LOG_FUNC_ERROR("IoCalLDriver", status);
            __leave;
        }
        Irp = NULL;
    }
    __finally
    {
        if (!SUCCEEDED(status))
        {
            // if we succeeded => lower driver will set status and information
            Irp->IoStatus.Status = status;

            IoCompleteIrp(Irp);
            Irp = NULL;
        }

        ASSERT(NULL == Irp);
    }

    return STATUS_SUCCESS;
}

STATUS
(__cdecl VolDispatchDeviceControl)(
    INOUT       PDEVICE_OBJECT      DeviceObject,
    INOUT       PIRP                Irp
    )
{
    STATUS status;
    PVOLUME pVolume;
    PIO_STACK_LOCATION pStackLocation;
    QWORD lengthInSectors;
    QWORD offsetInSectors;
    DWORD information;

    ASSERT(NULL != DeviceObject);
    ASSERT(NULL != Irp);

    status = STATUS_SUCCESS;
    pVolume = NULL;
    pStackLocation = NULL;
    lengthInSectors = 0;
    offsetInSectors = 0;
    information = 0;

    pVolume = IoGetDeviceExtension(DeviceObject);
    ASSERT(NULL != pVolume);

    // validate parameters
    pStackLocation = IoGetCurrentIrpStackLocation(Irp);
    ASSERT(IRP_MJ_DEVICE_CONTROL == pStackLocation->MajorFunction);

    switch (pStackLocation->Parameters.DeviceControl.IoControlCode)
    {
    case IOCTL_VOLUME_PARTITION_INFO:
        {
            information = sizeof(PARTITION_INFORMATION);

            if (pStackLocation->Parameters.DeviceControl.OutputBufferLength < information)
            {
                status = STATUS_BUFFER_TOO_SMALL;
                break;
            }

            memcpy(pStackLocation->Parameters.DeviceControl.OutputBuffer, &pVolume->PartitionInformation, information);
        }
        break;
    default:
        status = STATUS_UNSUPPORTED;
    }

    IoCompleteIrp(Irp);
    Irp->IoStatus.Status = status;
    Irp->IoStatus.Information = information;

    return STATUS_SUCCESS;
}