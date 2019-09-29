#include "eth_82574L_base.h"
#include "eth_82574L.h"
#include "eth_82574L_operations.h"
#include "network_port.h"

static FUNC_NetworkMiniportInitializeDevice     _Eth82574LInitializeMiniport;
static FUNC_NetworkMiniportSendBuffer           _Eth82574LSendBuffer;
static FUNC_NetworkMiniportInterruptHandler     _Eth82574LReceiveInterrupt;
static FUNC_NetworkMiniportChangeDeviceStatus   _Eth82574LChangeDeviceStatus;

__forceinline
void
_EthInitializeBuffers(
    IN          PMINIPORT_BUFFER_INITIALIZATION     BufferInit,
    OUT         PETH_BUFFERS                        Buffers,
    IN          BOOLEAN                             TransmitBuffers
    )
{
    DWORD i;

    ASSERT( NULL != BufferInit );
    ASSERT( NULL != Buffers );

    i = 0;

    ASSERT( BufferInit->NumberOfBuffers <= MAX_WORD );
    Buffers->NumberOfDescriptors = (WORD) BufferInit->NumberOfBuffers;

    memzero(BufferInit->RingBuffer,
            Buffers->NumberOfDescriptors * ( TransmitBuffers ? sizeof(TRANSMIT_DESCRIPTOR_SHADOW) : sizeof(RECEIVE_DESCRIPTOR_SHADOW) ) );
    for (i = 0; i < BufferInit->NumberOfBuffers; ++i)
    {
        if (TransmitBuffers)
        {
            PTRANSMIT_DESCRIPTOR pTxBuffer = BufferInit->RingBuffer;

            pTxBuffer[i].BufferAddress = BufferInit->Buffers[i];

            // set DescriptorDone so that we will know that descriptor is available for software
            pTxBuffer[i].DescriptorDone = 1;
        }
        else
        {
            PRECEIVE_DESCRIPTOR pRxBuffer = BufferInit->RingBuffer;

            pRxBuffer[i].BufferAddress = BufferInit->Buffers[i];
        }
    }
    Buffers->CurrentDescriptor = 0;
    Buffers->BufferSize = BufferInit->BufferSize;
}

STATUS
(__cdecl Eth82574LDriverEntry)(
    INOUT       PDRIVER_OBJECT      DriverObject
    )
{
    STATUS status;
    MINIPORT_REGISTRATION registration;

    ASSERT( NULL != DriverObject );

    LOG_FUNC_START;

    status = STATUS_SUCCESS;

    memzero(&registration, sizeof(MINIPORT_REGISTRATION));

    registration.DeviceContextSize = sizeof(ETH_DEVICE);

    registration.RxBuffers.BufferSize = ETH_BUFFER_SIZE;
    registration.RxBuffers.DescriptorSize = ETH_DESCRIPTOR_SIZE;
    registration.RxBuffers.NumberOfBuffers = ETH_NO_OF_RX_DESCS;

    registration.Specification.MatchVendor = TRUE;
    registration.Specification.MatchDevice = TRUE;
    registration.Specification.Description.VendorId = PciVendorIdIntel;
    registration.Specification.Description.DeviceId = INTEL_82574L_DEV_ID;

    registration.TxBuffers.BufferSize = ETH_BUFFER_SIZE;
    registration.TxBuffers.DescriptorSize = ETH_DESCRIPTOR_SIZE;
    registration.TxBuffers.NumberOfBuffers = ETH_NO_OF_TX_DESCS;

    registration.MiniportFunctions.MiniportInitializeDevice = _Eth82574LInitializeMiniport;
    registration.MiniportFunctions.MiniportUninitializeDevice = NULL;
    registration.MiniportFunctions.MiniportSendBuffer = _Eth82574LSendBuffer;
    registration.MiniportFunctions.MiniportInterruptHandler = _Eth82574LReceiveInterrupt;
    registration.MiniportFunctions.MiniportChangeDeviceStatus = _Eth82574LChangeDeviceStatus;

    // if we don't have any devices or we haven't managed to actually initialize
    // any device there is no reason for the driver to remain 'loaded' =>
    // NetworkPortRegisterMiniportDriver will fail
    status = NetworkPortRegisterMiniportDriver(DriverObject,
                                               &registration
                                               );
    if (!SUCCEEDED(status))
    {
        LOG_FUNC_ERROR("NetworkPortRegisterMiniportDriver", status );
        return status;
    }

    LOG_FUNC_END;

    return status;
}

static
STATUS
(__cdecl _Eth82574LInitializeMiniport)(
    INOUT                           PMINIPORT_DEVICE                    MiniportDevice,
    IN                              PMINIPORT_DEVICE_INITIALIZATION     MiniportInitialization
    )
{
    STATUS status;
    PETH_DEVICE pEthDevice;

    ASSERT( NULL != MiniportDevice );
    ASSERT( NULL != MiniportInitialization );
    ASSERT( NULL != MiniportInitialization->PciBar );

    ASSERT( ETH_NO_OF_RX_DESCS == MiniportInitialization->RxBuffers.NumberOfBuffers );
    ASSERT( NULL != MiniportInitialization->RxBuffers.Buffers );
    ASSERT( NULL != MiniportInitialization->RxBuffers.RingBuffer );
    ASSERT( ETH_BUFFER_SIZE == MiniportInitialization->RxBuffers.BufferSize );

    ASSERT(ETH_NO_OF_TX_DESCS == MiniportInitialization->TxBuffers.NumberOfBuffers);
    ASSERT(NULL != MiniportInitialization->TxBuffers.Buffers);
    ASSERT(NULL != MiniportInitialization->TxBuffers.RingBuffer);
    ASSERT( ETH_BUFFER_SIZE == MiniportInitialization->TxBuffers.BufferSize );

    LOG_FUNC_START;

    status = STATUS_SUCCESS;
    pEthDevice = NULL;

    pEthDevice = NetworkPortGetMiniportExtension(MiniportDevice);
    ASSERT( NULL != pEthDevice );

    _EthInitializeBuffers(&MiniportInitialization->RxBuffers, &pEthDevice->RxData.Buffers, FALSE );
    pEthDevice->RxData.ReceiveBuffer = MiniportInitialization->RxBuffers.RingBuffer;

    _EthInitializeBuffers(&MiniportInitialization->TxBuffers, &pEthDevice->TxData.Buffers, TRUE );
    pEthDevice->TxData.TransmitBuffer = MiniportInitialization->TxBuffers.RingBuffer;

    pEthDevice->MiniportDevice = MiniportDevice;

    status = EthInitializeDevice( MiniportInitialization->PciBar, pEthDevice );
    if (!SUCCEEDED(status))
    {
        LOG_FUNC_ERROR("EthInitializeDevice", status );
        return status;
    }

    LOG_FUNC_END;

    return status;
}

static
STATUS
(__cdecl _Eth82574LSendBuffer)(
    IN  PMINIPORT_DEVICE            MiniportDevice,
    IN  WORD                        DescriptorIndex,
    IN  WORD                        Length
    )
{
    PETH_DEVICE pEthDevice;

    ASSERT(NULL != MiniportDevice);

    pEthDevice = NetworkPortGetMiniportExtension(MiniportDevice);
    ASSERT(NULL != pEthDevice);

    return EthSendFrame(pEthDevice, DescriptorIndex, Length);
}

static
BOOLEAN
(__cdecl _Eth82574LReceiveInterrupt)(
    IN  PMINIPORT_DEVICE            MiniportDevice
    )
{
    PETH_DEVICE pEthDevice;

    ASSERT( NULL != MiniportDevice );

    pEthDevice = NetworkPortGetMiniportExtension(MiniportDevice);
    ASSERT( NULL != pEthDevice );

    return EthHandleInterrupt(pEthDevice);
}

static
void
(__cdecl _Eth82574LChangeDeviceStatus)(
    IN  PMINIPORT_DEVICE            MiniportDevice,
    IN  PNETWORK_DEVICE_STATUS      DeviceStatus
    )
{
    PETH_DEVICE pEthDevice;

    ASSERT( NULL != MiniportDevice );
    ASSERT( NULL != DeviceStatus );

    pEthDevice = NetworkPortGetMiniportExtension(MiniportDevice);
    ASSERT(NULL != pEthDevice);

    EthChangeDeviceStatus(pEthDevice, DeviceStatus );
}