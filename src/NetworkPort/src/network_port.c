#include "network_port_base.h"
#include "network_port.h"
#include "network_dispatch.h"
#include "ex.h"

static FUNC_InterruptFunction   _NetworkPortGenericInterrupt;

__forceinline
BOOLEAN
_NetworkPortValidateMiniportFunctions(
    IN      PMINIPORT_FUNCTIONS     MiniportFunctions
    )
{
    ASSERT( NULL != MiniportFunctions );

    if ((NULL == MiniportFunctions->MiniportInitializeDevice)   ||
        (NULL == MiniportFunctions->MiniportSendBuffer)         ||
        (NULL == MiniportFunctions->MiniportInterruptHandler)   ||
        (NULL == MiniportFunctions->MiniportChangeDeviceStatus)
        )
    {
        return FALSE;
    }

    return TRUE;
}

__forceinline
BOOLEAN
_NetworkPortValidateBufferDescription(
    IN      PMINIPORT_BUFFER_DESCRIPTION    BufferDescription
    )
{
    ASSERT( NULL != BufferDescription );

    if ((0 == BufferDescription->NumberOfBuffers) ||
        (0 == BufferDescription->DescriptorSize) ||
        (0 == BufferDescription->BufferSize)
        )
    {
        return FALSE;
    }

    return TRUE;
}

static
STATUS
_NetworkPortConfigureDevice(
    IN                          PDRIVER_OBJECT          DriverObject,
    IN                          PMINIPORT_REGISTRATION  MiniportRegistration,
    IN                          PPCI_DEVICE_DESCRIPTION PciDevice,
    OUT_WRITES_ALL(MiniportRegistration->RxBuffers.NumberOfBuffers)
                                PHYSICAL_ADDRESS*       RxPhysicalAddresses,
    OUT_WRITES_ALL(MiniportRegistration->TxBuffers.NumberOfBuffers)
                                PHYSICAL_ADDRESS*       TxPhysicalAddresses
    );

static
STATUS
_NetworkPortInitializeMiniportBuffers(
    IN      PMINIPORT_BUFFER_DESCRIPTION                    BufferDescription,
    OUT_WRITES_ALL(BufferDescription->NumberOfBuffers)
            PHYSICAL_ADDRESS*                               PhysicalAddresses,
    OUT_PTR PVOID**                                         BufferArray,
    OUT_PTR PVOID*                                          DescriptorArray
    );

STATUS
NetworkPortRegisterMiniportDriver(
    IN      PDRIVER_OBJECT          DriverObject,
    IN      PMINIPORT_REGISTRATION  MiniportRegistration
    )
{
    STATUS status;
    PPCI_DEVICE_DESCRIPTION* pPciDevices;
    DWORD noOfDevices;
    DWORD i, j;
    PHYSICAL_ADDRESS* pRxPhysicalAddresses;
    PHYSICAL_ADDRESS* pTxPhysicalAddresses;
    DWORD noOfDevicesInitialized;
    DWORD noOfRxBuffers;
    DWORD noOfTxBuffers;

    if (NULL == DriverObject)
    {
        return STATUS_INVALID_PARAMETER1;
    }

    if (NULL == MiniportRegistration)
    {
        return STATUS_INVALID_PARAMETER2;
    }

    if (!_NetworkPortValidateBufferDescription(&MiniportRegistration->RxBuffers))
    {
        return STATUS_INVALID_BUFFER;
    }

    if (!_NetworkPortValidateBufferDescription(&MiniportRegistration->TxBuffers))
    {
        return STATUS_INVALID_BUFFER;
    }

    if (!_NetworkPortValidateMiniportFunctions(&MiniportRegistration->MiniportFunctions))
    {
        return STATUS_INVALID_FUNCTION;
    }

    LOG_FUNC_START;

    status = STATUS_SUCCESS;
    pPciDevices = NULL;
    noOfDevices = 0;
    pRxPhysicalAddresses = NULL;
    noOfDevicesInitialized = 0;
    noOfRxBuffers = MiniportRegistration->RxBuffers.NumberOfBuffers;
    noOfTxBuffers = MiniportRegistration->TxBuffers.NumberOfBuffers;
    pRxPhysicalAddresses = NULL;
    pTxPhysicalAddresses = NULL;

    DriverObject->DispatchFunctions[IRP_MJ_DEVICE_CONTROL] = NetPortDeviceControl;

    __try
    {
        status = IoGetPciDevicesMatchingSpecification(MiniportRegistration->Specification,
                                                      &pPciDevices,
                                                      &noOfDevices
        );
        if (!SUCCEEDED(status))
        {
            LOG_FUNC_ERROR("IoGetPciDevicesMatchingSpecification", status);
            __leave;
        }

        ASSERT(NULL == DriverObject->DriverExtension);
        DriverObject->DriverExtension = ExAllocatePoolWithTag(PoolAllocateZeroMemory,
                                                              sizeof(NETWORK_PORT_DRIVER_DATA),
                                                              HEAP_PORT_TAG,
                                                              0
        );
        if (NULL == DriverObject->DriverExtension)
        {
            LOG_FUNC_ERROR_ALLOC("ExALlocatePoolWithTag", sizeof(NETWORK_PORT_DRIVER_DATA));
            status = STATUS_HEAP_INSUFFICIENT_RESOURCES;
            __leave;
        }
        memcpy(&((PNETWORK_PORT_DRIVER_DATA)DriverObject->DriverExtension)->MiniportFunctions, &MiniportRegistration->MiniportFunctions, sizeof(MINIPORT_FUNCTIONS));

        pRxPhysicalAddresses = ExAllocatePoolWithTag(PoolAllocateZeroMemory,
                                                     sizeof(PHYSICAL_ADDRESS) * noOfRxBuffers,
                                                     HEAP_TEMP_TAG,
                                                     0
        );
        if (NULL == pRxPhysicalAddresses)
        {
            LOG_FUNC_ERROR_ALLOC("ExAllocatePoolWithTag", sizeof(PHYSICAL_ADDRESS) * noOfRxBuffers);
            status = STATUS_HEAP_INSUFFICIENT_RESOURCES;
            __leave;
        }

        pTxPhysicalAddresses = ExAllocatePoolWithTag(PoolAllocateZeroMemory,
                                                     sizeof(PHYSICAL_ADDRESS) * noOfTxBuffers,
                                                     HEAP_TEMP_TAG,
                                                     0
        );
        if (NULL == pTxPhysicalAddresses)
        {
            LOG_FUNC_ERROR_ALLOC("ExAllocatePoolWithTag", sizeof(PHYSICAL_ADDRESS) * noOfTxBuffers);
            status = STATUS_HEAP_INSUFFICIENT_RESOURCES;
            __leave;
        }

        for (i = 0; i < noOfDevices; ++i)
        {
            // configure current PCI device
            status = _NetworkPortConfigureDevice(DriverObject,
                                                 MiniportRegistration,
                                                 pPciDevices[i],
                                                 pRxPhysicalAddresses,
                                                 pTxPhysicalAddresses
            );
            if (!SUCCEEDED(status))
            {
                LOG_FUNC_ERROR("_NetworkPortConfigureDevice", status);
                continue;
            }

            LOGL("Successfully configured network device found on PCI location (%u.%u.%u)\n",
                 pPciDevices[i]->DeviceLocation.Bus,
                 pPciDevices[i]->DeviceLocation.Device,
                 pPciDevices[i]->DeviceLocation.Function
            );

            // if we're here => we successfully initialized the device
            noOfDevicesInitialized = noOfDevicesInitialized + 1;

            for (j = 0; j < noOfRxBuffers; ++j)
            {
                pRxPhysicalAddresses[j] = NULL;
            }
            for (j = 0; j < noOfTxBuffers; ++j)
            {
                pTxPhysicalAddresses[j] = NULL;
            }
        }
    }
    __finally
    {
        if (NULL != pRxPhysicalAddresses)
        {
            for (j = 0; j < noOfRxBuffers; ++j)
            {
                if (NULL != pRxPhysicalAddresses[j])
                {
                    pRxPhysicalAddresses[j] = NULL;
                }
            }

            ExFreePoolWithTag(pRxPhysicalAddresses, HEAP_TEMP_TAG);
            pRxPhysicalAddresses = NULL;
        }

        if (NULL != pTxPhysicalAddresses)
        {
            for (j = 0; j < noOfTxBuffers; ++j)
            {
                if (NULL != pTxPhysicalAddresses[j])
                {
                    pTxPhysicalAddresses[j] = NULL;
                }
            }

            ExFreePoolWithTag(pTxPhysicalAddresses, HEAP_TEMP_TAG);
            pTxPhysicalAddresses = NULL;
        }

        if (NULL != pPciDevices)
        {
            IoFreeTemporaryData(pPciDevices);
            pPciDevices = NULL;
        }

        // if we initialized at least a device we can say we did our job :)
        status = noOfDevicesInitialized > 1 ? STATUS_SUCCESS : STATUS_DEVICE_DOES_NOT_EXIST;

        if (!SUCCEEDED(status))
        {
            if (NULL != DriverObject->DriverExtension)
            {
                ExFreePoolWithTag(DriverObject->DriverExtension, HEAP_PORT_TAG);
                DriverObject->DriverExtension = NULL;
            }
        }

        LOG_FUNC_END;
    }

    return status;
}

PTR_SUCCESS
PVOID
NetworkPortGetMiniportExtension(
    IN      PMINIPORT_DEVICE        Device
    )
{
    ASSERT( NULL != Device );

    return Device->DeviceExtension;
}

static
STATUS
_NetworkPortConfigureDevice(
    IN                          PDRIVER_OBJECT          DriverObject,
    IN                          PMINIPORT_REGISTRATION  MiniportRegistration,
    IN                          PPCI_DEVICE_DESCRIPTION PciDevice,
    OUT_WRITES_ALL(MiniportRegistration->RxBuffers.NumberOfBuffers)
                                PHYSICAL_ADDRESS*       RxPhysicalAddresses,
    OUT_WRITES_ALL(MiniportRegistration->TxBuffers.NumberOfBuffers)
                                PHYSICAL_ADDRESS*       TxPhysicalAddresses
    )
{
    MINIPORT_DEVICE_INITIALIZATION initialization;
    STATUS status;
    PDEVICE_OBJECT pDevObj;
    PNETWORK_PORT_DEVICE pPortDevice;
    PMINIPORT_DEVICE pMiniportDevice;
    PVOID* pRxBuffers;
    PVOID* pTxBuffers;
    DWORD i;
    IO_INTERRUPT ioInterrupt;

    ASSERT( NULL != DriverObject );
    ASSERT( NULL != MiniportRegistration );
    ASSERT( NULL != PciDevice );

    status = STATUS_SUCCESS;
    memzero(&initialization, sizeof(MINIPORT_DEVICE_INITIALIZATION));
    pDevObj = NULL;
    pPortDevice = NULL;
    pMiniportDevice= NULL;
    pRxBuffers = NULL;
    pTxBuffers = NULL;
    memzero(&ioInterrupt, sizeof(IO_INTERRUPT));

    __try
    {
        pDevObj = IoCreateDevice(DriverObject, sizeof(NETWORK_PORT_DEVICE), DeviceTypePhysicalNetcard);
        if (NULL == pDevObj)
        {
            LOG_FUNC_ERROR_ALLOC("IoCreateDevice", sizeof(NETWORK_PORT_DEVICE));
            status = STATUS_DEVICE_COULD_NOT_BE_CREATED;
            __leave;
        }

        pPortDevice = IoGetDeviceExtension(pDevObj);
        ASSERT(NULL != pPortDevice);

        NetworkPortDevicePreinit(pPortDevice);

        pMiniportDevice = ExAllocatePoolWithTag(PoolAllocateZeroMemory, sizeof(MINIPORT_DEVICE), HEAP_PORT_TAG, 0);
        if (NULL == pMiniportDevice)
        {
            LOG_FUNC_ERROR_ALLOC("ExAllocatePoolWithTag", sizeof(MINIPORT_DEVICE));
            status = STATUS_HEAP_INSUFFICIENT_RESOURCES;
            __leave;
        }
        pMiniportDevice->DeviceObject = pDevObj;

        if (0 != MiniportRegistration->DeviceContextSize)
        {
            pMiniportDevice->DeviceExtension = ExAllocatePoolWithTag(PoolAllocateZeroMemory,
                                                                     MiniportRegistration->DeviceContextSize,
                                                                     HEAP_PORT_TAG,
                                                                     0
            );
            if (NULL == pMiniportDevice->DeviceExtension)
            {
                LOG_FUNC_ERROR_ALLOC("ExAllocatePoolWithTag", MiniportRegistration->DeviceContextSize);
                status = STATUS_HEAP_INSUFFICIENT_RESOURCES;
                __leave;
            }
        }

        status = _NetworkPortInitializeMiniportBuffers(&MiniportRegistration->RxBuffers,
                                                       RxPhysicalAddresses,
                                                       &pRxBuffers,
                                                       &initialization.RxBuffers.RingBuffer
        );
        if (!SUCCEEDED(status))
        {
            LOG_FUNC_ERROR("_NetworkPortInitializeMiniportBuffers", status);
            __leave;
        }

        status = _NetworkPortInitializeMiniportBuffers(&MiniportRegistration->TxBuffers,
                                                       TxPhysicalAddresses,
                                                       &pTxBuffers,
                                                       &initialization.TxBuffers.RingBuffer
        );
        if (!SUCCEEDED(status))
        {
            LOG_FUNC_ERROR("_NetworkPortInitializeMiniportBuffers", status);
            __leave;
        }

        initialization.PciBar = PciDevice->DeviceData->Header.Device.Bar;

        initialization.RxBuffers.NumberOfBuffers = MiniportRegistration->RxBuffers.NumberOfBuffers;
        initialization.RxBuffers.Buffers = RxPhysicalAddresses;
        initialization.RxBuffers.BufferSize = MiniportRegistration->RxBuffers.BufferSize;

        initialization.TxBuffers.NumberOfBuffers = MiniportRegistration->TxBuffers.NumberOfBuffers;
        initialization.TxBuffers.Buffers = TxPhysicalAddresses;
        initialization.TxBuffers.BufferSize = MiniportRegistration->TxBuffers.BufferSize;

        // initialize miniport device
        status = MiniportRegistration->MiniportFunctions.MiniportInitializeDevice(pMiniportDevice,
                                                                                  &initialization
        );
        if (!SUCCEEDED(status))
        {
            LOG_FUNC_ERROR("MiniportInitializeDevice", status);
            __leave;
        }

        LOG_TRACE_NETWORK("Miniport device successfully initialized\n");

        // initialize port device
        status = NetworkPortDeviceInit(pPortDevice,
                                       pMiniportDevice,
                                       MiniportRegistration->RxBuffers.NumberOfBuffers,
                                       pRxBuffers,
                                       MiniportRegistration->RxBuffers.BufferSize,
                                       MiniportRegistration->TxBuffers.NumberOfBuffers,
                                       pTxBuffers,
                                       MiniportRegistration->TxBuffers.BufferSize
        );
        if (!SUCCEEDED(status))
        {
            LOG_FUNC_ERROR("NetworkPortDeviceInit", status);
            __leave;
        }

        // register interrupt
        ioInterrupt.Type = IoInterruptTypePci;
        ioInterrupt.Irql = IrqlNetworkLevel;
        ioInterrupt.ServiceRoutine = _NetworkPortGenericInterrupt;
        ioInterrupt.Exclusive = FALSE;
        ioInterrupt.Pci.PciDevice = PciDevice;

        status = IoRegisterInterrupt(&ioInterrupt, pDevObj);
        ASSERT(SUCCEEDED(status));

        LOG_TRACE_NETWORK("Successfully registered interrupt for network device\n");
    }
    __finally
    {
        if (!SUCCEEDED(status))
        {
            if (NULL != pPortDevice)
            {
                NetworkPortDeviceUninit(pPortDevice);
                pPortDevice = NULL;
            }

            if (NULL != pDevObj)
            {
                IoDeleteDevice(pDevObj);
                pDevObj = NULL;
            }

            if (NULL != pMiniportDevice)
            {
                if (NULL != pMiniportDevice->DeviceExtension)
                {
                    ExFreePoolWithTag(pMiniportDevice->DeviceExtension, HEAP_PORT_TAG);
                    pMiniportDevice->DeviceExtension = NULL;
                }

                ExFreePoolWithTag(pMiniportDevice, HEAP_PORT_TAG);
                pMiniportDevice = NULL;
            }

            if (NULL != pRxBuffers)
            {
                for (i = 0; i < MiniportRegistration->RxBuffers.NumberOfBuffers; ++i)
                {
                    if (NULL != pRxBuffers[i])
                    {
                        IoFreeContinuousMemory(pRxBuffers[i]);
                        pRxBuffers[i] = NULL;
                    }
                }

                ExFreePoolWithTag(pRxBuffers, HEAP_PORT_TAG);
                pRxBuffers = NULL;
            }

            if (NULL != pTxBuffers)
            {
                for (i = 0; i < MiniportRegistration->TxBuffers.NumberOfBuffers; ++i)
                {
                    if (NULL != pTxBuffers[i])
                    {
                        IoFreeContinuousMemory(pTxBuffers[i]);
                        pTxBuffers[i] = NULL;
                    }
                }

                ExFreePoolWithTag(pTxBuffers, HEAP_PORT_TAG);
                pTxBuffers = NULL;
            }
        }
    }

    return status;
}

static
STATUS
_NetworkPortInitializeMiniportBuffers(
    IN      PMINIPORT_BUFFER_DESCRIPTION                    BufferDescription,
    OUT_WRITES_ALL(BufferDescription->NumberOfBuffers)
            PHYSICAL_ADDRESS*                               PhysicalAddresses,
    OUT_PTR PVOID**                                         BufferArray,
    OUT_PTR PVOID*                                          DescriptorArray
    )
{
    STATUS status;
    PVOID* bufferArray;
    PVOID descriptorArray;
    DWORD i;

    ASSERT( NULL != BufferDescription );
    ASSERT( NULL != BufferArray );
    ASSERT( NULL != DescriptorArray );

    status = STATUS_SUCCESS;
    bufferArray = NULL;
    descriptorArray = NULL;
    i = 0;

    __try
    {
        bufferArray = ExAllocatePoolWithTag(PoolAllocateZeroMemory, sizeof(PVOID) * BufferDescription->NumberOfBuffers, HEAP_PORT_TAG, 0);
        if (NULL == bufferArray)
        {
            LOG_FUNC_ERROR_ALLOC("ExAllocatePoolWithTag", sizeof(PVOID) * BufferDescription->NumberOfBuffers);
            status = STATUS_HEAP_INSUFFICIENT_RESOURCES;
            __leave;
        }

        for (i = 0; i < BufferDescription->NumberOfBuffers; ++i)
        {
            bufferArray[i] = IoAllocateContinuousMemory(BufferDescription->BufferSize);
            if (NULL == bufferArray[i])
            {
                LOG_FUNC_ERROR_ALLOC("IoAllocateContinuousMemory", BufferDescription->BufferSize);
                status = STATUS_HEAP_INSUFFICIENT_RESOURCES;
                __leave;
            }

            PhysicalAddresses[i] = IoGetPhysicalAddress(bufferArray[i]);
            ASSERT(NULL != PhysicalAddresses[i]);
        }

        descriptorArray = IoAllocateContinuousMemory(BufferDescription->NumberOfBuffers * BufferDescription->DescriptorSize);
        if (NULL == descriptorArray)
        {
            LOG_FUNC_ERROR_ALLOC("IoAllocateContinuousMemory", BufferDescription->NumberOfBuffers * BufferDescription->DescriptorSize);
            status = STATUS_HEAP_INSUFFICIENT_RESOURCES;
            __leave;
        }
    }
    __finally
    {
        if (!SUCCEEDED(status))
        {
            if (NULL != descriptorArray)
            {
                IoFreeContinuousMemory(descriptorArray);
                descriptorArray = NULL;
            }

            if (NULL != bufferArray)
            {
                for (i = 0; i < BufferDescription->NumberOfBuffers; ++i)
                {
                    if (NULL != bufferArray[i])
                    {
                        IoFreeContinuousMemory(bufferArray[i]);
                        bufferArray[i] = NULL;
                    }
                }

                ExFreePoolWithTag(bufferArray, HEAP_PORT_TAG);
                bufferArray = NULL;
            }
        }
        else
        {
            *BufferArray = bufferArray;
            *DescriptorArray = descriptorArray;
        }
    }

    return status;
}

static
BOOLEAN
(__cdecl _NetworkPortGenericInterrupt)(
    IN      PDEVICE_OBJECT  Device
    )
{
    PNETWORK_PORT_DEVICE pPortDevice;
    PMINIPORT_DEVICE pMiniportDevice;
    PNETWORK_PORT_DRIVER_DATA pDriverExtension;

    ASSERT(NULL != Device);

    pPortDevice = IoGetDeviceExtension(Device);
    ASSERT(NULL != pPortDevice);

    pMiniportDevice = pPortDevice->Miniport;
    ASSERT(NULL != pMiniportDevice);

    pDriverExtension = IoGetDriverExtension( Device );
    ASSERT( NULL != pDriverExtension );

    ASSERT( NULL != pDriverExtension->MiniportFunctions.MiniportInterruptHandler);

    return pDriverExtension->MiniportFunctions.MiniportInterruptHandler( pMiniportDevice );
}