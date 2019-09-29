#include "volume_base.h"
#include "volume.h"
#include "volume_dispatch.h"

static
STATUS
_VolInitialize(
    IN          PDEVICE_OBJECT          Disk,
    IN          PPARTITION_INFORMATION  PartitionInformation,
    INOUT       PDRIVER_OBJECT          DriverObject
    );

STATUS
(__cdecl VolDriverEntry)(
    INOUT       PDRIVER_OBJECT      DriverObject
    )
{
    STATUS status;
    PDEVICE_OBJECT* pDiskDevices;
    DWORD noOfDisks;
    DWORD i;
    DWORD j;
    PIRP pIrp;
    DWORD structureSize;
    PDISK_LAYOUT_INFORMATION pDiskLayout;

    ASSERT(NULL != DriverObject);

    status = STATUS_SUCCESS;
    pDiskDevices = NULL;
    noOfDisks = 0;
    pIrp = NULL;
    structureSize = 0;
    pDiskLayout = NULL;

    DriverObject->DispatchFunctions[IRP_MJ_READ] = VolDispatchReadWrite;
    DriverObject->DispatchFunctions[IRP_MJ_WRITE] = VolDispatchReadWrite;
    DriverObject->DispatchFunctions[IRP_MJ_DEVICE_CONTROL] = VolDispatchDeviceControl;

    status = IoGetDevicesByType(DeviceTypeDisk, &pDiskDevices, &noOfDisks);
    if (!SUCCEEDED(status))
    {
        LOG_FUNC_ERROR("IoGetDevicesByType\n", status);
        return status;
    }

    __try
    {
        for (i = 0; i < noOfDisks; ++i)
        {
            PDEVICE_OBJECT pCurDisk = pDiskDevices[i];

            do
            {
                if (NULL != pDiskLayout)
                {
                    LOGL("Calling HeapFreePoolWithTag for pDiskLayout: 0x%X\n", pDiskLayout);

                    ExFreePoolWithTag(pDiskLayout, HEAP_TEMP_TAG);
                    pDiskLayout = NULL;
                }

                if (0 != structureSize)
                {
                    pDiskLayout = ExAllocatePoolWithTag(PoolAllocateZeroMemory, structureSize, HEAP_TEMP_TAG, 0);
                    if (NULL == pDiskLayout)
                    {
                        status = STATUS_HEAP_NO_MORE_MEMORY;
                        LOG_FUNC_ERROR_ALLOC("HeapALllocatePoolWithTag", structureSize);
                        __leave;
                    }

                    LOGL("pDiskLayout has address: 0x%X\n", pDiskLayout);
                }

                if (NULL != pIrp)
                {
                    IoFreeIrp(pIrp);
                    pIrp = NULL;
                }

                pIrp = IoBuildDeviceIoControlRequest(IOCTL_DISK_LAYOUT_INFO,
                                                     pCurDisk,
                                                     NULL,
                                                     0,
                                                     pDiskLayout,
                                                     structureSize
                );
                ASSERT(NULL != pIrp);

                LOG_TRACE_IO("Successfully built IRP for device control\n");

                status = IoCallDriver(pCurDisk, pIrp);
                if (!SUCCEEDED(status))
                {
                    LOG_FUNC_ERROR("IoCallDriver", status);
                    __leave;
                }

                status = pIrp->IoStatus.Status;

                ASSERT(pIrp->IoStatus.Information <= MAX_DWORD);
                structureSize = (DWORD)pIrp->IoStatus.Information;
            } while (STATUS_BUFFER_TOO_SMALL == status);

            if (!SUCCEEDED(status))
            {
                LOG_FUNC_ERROR("IoCallDriver", status);
                __leave;
            }

            ASSERT(NULL != pDiskLayout);

            for (j = 0; j < pDiskLayout->NumberOfPartitions; ++j)
            {
                status = _VolInitialize(pCurDisk,
                                        &pDiskLayout->Partitions[j],
                                        DriverObject
                );
                if (!SUCCEEDED(status))
                {
                    LOG_FUNC_ERROR("VolInitialize", status);
                    __leave;
                }
                LOG("[%d][%d] Volume successfully initialized\n", i, j);
            }
        }

        
    }
    __finally
    {
        if (NULL != pDiskLayout)
        {
            LOGL("Calling HeapFreePoolWithTag for pDiskLayout: 0x%X\n", pDiskLayout);

            ExFreePoolWithTag(pDiskLayout, HEAP_TEMP_TAG);
            pDiskLayout = NULL;
        }

        if (NULL != pDiskDevices)
        {
            IoFreeTemporaryData(pDiskDevices);
            pDiskDevices = NULL;
        }

        if (NULL != pIrp)
        {
            IoFreeIrp(pIrp);
            pIrp = NULL;
        }

        LOG_FUNC_END;
    }

    return status;
}

STATUS
_VolInitialize(
    IN          PDEVICE_OBJECT          Disk,
    IN          PPARTITION_INFORMATION  PartitionInformation,
    INOUT       PDRIVER_OBJECT          DriverObject
    )
{
    STATUS status;
    PVOLUME pVolumeData;
    PDEVICE_OBJECT pVolumeDevice;
    

    if (NULL == Disk)
    {
        return STATUS_INVALID_PARAMETER1;
    }

    status = STATUS_SUCCESS;
    pVolumeData = NULL;
    pVolumeDevice = NULL;

    __try
    {
        pVolumeDevice = IoCreateDevice(DriverObject, sizeof(VOLUME), DeviceTypeVolume);
        if (NULL == pVolumeDevice)
        {
            LOG_FUNC_ERROR_ALLOC("IoCreateDevice", sizeof(VOLUME));
            status = STATUS_DEVICE_COULD_NOT_BE_CREATED;
            __leave;
        }
        pVolumeDevice->DeviceAlignment = Disk->DeviceAlignment;

        // get volume object
        pVolumeData = (PVOLUME)IoGetDeviceExtension(pVolumeDevice);
        ASSERT(NULL != pVolumeData);

        pVolumeData->DiskDevice = Disk;
        memcpy(&pVolumeData->PartitionInformation, PartitionInformation, sizeof(PARTITION_INFORMATION));

        // attach to disk
        IoAttachDevice(pVolumeDevice, Disk);
    }
    __finally
    {
        if (!SUCCEEDED(status))
        {
            if (NULL != pVolumeDevice)
            {
                IoDeleteDevice(pVolumeDevice);
                pVolumeDevice = NULL;
            }
        }
    }

    return status;
}