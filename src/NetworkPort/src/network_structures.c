#include "network_port_base.h"
#include "network_dispatch.h"
#include "ex.h"

__forceinline
void
_NetworkPortPreinitBuffers(
    OUT         PPORT_BUFFERS           Buffers
    )
{
    memzero(Buffers, sizeof(PORT_BUFFERS));

    LockInit(&Buffers->FramesLock);
    InitializeListHead(&Buffers->FramesList);
}

__forceinline
void
_NetworkPortDeviceInitBuffers(
    OUT         PPORT_BUFFERS           PortBuffers,
    IN          DWORD                   NumberOfBuffers,
    IN          PVOID*                  Buffers,
    IN          WORD                    BufferSize
    )
{
    ASSERT( NULL != PortBuffers );
    ASSERT( 0 != NumberOfBuffers );
    ASSERT( NULL != Buffers );
    ASSERT( 0 != BufferSize );

    PortBuffers->NumberOfBuffers = NumberOfBuffers;
    PortBuffers->Buffers = (PVOID*)Buffers;
    PortBuffers->BufferSize = BufferSize;
}

static
STATUS
_NetworkPortDeviceInitRx(
    INOUT       PRX_DATA                RxData,
    IN          DWORD                   NumberOfReceiveBuffers,
    IN          PVOID*                  ReceiveBuffers,
    IN          WORD                    ReceiveBufferSize
    );

static
STATUS
_NetworkPortDeviceInitTx(
    INOUT       PTX_DATA                TxData,
    IN          PNETWORK_PORT_DEVICE    PortDevice,
    IN          DWORD                   NumberOfTransmitBuffers,
    IN          PVOID*                  TransmitBuffers,
    IN          WORD                    TransmitBufferSize
    );

void
NetworkPortDevicePreinit(
    OUT         PNETWORK_PORT_DEVICE    PortDevice
    )
{
    ASSERT( NULL != PortDevice );

    memzero(PortDevice, sizeof(NETWORK_PORT_DEVICE));

    _NetworkPortPreinitBuffers(&PortDevice->RxData.Buffers);
    _NetworkPortPreinitBuffers(&PortDevice->TxData.Buffers);
}

STATUS
NetworkPortDeviceInit(
    INOUT       PNETWORK_PORT_DEVICE    PortDevice,
    IN          PMINIPORT_DEVICE        MiniportDevice,
    IN          DWORD                   NumberOfReceiveBuffers,
    IN          PVOID*                  ReceiveBuffers,
    IN          WORD                    ReceiveBufferSize,
    IN          DWORD                   NumberOfTransmitBuffers,
    IN          PVOID*                  TransmitBuffers,
    IN          WORD                    TransmitBufferSize
    )
{
    STATUS status;

    if (NULL == PortDevice)
    {
        return STATUS_INVALID_PARAMETER1;
    }

    if (NULL == MiniportDevice)
    {
        return STATUS_INVALID_PARAMETER2;
    }

    if (0 == NumberOfReceiveBuffers)
    {
        return STATUS_INVALID_PARAMETER3;
    }

    if (NULL == ReceiveBuffers)
    {
        return STATUS_INVALID_PARAMETER4;
    }

    if (0 == ReceiveBufferSize)
    {
        return STATUS_INVALID_PARAMETER5;
    }

    if (0 == NumberOfTransmitBuffers)
    {
        return STATUS_INVALID_PARAMETER6;
    }

    if (NULL == TransmitBuffers)
    {
        return STATUS_INVALID_PARAMETER7;
    }

    if (0 == TransmitBufferSize)
    {
        return STATUS_INVALID_PARAMETER8;
    }

    status = STATUS_SUCCESS;

    PortDevice->Miniport = MiniportDevice;

    status = _NetworkPortDeviceInitRx(&PortDevice->RxData,
                                      NumberOfReceiveBuffers,
                                      ReceiveBuffers,
                                      ReceiveBufferSize
                                      );
    if (!SUCCEEDED(status))
    {
        LOG_FUNC_ERROR("_NetworkPortDeviceInitRx", status );
        return status;
    }

    status = _NetworkPortDeviceInitTx(&PortDevice->TxData,
                                      PortDevice,
                                      NumberOfTransmitBuffers,
                                      TransmitBuffers,
                                      TransmitBufferSize
                                      );
    if (!SUCCEEDED(status))
    {
        LOG_FUNC_ERROR("_NetworkPortDeviceInitTx", status);
        return status;
    }


    return status;
}

void
NetworkPortDeviceUninit(
    INOUT       PNETWORK_PORT_DEVICE    PortDevice
    )
{
    PMINIPORT_DEVICE pMiniportDevice;

    ASSERT( NULL != PortDevice );

    pMiniportDevice = PortDevice->Miniport;

    if (NULL != pMiniportDevice)
    {
        if (NULL != pMiniportDevice->DeviceObject)
        {
            PNETWORK_PORT_DRIVER_DATA pDriverData = IoGetDriverExtension(pMiniportDevice->DeviceObject);
            if (NULL != pDriverData)
            {
                if (NULL != pDriverData->MiniportFunctions.MiniportUninitializeDevice)
                {
                    pDriverData->MiniportFunctions.MiniportUninitializeDevice(pMiniportDevice);
                }
            }
        }

        if (NULL != pMiniportDevice->DeviceExtension)
        {
            ExFreePoolWithTag(pMiniportDevice->DeviceExtension, HEAP_PORT_TAG);
            pMiniportDevice->DeviceExtension = NULL;
        }

        ExFreePoolWithTag(pMiniportDevice, HEAP_PORT_TAG);
        pMiniportDevice = NULL;
    }

    if (NULL != PortDevice->RxData.Buffers.Buffers)
    {
        ExFreePoolWithTag(PortDevice->RxData.Buffers.Buffers, HEAP_PORT_TAG);
        PortDevice->RxData.Buffers.Buffers = NULL;
    }

    if (NULL != PortDevice->TxData.Buffers.Buffers)
    {
        ExFreePoolWithTag(PortDevice->TxData.Buffers.Buffers, HEAP_PORT_TAG);
        PortDevice->TxData.Buffers.Buffers = NULL;
    }

    memzero(PortDevice, sizeof(NETWORK_PORT_DEVICE));
}

PTR_SUCCESS
PFRAME_DESCRIPTOR_ENTRY
NetworkPortAllocateFrameDescriptor(
    IN          DWORD                   BufferSize
    )
{
    ASSERT(0 != BufferSize);

    return ExAllocatePoolWithTag(0, sizeof(FRAME_DESCRIPTOR_ENTRY) + BufferSize, HEAP_PORT_TAG, 0);
}

void
NetworkPortFreeFrameDescriptor(
    IN          PFRAME_DESCRIPTOR_ENTRY Descriptor
    )
{
    ASSERT( NULL != Descriptor );

    ExFreePoolWithTag(Descriptor, HEAP_PORT_TAG);
}

static
STATUS
_NetworkPortDeviceInitRx(
    INOUT       PRX_DATA                RxData,
    IN          DWORD                   NumberOfReceiveBuffers,
    IN          PVOID*                  ReceiveBuffers,
    IN          WORD                    ReceiveBufferSize
    )
{
    STATUS status;

    ASSERT( NULL != RxData );

    status = STATUS_SUCCESS;

    _NetworkPortDeviceInitBuffers(&RxData->Buffers,
                                  NumberOfReceiveBuffers,
                                  ReceiveBuffers,
                                  ReceiveBufferSize
                                  );

    status = ExEventInit(&RxData->Buffers.FramesListNotEmptyEvent, ExEventTypeNotification, FALSE);
    if (!SUCCEEDED(status))
    {
        LOG_FUNC_ERROR("ExEventInit", status);
        return status;
    }

    return status;
}

static
STATUS
_NetworkPortDeviceInitTx(
    INOUT       PTX_DATA                TxData,
    IN          PNETWORK_PORT_DEVICE    PortDevice,
    IN          DWORD                   NumberOfTransmitBuffers,
    IN          PVOID*                  TransmitBuffers,
    IN          WORD                    TransmitBufferSize
    )
{
    STATUS status;

    ASSERT(NULL != TxData);

    status = STATUS_SUCCESS;

    _NetworkPortDeviceInitBuffers(&TxData->Buffers,
                                  NumberOfTransmitBuffers,
                                  TransmitBuffers,
                                  TransmitBufferSize
                                  );

    status = ExEventInit(&TxData->Buffers.FramesListNotEmptyEvent, ExEventTypeNotification, FALSE);
    if (!SUCCEEDED(status))
    {
        LOG_FUNC_ERROR("ExEventInit", status);
        return status;
    }

    status = ExEventInit(&TxData->DescriptorsAvailable, ExEventTypeNotification, TRUE);
    if (!SUCCEEDED(status))
    {
        LOG_FUNC_ERROR("ExEventInit", status);
        return status;
    }

    status = ThreadCreate("TX worker thread",
                          ThreadPriorityDefault,
                          NetPortTransmitFunction,
                          PortDevice,
                          &PortDevice->TxData.TransmitWorkerThread
                          );
    if (!SUCCEEDED(status))
    {
        LOG_FUNC_ERROR("ThreadCreate", status);
        return status;
    }

    return status;
}