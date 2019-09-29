#include "swapfs_base.h"
#include "swapfs.h"

static FUNC_DriverDispatch     _SwapFsCreate;
static FUNC_DriverDispatch     _SwapFsClose;
static FUNC_DriverDispatch     _SwapFsRead;
static FUNC_DriverDispatch     _SwapFsWrite;

typedef struct _SWAPFS_DATA
{
    PDEVICE_OBJECT              VolumeDevice;

    QWORD                       FileSystemSize;
} SWAPFS_DATA, *PSWAPFS_DATA;

STATUS
(__cdecl SwapFsDriverEntry)(
    INOUT       PDRIVER_OBJECT      DriverObject
    )
{
    STATUS status;
    PDEVICE_OBJECT* pVolumeDevices;
    PDEVICE_OBJECT pCurVolume;
    PARTITION_INFORMATION partitionInformation;
    DWORD i;
    DWORD numberOfDevices;
    PIRP pIrp;
    PDEVICE_OBJECT pFileSystemDevice;
    PSWAPFS_DATA pSwapFsData;

    LOG_FUNC_START;

    ASSERT(NULL != DriverObject);

    status = STATUS_SUCCESS;
    pVolumeDevices = NULL;
    pCurVolume = NULL;
    numberOfDevices = 0;
    pIrp = NULL;
    pFileSystemDevice = NULL;
    pSwapFsData = NULL;

    DriverObject->DispatchFunctions[IRP_MJ_CREATE] = _SwapFsCreate;
    DriverObject->DispatchFunctions[IRP_MJ_CLOSE] = _SwapFsClose;
    DriverObject->DispatchFunctions[IRP_MJ_READ] = _SwapFsRead;
    DriverObject->DispatchFunctions[IRP_MJ_WRITE] = _SwapFsWrite;

    status = IoGetDevicesByType(DeviceTypeVolume, &pVolumeDevices, &numberOfDevices);
    if (!SUCCEEDED(status))
    {
        LOG_FUNC_ERROR("IoGetDeviceByType", status);
        return status;
    }

    __try
    {
        for (i = 0; i < numberOfDevices; ++i)
        {
            pCurVolume = pVolumeDevices[i];

            if (NULL != pFileSystemDevice)
            {
                IoDeleteDevice(pFileSystemDevice);
                pFileSystemDevice = NULL;
            }

            memzero(&partitionInformation, sizeof(PARTITION_INFORMATION));

            if (NULL != pIrp)
            {
                IoFreeIrp(pIrp);
                pIrp = NULL;
            }

            // send an IOCTL to see device description
            pIrp = IoBuildDeviceIoControlRequest(IOCTL_VOLUME_PARTITION_INFO,
                                                 pCurVolume,
                                                 NULL,
                                                 0,
                                                 &partitionInformation,
                                                 sizeof(PARTITION_INFORMATION)
            );
            if (NULL == pIrp)
            {
                LOG_ERROR("IoBuildDeviceIoControlRequest failed\n");
                continue;
            }

            status = IoCallDriver(pCurVolume, pIrp);
            if (!SUCCEEDED(status))
            {
                LOG_FUNC_ERROR("IoCallDriver", status);
                continue;
            }

            if (!SUCCEEDED(pIrp->IoStatus.Status))
            {
                LOG_FUNC_ERROR("IoCallDriver", pIrp->IoStatus.Status);
                continue;
            }

            LOG_TRACE_FILESYSTEM("Succesfully called device with IOCTL IOCTL_VOLUME_PARTITION_INFO\n");

            if (PARTITION_TYPE_LINUX_SWAP != partitionInformation.PartitionType)
            {
                LOG_WARNING("Partition type 0x%x not supported\n", partitionInformation.PartitionType);
                continue;
            }

            LOG_TRACE_FILESYSTEM("Found supported partition type: 0x%x\n", partitionInformation.PartitionType);

            pFileSystemDevice = IoCreateDevice(DriverObject, sizeof(SWAPFS_DATA), DeviceTypeFilesystem);
            if (NULL == pFileSystemDevice)
            {
                LOG_FUNC_ERROR_ALLOC("IoCreateDevice", sizeof(SWAPFS_DATA));
                status = STATUS_DEVICE_COULD_NOT_BE_CREATED;
                __leave;
            }

            pFileSystemDevice->DeviceAlignment = PAGE_SIZE;

            pSwapFsData = IoGetDeviceExtension(pFileSystemDevice);
            ASSERT(NULL != pSwapFsData);

            pSwapFsData->VolumeDevice = pCurVolume;
            pSwapFsData->FileSystemSize = partitionInformation.PartitionSize * SECTOR_SIZE;

            LOG_TRACE_FILESYSTEM("Mounted SWAP FS on partition of size 0x%X\n", pSwapFsData->FileSystemSize);

            // attach to volume
            IoAttachDevice(pFileSystemDevice, pCurVolume);

            pCurVolume->Vpb->FilesystemDevice = pFileSystemDevice;
            pCurVolume->Vpb->Flags.Mounted = TRUE;
            pCurVolume->Vpb->Flags.SwapSpace = TRUE;

            pFileSystemDevice->Vpb = pCurVolume->Vpb;

            pFileSystemDevice = NULL;
            pSwapFsData = NULL;
        }
    }
    __finally
    {
        if (NULL != pIrp)
        {
            IoFreeIrp(pIrp);
            pIrp = NULL;
        }

        if (NULL != pVolumeDevices)
        {
            IoFreeTemporaryData(pVolumeDevices);
            pVolumeDevices = NULL;
        }
    }

    return status;
}

static
STATUS
(__cdecl _SwapFsCreate)(
    INOUT       PDEVICE_OBJECT      DeviceObject,
    INOUT       PIRP                Irp
    )
{
    STATUS status;
    PIO_STACK_LOCATION pStackLocation;
    PSWAPFS_DATA pSwapFsData;

    LOG_FUNC_START;

    ASSERT(NULL != DeviceObject);
    ASSERT(NULL != Irp);

    status = STATUS_SUCCESS;
    pStackLocation = NULL;
    pSwapFsData = NULL;

    pStackLocation = IoGetCurrentIrpStackLocation(Irp);
    ASSERT(IRP_MJ_CREATE == pStackLocation->MajorFunction);

    pSwapFsData = IoGetDeviceExtension(DeviceObject);
    ASSERT(NULL != pSwapFsData);

    ASSERT(NULL == pStackLocation->FileObject->RelatedFileObject);

    __try
    {
        // in the case of swap FS we only have the root file which we'll use for all our read/write operations
        if (strcmp(pStackLocation->FileObject->FileName, "\\") != 0)
        {
            status = STATUS_FILE_NOT_FOUND;
            __leave;
        }

        pStackLocation->FileObject->FileSize = pSwapFsData->FileSystemSize;
    }
    __finally
    {
        Irp->IoStatus.Status = status;
        Irp->IoStatus.Information = 0;

        IoCompleteIrp(Irp);

        LOG_FUNC_END;
    }

    return STATUS_SUCCESS;
}

static
STATUS
(__cdecl _SwapFsClose)(
    INOUT       PDEVICE_OBJECT      DeviceObject,
    INOUT       PIRP                Irp
    )
{
    LOG_FUNC_START;

    ASSERT(NULL != DeviceObject);
    ASSERT(NULL != Irp);

    Irp->IoStatus.Status = STATUS_SUCCESS;
    IoCompleteIrp(Irp);

    LOG_FUNC_END;

    return STATUS_SUCCESS;
}

static
STATUS
(__cdecl _SwapFsRead)(
    INOUT       PDEVICE_OBJECT      DeviceObject,
    INOUT       PIRP                Irp
    )
{
    STATUS status;
    PIO_STACK_LOCATION pStackLocation;
    PSWAPFS_DATA pSwapFsData;
    QWORD bytesRead;

    LOG_FUNC_START;

    ASSERT(NULL != DeviceObject);
    ASSERT(NULL != Irp);

    status = STATUS_SUCCESS;
    pStackLocation = NULL;
    pSwapFsData = NULL;
    bytesRead = 0;

    pStackLocation = IoGetCurrentIrpStackLocation(Irp);
    ASSERT(IRP_MJ_READ == pStackLocation->MajorFunction);

    pSwapFsData = IoGetDeviceExtension(DeviceObject);
    ASSERT(NULL != pSwapFsData);

    __try
    {
        bytesRead = pStackLocation->Parameters.ReadWrite.Length;

        if (bytesRead != PAGE_SIZE)
        {
            LOG_ERROR("We cannot read more than a page at time!Bytes requested: %U\n", bytesRead);
            status = STATUS_DEVICE_NOT_SUPPORTED;
            __leave;
        }

        status = IoReadDeviceEx(pSwapFsData->VolumeDevice,
                                Irp->Buffer,
                                &bytesRead,
                                pStackLocation->Parameters.ReadWrite.Offset,
                                FALSE);
        if (!SUCCEEDED(status))
        {
            LOG_FUNC_ERROR("IoReadDeviceEx", status);
            __leave;
        }
    }
    __finally
    {
        Irp->IoStatus.Status = status;
        Irp->IoStatus.Information = bytesRead;

        IoCompleteIrp(Irp);

        LOG_FUNC_END;
    }

    return STATUS_SUCCESS;
}

static
STATUS
(__cdecl _SwapFsWrite)(
    INOUT       PDEVICE_OBJECT      DeviceObject,
    INOUT       PIRP                Irp
    )
{
    STATUS status;
    PIO_STACK_LOCATION pStackLocation;
    PSWAPFS_DATA pSwapFsData;
    QWORD bytesWritten;

    LOG_FUNC_START;

    ASSERT(NULL != DeviceObject);
    ASSERT(NULL != Irp);

    status = STATUS_SUCCESS;
    pStackLocation = NULL;
    pSwapFsData = NULL;
    bytesWritten = 0;

    pStackLocation = IoGetCurrentIrpStackLocation(Irp);
    ASSERT(IRP_MJ_WRITE == pStackLocation->MajorFunction);

    pSwapFsData = IoGetDeviceExtension(DeviceObject);
    ASSERT(NULL != pSwapFsData);

    __try
    {
        bytesWritten = pStackLocation->Parameters.ReadWrite.Length;

        if (bytesWritten != PAGE_SIZE)
        {
            LOG_ERROR("We cannot write more than a page at time!Bytes requested: %U\n", bytesWritten);
            status = STATUS_DEVICE_NOT_SUPPORTED;
            __leave;
        }

        status = IoWriteDeviceEx(pSwapFsData->VolumeDevice,
                                 Irp->Buffer,
                                 &bytesWritten,
                                 pStackLocation->Parameters.ReadWrite.Offset,
                                 FALSE);
        if (!SUCCEEDED(status))
        {
            LOG_FUNC_ERROR("IoWriteDeviceEx", status);
            __leave;
        }
    }
    __finally
    {
        Irp->IoStatus.Status = status;
        Irp->IoStatus.Information = bytesWritten;

        IoCompleteIrp(Irp);

        LOG_FUNC_END;
    }

    return STATUS_SUCCESS;
}