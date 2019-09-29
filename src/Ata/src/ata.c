#include "ata_base.h"
#include "ata.h"
#include "ata_dispatch.h"
#include "ata_operations.h"

STATUS
(__cdecl AtaDriverEntry)(
    INOUT       PDRIVER_OBJECT      Driver
    )
{
    STATUS status;
    PPCI_DEVICE_DESCRIPTION* pPciDevices;
    DWORD i;
    DWORD j;
    DWORD k;
    PDEVICE_OBJECT pAtaDevice;
    BOOLEAN foundDevice;
    DWORD noOfDevices;
    PCI_SPEC pciSpec;

    ASSERT(NULL != Driver);

    LOG_FUNC_START;

    status = STATUS_SUCCESS;
    pPciDevices = NULL;
    pAtaDevice = NULL;
    foundDevice = FALSE;
    noOfDevices = 0;
    i = 0;
    j = 0;
    k = 0;
    memzero(&pciSpec, sizeof(PCI_SPEC));

    pciSpec.MatchClass = TRUE;
    pciSpec.MatchSubclass = TRUE;

    pciSpec.Description.ClassCode = PciDeviceClassMassStorageController;
    pciSpec.Description.Subclass = PciMassStorageIDE;

    status = IoGetPciDevicesMatchingSpecification( pciSpec, &pPciDevices, &noOfDevices);
    if (!SUCCEEDED(status))
    {
        LOG_FUNC_ERROR("IoGetPciDevicesMatchingClassAndSubclass", status);
        return status;
    }
    ASSERT(noOfDevices == 0 || pPciDevices != NULL);
    LOGL("Found %d IDE devices\n", noOfDevices );

    Driver->DispatchFunctions[IRP_MJ_READ] = AtaDispatchReadWrite;
    Driver->DispatchFunctions[IRP_MJ_WRITE] = AtaDispatchReadWrite;
    Driver->DispatchFunctions[IRP_MJ_DEVICE_CONTROL] = AtaDispatchDeviceControl;

    // lets try to initialize the device
    for (i = 0; i < noOfDevices; ++i)
    {
        ASSERT(pPciDevices[i] != NULL);

        for (j = 0; j < 2; ++j)
        {
            for (k = 0; k < 2; ++k)
            {
                if (NULL != pAtaDevice)
                {
                    IoDeleteDevice(pAtaDevice);
                    pAtaDevice = NULL;
                }

                // create device object
                pAtaDevice = IoCreateDevice(Driver, sizeof(ATA_DEVICE), DeviceTypeHarddiskController);
                if (NULL == pAtaDevice)
                {
                    LOG_FUNC_ERROR_ALLOC("IoCreateDevice", sizeof(ATA_DEVICE));
                    return STATUS_DEVICE_COULD_NOT_BE_CREATED;
                }
                pAtaDevice->DeviceAlignment = SECTOR_SIZE;

                // initialize ATA device
                status = AtaInitialize(pPciDevices[i], (BOOLEAN)j, (BOOLEAN)k, pAtaDevice);
                if (!SUCCEEDED(status))
                {
                    LOG_WARNING("AtaInitialize failed with status: 0x%x\n", status);
                    continue;
                }
                LOG("AtaInitialize succeded\n");

                foundDevice = TRUE;
                pAtaDevice = NULL;
            }
        }
    }

    if (NULL != pPciDevices)
    {
        IoFreeTemporaryData(pPciDevices);
        pPciDevices = NULL;
    }

    if (NULL != pAtaDevice)
    {
        // it means something failed
        IoDeleteDevice(pAtaDevice);
        pAtaDevice = NULL;
    }

    if (foundDevice)
    {
        // if we found at least a device we succeeded
        status = STATUS_SUCCESS;
    }

    LOG_FUNC_END;

    return status;
}