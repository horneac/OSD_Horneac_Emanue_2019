#include "network_port_base.h"
#include "ex.h"

STATUS
NetworkPortNotifyReceiveBuffer(
    IN                          PMINIPORT_DEVICE        Device,
    IN                          DWORD                   DesciptorIndex,
    IN                          DWORD                   BufferSize
    )
{
    STATUS status;
    PDEVICE_OBJECT pDevObject;
    PNETWORK_PORT_DEVICE pPortDevice;
    INTR_STATE oldState;
    PLIST_ENTRY pListEntry;
    PFRAME_DESCRIPTOR_ENTRY pFrameDescriptor;
    BOOLEAN bListWasEmpty;
    PVOID pReceiveBuffer;

    ASSERT( NULL != Device );
    ASSERT( 0 != BufferSize );

    status = STATUS_SUCCESS;
    pListEntry = NULL;
    pDevObject = NULL;
    pPortDevice = NULL;
    pFrameDescriptor = NULL;
    bListWasEmpty = FALSE;
    pReceiveBuffer = NULL;

    pDevObject = Device->DeviceObject;
    ASSERT( NULL != pDevObject );

    pPortDevice = IoGetDeviceExtension(pDevObject);
    ASSERT( NULL != pPortDevice );

    if (DesciptorIndex >= pPortDevice->RxData.Buffers.NumberOfBuffers)
    {
        return STATUS_INVALID_PARAMETER2;
    }

    if (BufferSize > pPortDevice->RxData.Buffers.BufferSize)
    {
        return STATUS_INVALID_PARAMETER3;
    }

    pFrameDescriptor = NetworkPortAllocateFrameDescriptor(BufferSize);
    if (NULL == pFrameDescriptor)
    {
        LOG_FUNC_ERROR_ALLOC("NetworkPortAllocateFrameDescriptor", BufferSize);
        return STATUS_HEAP_INSUFFICIENT_RESOURCES;
    }

    pFrameDescriptor->Frame.BufferSize = BufferSize;
    pReceiveBuffer = pPortDevice->RxData.Buffers.Buffers[DesciptorIndex];
    ASSERT( NULL != pReceiveBuffer );

    memcpy( pFrameDescriptor->Frame.Buffer, pReceiveBuffer, BufferSize );

    LockAcquire(&pPortDevice->RxData.Buffers.FramesLock, &oldState);

    bListWasEmpty = IsListEmpty(&pPortDevice->RxData.Buffers.FramesList);
    InsertTailList(&pPortDevice->RxData.Buffers.FramesList, &pFrameDescriptor->ListEntry);

    LockRelease(&pPortDevice->RxData.Buffers.FramesLock, oldState);

    if (bListWasEmpty)
    {
        // signal event
        ExEventSignal(&pPortDevice->RxData.Buffers.FramesListNotEmptyEvent);
    }

    _InterlockedIncrement64(&pPortDevice->RxData.Buffers.NumberOfFramesTransferred);

    return status;
}

void
NetworkPortNotifyTxDescriptorAvailable(
    IN                          PMINIPORT_DEVICE        Device
    )
{
    PNETWORK_PORT_DEVICE pPortDevice;
    PDEVICE_OBJECT pDevObject;

    ASSERT(NULL != Device);

    LOG_FUNC_START;

    pDevObject = Device->DeviceObject;
    ASSERT(NULL != pDevObject);

    pPortDevice = IoGetDeviceExtension(pDevObject);
    ASSERT(NULL != pPortDevice);

    ExEventSignal(&pPortDevice->TxData.DescriptorsAvailable);

    LOG_FUNC_END;
}

void
NetworkPortNotifyTxQueueFull(
    IN                          PMINIPORT_DEVICE        Device
    )
{
    PNETWORK_PORT_DEVICE pPortDevice;
    PDEVICE_OBJECT pDevObject;

    ASSERT(NULL != Device);

    LOG_FUNC_START;

    pDevObject = Device->DeviceObject;
    ASSERT(NULL != pDevObject);

    pPortDevice = IoGetDeviceExtension(pDevObject);
    ASSERT(NULL != pPortDevice);

    ExEventClearSignal(&pPortDevice->TxData.DescriptorsAvailable);

    LOG_FUNC_END;
}

void
NetworkPortNotifyLinkStatusChange(
    IN                          PMINIPORT_DEVICE        Device,
    IN                          BOOLEAN                 LinkUp
    )
{
    ASSERT(NULL != Device);

    LOG_FUNC_START;

    _InterlockedExchange8(&Device->LinkUp, LinkUp);

    LOG_FUNC_END;
}