#include "fat32_base.h"
#include "fat32.h"
#include "fat_operations.h"

FUNC_DriverDispatch     _FatDispatchCreate;
FUNC_DriverDispatch     _FatDispatchClose;
FUNC_DriverDispatch     _FatDispatchReadWrite;
FUNC_DriverDispatch     _FatDispatchQueryInformation;
FUNC_DriverDispatch     _FatDispatchDirectoryControl;

typedef struct _FCB
{
    // the offset in the volume, in sectors
    QWORD               FileOffsetInVolume;

    QWORD               ParentOffsetInVolume;

    FILE_INFORMATION    FileInformation;
} FCB, *PFCB;

STATUS
(__cdecl FatDriverEntry)(
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
    PFAT_DATA pFatData;

    LOG_FUNC_START;

    ASSERT(NULL != DriverObject);

    status = STATUS_SUCCESS;
    pVolumeDevices = NULL;
    pCurVolume = NULL;
    numberOfDevices = 0;
    pIrp = NULL;
    pFileSystemDevice = NULL;
    pFatData = NULL;

    DriverObject->DispatchFunctions[IRP_MJ_CREATE] = _FatDispatchCreate;
    DriverObject->DispatchFunctions[IRP_MJ_CLOSE] = _FatDispatchClose;
    DriverObject->DispatchFunctions[IRP_MJ_READ] = _FatDispatchReadWrite;
    DriverObject->DispatchFunctions[IRP_MJ_WRITE] = _FatDispatchReadWrite;
    DriverObject->DispatchFunctions[IRP_MJ_QUERY_INFORMATION] = _FatDispatchQueryInformation;
    DriverObject->DispatchFunctions[IRP_MJ_DIRECTORY_CONTROL] = _FatDispatchDirectoryControl;

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

            if (PARTITION_TYPE_FAT_CHS != partitionInformation.PartitionType &&
                PARTITION_TYPE_FAT_LBA != partitionInformation.PartitionType
                )
            {
                LOG_WARNING("Partition type 0x%x not supported\n", partitionInformation.PartitionType);
                continue;
            }

            LOG_TRACE_FILESYSTEM("Found supported partition type: 0x%x\n", partitionInformation.PartitionType);

            pFileSystemDevice = IoCreateDevice(DriverObject, sizeof(FAT_DATA), DeviceTypeFilesystem);
            if (NULL == pFileSystemDevice)
            {
                LOG_FUNC_ERROR_ALLOC("IoCreateDevice", sizeof(FAT_DATA));
                status = STATUS_DEVICE_COULD_NOT_BE_CREATED;
                __leave;
            }

            pFileSystemDevice->DeviceAlignment = pCurVolume->DeviceAlignment;

            pFatData = IoGetDeviceExtension(pFileSystemDevice);
            ASSERT(NULL != pFatData);

            pFatData->VolumeDevice = pCurVolume;

            status = FatInitVolume(pFatData);
            if (!SUCCEEDED(status))
            {
                LOG_FUNC_ERROR("FatInitVolume", status);
                continue;
            }

            LOG_TRACE_FILESYSTEM("FatInitVolume succeeded\n");

            // attach to volume
            IoAttachDevice(pFileSystemDevice, pCurVolume);

            pCurVolume->Vpb->FilesystemDevice = pFileSystemDevice;
            pCurVolume->Vpb->Flags.Mounted = TRUE;
            pFileSystemDevice->Vpb = pCurVolume->Vpb;

            pFileSystemDevice = NULL;
            pFatData = NULL;
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

STATUS
(__cdecl _FatDispatchCreate)(
    INOUT       PDEVICE_OBJECT      DeviceObject,
    INOUT       PIRP                Irp
    )
{
    STATUS status;
    PIO_STACK_LOCATION pStackLocation;
    PFAT_DATA pFatData;
    QWORD fileSector;
    BYTE fileType;
    FILE_INFORMATION fileInformation;
    BOOLEAN createOperation;
    PFCB pFcb;
    QWORD parentSector;

    LOG_FUNC_START;

    ASSERT(NULL != DeviceObject);
    ASSERT(NULL != Irp);

    status = STATUS_SUCCESS;
    fileSector = 0;
    pStackLocation = NULL;
    pFatData = NULL;
    fileType = 0;
    memzero(&fileInformation, sizeof(FILE_INFORMATION));
    createOperation = FALSE;
    pFcb = NULL;
    parentSector = 0;

    pStackLocation = IoGetCurrentIrpStackLocation(Irp);
    ASSERT(IRP_MJ_CREATE == pStackLocation->MajorFunction);

    pFatData = IoGetDeviceExtension(DeviceObject);
    ASSERT(NULL != pFatData);

    ASSERT(NULL == pStackLocation->FileObject->RelatedFileObject);

    fileType = (TRUE == pStackLocation->FileObject->Flags.DirectoryFile) ? ATTR_DIRECTORY : ATTR_NORMAL;
    createOperation = (BOOLEAN)pStackLocation->FileObject->Flags.Create;

    __try
    {
        if (createOperation)
        {
            // we first create the file
            status = FatCreateDirectoryEntry(pFatData, pStackLocation->FileObject->FileName, fileType);
            if (!SUCCEEDED(status))
            {
                LOG_TRACE_FILESYSTEM("[ERROR]FatCreateDirectoryEntry with status 0x%x\n", status);
                __leave;
            }

            LOG_TRACE_FILESYSTEM("FatCreateDirectoryEntry succeeded\n");
        }

        status = FatSearch(pFatData,
            pStackLocation->FileObject->FileName,
            &fileSector,
            fileType,
            &fileInformation,
            &parentSector
        );
        if (!SUCCEEDED(status))
        {
            LOG_TRACE_FILESYSTEM("[ERROR]FatSearch failed with status 0x%x\n", status);
            __leave;
        }

        // create FCB
        pFcb = ExAllocatePoolWithTag(PoolAllocateZeroMemory, sizeof(FCB), HEAP_FS_TAG, 0);
        if (NULL == pFcb)
        {
            LOG_FUNC_ERROR_ALLOC("HeapAllocatePoolWithTag", sizeof(FCB));
            __leave;
        }

        pFcb->FileOffsetInVolume = fileSector;
        pFcb->ParentOffsetInVolume = parentSector;
        memcpy(&pFcb->FileInformation, &fileInformation, sizeof(FILE_INFORMATION));

        pStackLocation->FileObject->FileSize = fileInformation.FileSize;
        pStackLocation->FileObject->FsContext2 = pFcb;
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

STATUS
(__cdecl _FatDispatchClose)(
    INOUT       PDEVICE_OBJECT      DeviceObject,
    INOUT       PIRP                Irp
    )
{
    STATUS status;
    PIO_STACK_LOCATION pStackLocation;
    PFCB pFcb;

    LOG_FUNC_START;

    ASSERT(NULL != DeviceObject);
    ASSERT(NULL != Irp);

    status = STATUS_SUCCESS;
    pFcb = NULL;

    pStackLocation = IoGetCurrentIrpStackLocation(Irp);

    ASSERT(IRP_MJ_CLOSE == pStackLocation->MajorFunction);

    ASSERT(NULL != pStackLocation->FileObject);
    pFcb = pStackLocation->FileObject->FsContext2;

    ASSERT(NULL != pFcb);

    // as part of the close we need to free the FCB
    ExFreePoolWithTag(pFcb, HEAP_FS_TAG);
    pFcb = NULL;
    pStackLocation->FileObject->FsContext2 = NULL;

    Irp->IoStatus.Status = status;

    IoCompleteIrp(Irp);

    LOG_FUNC_END;

    return STATUS_SUCCESS;
}

STATUS
(__cdecl _FatDispatchReadWrite)(
    INOUT       PDEVICE_OBJECT      DeviceObject,
    INOUT       PIRP                Irp
    )
{
    STATUS status;
    PIO_STACK_LOCATION pStackLocation;
    PFAT_DATA pFatData;
    PFCB pFcb;
    QWORD sectorsReadOrWritten;

    PFUNC_FatReadWriteFile FatReadWriteFunc;

    LOG_FUNC_START;

    ASSERT(NULL != DeviceObject);
    ASSERT(NULL != Irp);

    status = STATUS_SUCCESS;
    pStackLocation = NULL;
    pFatData = NULL;
    pFcb = NULL;
    sectorsReadOrWritten = 0;

    pStackLocation = IoGetCurrentIrpStackLocation(Irp);
    ASSERT(IRP_MJ_READ == pStackLocation->MajorFunction || IRP_MJ_WRITE == pStackLocation->MajorFunction);

    pFatData = IoGetDeviceExtension(DeviceObject);
    ASSERT(NULL != pFatData);

    pFcb = (PFCB)pStackLocation->FileObject->FsContext2;
    ASSERT(NULL != pFcb);

    ASSERT(MAX_DWORD >= pFcb->FileOffsetInVolume);
    ASSERT(MAX_DWORD >= pStackLocation->Parameters.ReadWrite.Offset);
    ASSERT(MAX_DWORD >= pStackLocation->Parameters.ReadWrite.Length);

    LOG_TRACE_FILESYSTEM("pFcb->OffsetInVolume: 0x%X\n", pFcb->FileOffsetInVolume);

    __try
    {
        if (IRP_MJ_READ == pStackLocation->MajorFunction)
        {
            FatReadWriteFunc = FatReadFile;
        }
        else
        {
            ASSERT(pStackLocation->MajorFunction == IRP_MJ_WRITE);
            FatReadWriteFunc = FatWriteFile;
        }

        status = FatReadWriteFunc(
            pFatData,
            (DWORD)pFcb->FileOffsetInVolume,
            (DWORD)(pStackLocation->Parameters.ReadWrite.Offset / pFatData->BytesPerSector),
            pFcb->ParentOffsetInVolume,
            Irp->Buffer,
            (DWORD)(pStackLocation->Parameters.ReadWrite.Length / pFatData->BytesPerSector),
            &sectorsReadOrWritten,
            (BOOLEAN)Irp->Flags.Asynchronous
        );
        if (!SUCCEEDED(status))
        {
            LOG_FUNC_ERROR("FatReadWriteFunc", status);
            __leave;
        }
    }
    __finally
    {
        Irp->IoStatus.Status = status;
        Irp->IoStatus.Information = sectorsReadOrWritten * pFatData->BytesPerSector;

        IoCompleteIrp(Irp);

        LOG_FUNC_END;
    }

    return STATUS_SUCCESS;
}

STATUS
(__cdecl _FatDispatchQueryInformation)(
    INOUT       PDEVICE_OBJECT      DeviceObject,
    INOUT       PIRP                Irp
    )
{
    STATUS status;
    PIO_STACK_LOCATION pStackLocation;
    PFCB pFcb;
    DWORD information;

    LOG_FUNC_START;

    ASSERT(NULL != DeviceObject);
    ASSERT(NULL != Irp);

    status = STATUS_SUCCESS;
    pStackLocation = NULL;
    pFcb = NULL;
    information = sizeof(FILE_INFORMATION);

    __try
    {
        pStackLocation = IoGetCurrentIrpStackLocation(Irp);
        ASSERT(IRP_MJ_QUERY_INFORMATION == pStackLocation->MajorFunction);
        ASSERT(IRP_MN_INFORMATION_FILE_INFORMATION == pStackLocation->MinorFunction);

        if (pStackLocation->Parameters.QueryFile.Length < information)
        {
            status = STATUS_BUFFER_TOO_SMALL;
            __leave;
        }

        ASSERT(NULL != pStackLocation->FileObject);

        pFcb = (PFCB)pStackLocation->FileObject->FsContext2;
        ASSERT(NULL != pFcb);

        memcpy(Irp->Buffer, &pFcb->FileInformation, information);
    }
    __finally
    {
        Irp->IoStatus.Status = status;
        Irp->IoStatus.Information = information;

        IoCompleteIrp(Irp);

        LOG_FUNC_END;
    }

    return STATUS_SUCCESS;
}

STATUS
(__cdecl _FatDispatchDirectoryControl)(
    INOUT       PDEVICE_OBJECT      DeviceObject,
    INOUT       PIRP                Irp
    )
{
    STATUS status;
    PIO_STACK_LOCATION pStackLocation;
    PFCB pFcb;
    DWORD information;
    PFAT_DATA pFatData;

    ASSERT(NULL != DeviceObject);
    ASSERT(NULL != Irp);

    status = STATUS_SUCCESS;
    pStackLocation = NULL;
    pFcb = NULL;
    information = 0;
    pFatData = NULL;

    __try
    {
        pFatData = IoGetDeviceExtension(DeviceObject);
        ASSERT(NULL != pFatData);

        pStackLocation = IoGetCurrentIrpStackLocation(Irp);

        ASSERT(IRP_MJ_DIRECTORY_CONTROL == pStackLocation->MajorFunction);
        ASSERT(IRP_MN_QUERY_DIRECTORY == pStackLocation->MinorFunction);

        ASSERT(NULL != pStackLocation->FileObject);

        if (!pStackLocation->FileObject->Flags.DirectoryFile)
        {
            // we need an actual directory to perform a directory query :)
            status = STATUS_FILE_NOT_DIRECTORY;
            __leave;
        }

        pFcb = (PFCB)pStackLocation->FileObject->FsContext2;

        ASSERT(NULL != pFcb);

        // status = FatQueryDirectory
        status = FatQueryDirectory(pFatData,
            pFcb->FileOffsetInVolume,
            pStackLocation->Parameters.QueryDirectory.Length,
            Irp->Buffer,
            &information
        );
        if (!SUCCEEDED(status))
        {
            LOG_FUNC_ERROR("FatQueryDirectory", status);
            __leave;
        }

        if (information > pStackLocation->Parameters.QueryDirectory.Length)
        {
            // FatQueryDirectory doesn't return any error if the buffer is too small
            status = STATUS_BUFFER_TOO_SMALL;
        }
    }
    __finally
    {
        Irp->IoStatus.Status = status;
        Irp->IoStatus.Information = information;

        IoCompleteIrp(Irp);
    }

    return STATUS_SUCCESS;
}