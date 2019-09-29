#include "disk_base.h"
#include "disk_dispatch.h"

STATUS
(__cdecl DiskDispatchReadWrite)(
    INOUT       PDEVICE_OBJECT      DeviceObject,
    INOUT       PIRP                Irp
    )
{
    STATUS status;
    PDISK_OBJECT pDiskObject;

    ASSERT(NULL != DeviceObject);
    ASSERT(NULL != Irp);

    LOG_FUNC_START;

    status = STATUS_SUCCESS;

    pDiskObject = IoGetDeviceExtension(DeviceObject);
    ASSERT(NULL != pDiskObject);


    // copy stack location
    // this function also advances the current stack location
    IoCopyCurrentStackLocationToNext(Irp);

    LOG_TRACE_STORAGE("Copied current IRP stack location\n");

    // call lower device object
    status = IoCallDriver(pDiskObject->DiskDeviceController, Irp);
    if (!SUCCEEDED(status))
    {
        LOG_FUNC_ERROR("IoCalLDriver", status);
        return status;
    }
    Irp = NULL;

    LOG_FUNC_END;

    return status;
}

STATUS
(__cdecl DiskDispatchDeviceControl)(
    INOUT       PDEVICE_OBJECT      DeviceObject,
    INOUT       PIRP                Irp
    )
{
    STATUS status;
    PDISK_OBJECT pDiskObject;
    PIO_STACK_LOCATION pStackLocation;
    DWORD information;

    ASSERT(NULL != DeviceObject);
    ASSERT(NULL != Irp);

    status = STATUS_SUCCESS;
    pStackLocation = IoGetCurrentIrpStackLocation(Irp);
    pDiskObject = IoGetDeviceExtension(DeviceObject);
    ASSERT(NULL != pDiskObject);

    ASSERT(IRP_MJ_DEVICE_CONTROL == pStackLocation->MajorFunction);

    information = sizeof(DISK_LAYOUT_INFORMATION) + pDiskObject->DiskLayout->NumberOfPartitions * sizeof(PARTITION_INFORMATION);

    switch (pStackLocation->Parameters.DeviceControl.IoControlCode)
    {
    case IOCTL_DISK_LAYOUT_INFO:
        if (pStackLocation->Parameters.DeviceControl.OutputBufferLength < information)
        {
            status = STATUS_BUFFER_TOO_SMALL;
            break;
        }

        memcpy(pStackLocation->Parameters.DeviceControl.OutputBuffer, pDiskObject->DiskLayout, information);
        break;
    default:
        status = STATUS_UNSUPPORTED;
    }

    Irp->IoStatus.Information = information;
    Irp->IoStatus.Status = status;

    return STATUS_SUCCESS;
}