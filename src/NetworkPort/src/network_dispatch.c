#include "network_port_base.h"
#include "network_dispatch.h"
#include "ex.h"

static
STATUS
_NetDispatchReceiveFrame(
    INOUT                                   PNETWORK_PORT_DEVICE        Device,
    IN                                      DWORD                       OutputBufferSize,
    OUT_WRITES_BYTES(OutputBufferSize)      PNET_RECEIVE_FRAME_OUTPUT   ReceiveOutput,
    OUT                                     QWORD*                      Information
    );

static
STATUS
_NetDispatchSendFrame(
    INOUT                                   PNETWORK_PORT_DEVICE        Device,
    IN                                      DWORD                       InputBufferSize,
    IN_READS_BYTES(InputBufferSize)         PNET_RECEIVE_FRAME_OUTPUT   SendBuffer
    );

static
STATUS
_NetDispatchChangeDeviceStatus(
    INOUT                                   PNETWORK_PORT_DEVICE        Device,
    IN                                      PNET_GET_SET_DEVICE_STATUS  DeviceStatus
    );

STATUS
NetPortDeviceControl(
    INOUT       PDEVICE_OBJECT          DeviceObject,
    INOUT       PIRP                    Irp
)
{
    STATUS status;
    PNETWORK_PORT_DEVICE pPortDevice;
    PIO_STACK_LOCATION pStackLocation;
    QWORD information;

    ASSERT(NULL != DeviceObject);
    ASSERT(NULL != Irp);

    LOG_FUNC_START;

    status = STATUS_SUCCESS;
    pPortDevice = NULL;
    information = 0;
    pStackLocation = IoGetCurrentIrpStackLocation(Irp);

    pPortDevice = IoGetDeviceExtension(DeviceObject);
    ASSERT(NULL != pPortDevice);

    switch (pStackLocation->Parameters.DeviceControl.IoControlCode)
    {
    case IOCTL_NET_RECEIVE_FRAME:
        information = sizeof(NET_RECEIVE_FRAME_OUTPUT);

        if (pStackLocation->Parameters.DeviceControl.OutputBufferLength < information)
        {
            status = STATUS_BUFFER_TOO_SMALL;
            break;
        }

        status = _NetDispatchReceiveFrame(pPortDevice, pStackLocation->Parameters.DeviceControl.OutputBufferLength, pStackLocation->Parameters.DeviceControl.OutputBuffer, &information );

        break;
    case IOCTL_NET_GET_PHYSICAL_ADDRESS:
        information = sizeof(NET_GET_SET_PHYSICAL_ADDRESS);

        if (pStackLocation->Parameters.DeviceControl.OutputBufferLength < information)
        {
            status = STATUS_BUFFER_TOO_SMALL;
            break;
        }

        memcpy(pStackLocation->Parameters.DeviceControl.OutputBuffer, &pPortDevice->Miniport->PhysicalAddress, sizeof(MAC_ADDRESS));
        break;
    case IOCTL_NET_SEND_FRAME:
        status = _NetDispatchSendFrame(pPortDevice, pStackLocation->Parameters.DeviceControl.InputBufferLength, Irp->Buffer );
        break;
    case IOCTL_NET_GET_DEVICE_STATUS:
        {
            PNET_GET_SET_DEVICE_STATUS pDeviceStatus = (PNET_GET_SET_DEVICE_STATUS) pStackLocation->Parameters.DeviceControl.OutputBuffer;

            information = sizeof(NET_GET_SET_DEVICE_STATUS);

            if (pStackLocation->Parameters.DeviceControl.OutputBufferLength < information)
            {
                status = STATUS_BUFFER_TOO_SMALL;
                break;
            }

            memcpy(&pDeviceStatus->DeviceStatus, &pPortDevice->Miniport->DeviceStatus, sizeof(NETWORK_DEVICE_STATUS));
        }
        break;
    case IOCTL_NET_SET_DEVICE_STATUS:
        information = sizeof(NET_GET_SET_DEVICE_STATUS);

        if (pStackLocation->Parameters.DeviceControl.InputBufferLength < information)
        {
            status = STATUS_BUFFER_TOO_SMALL;
            break;
        }

        status = _NetDispatchChangeDeviceStatus(pPortDevice, Irp->Buffer);
        break;
    case IOCTL_NET_GET_LINK_STATUS:
        {
            PNET_GET_LINK_STATUS pLinkStatus = (PNET_GET_LINK_STATUS) pStackLocation->Parameters.DeviceControl.OutputBuffer;

            information = sizeof(NET_GET_LINK_STATUS);

            if (pStackLocation->Parameters.DeviceControl.OutputBufferLength < information)
            {
                status = STATUS_BUFFER_TOO_SMALL;
                break;
            }

            pLinkStatus->LinkUp = pPortDevice->Miniport->LinkUp;
        }
        break;
    default:
        status = STATUS_UNSUPPORTED;
    }

    Irp->IoStatus.Status = status;
    Irp->IoStatus.Information = information;
    IoCompleteIrp(Irp);

    LOG_FUNC_END;

    return STATUS_SUCCESS;
}

STATUS
(__cdecl NetPortTransmitFunction)(
    IN_OPT      PVOID       Context
    )
{
    PNETWORK_PORT_DEVICE pPortDevice;
    PNETWORK_PORT_DRIVER_DATA pDriverExtension;
    INTR_STATE intrState;
    PLIST_ENTRY pEntry;
    BOOLEAN bListEmpty;
    PFRAME_DESCRIPTOR_ENTRY pDescriptorEntry;
    STATUS status;
    WORD curTxIndex;

    ASSERT( NULL != Context );

    pPortDevice = Context;
    pEntry = NULL;
    bListEmpty = FALSE;
    status = STATUS_SUCCESS;
    pDriverExtension = NULL;

    LOG_FUNC_START;

    pDriverExtension = IoGetDriverExtension(pPortDevice->Miniport->DeviceObject);
    ASSERT(NULL != pDriverExtension);

#pragma warning(suppress:4127)
    while (TRUE)
    {
        pDescriptorEntry = NULL;

        // wait to have actual data to send
        ExEventWaitForSignal(&pPortDevice->TxData.Buffers.FramesListNotEmptyEvent);

        LockAcquire(&pPortDevice->TxData.Buffers.FramesLock, &intrState);
        pEntry = RemoveHeadList(&pPortDevice->TxData.Buffers.FramesList);
        bListEmpty = ( pEntry == &pPortDevice->TxData.Buffers.FramesList );

        if (bListEmpty)
        {
            ExEventClearSignal(&pPortDevice->TxData.Buffers.FramesListNotEmptyEvent);
        }

        LockRelease(&pPortDevice->TxData.Buffers.FramesLock, intrState );

        if (bListEmpty)
        {
            // list is empty :(
            continue;
        }

        pDescriptorEntry = CONTAINING_RECORD(pEntry, FRAME_DESCRIPTOR_ENTRY, ListEntry );
        curTxIndex = pPortDevice->TxData.CurrentTxIndex;

        ExEventWaitForSignal(&pPortDevice->TxData.DescriptorsAvailable);

        ASSERT( pDescriptorEntry->Frame.BufferSize <= MAX_WORD );
        memcpy( pPortDevice->TxData.Buffers.Buffers[curTxIndex], pDescriptorEntry->Frame.Buffer, pDescriptorEntry->Frame.BufferSize );

        status = pDriverExtension->MiniportFunctions.MiniportSendBuffer( pPortDevice->Miniport, curTxIndex, (WORD) pDescriptorEntry->Frame.BufferSize );
        ASSERT(SUCCEEDED(status));

        curTxIndex = ( curTxIndex + 1 ) % pPortDevice->TxData.Buffers.NumberOfBuffers;
        _InterlockedIncrement64(&pPortDevice->TxData.Buffers.NumberOfFramesTransferred);
        pPortDevice->TxData.CurrentTxIndex = curTxIndex;

        NetworkPortFreeFrameDescriptor(pDescriptorEntry);
        pDescriptorEntry = NULL;
    }

    LOG_FUNC_END;

    return status;
}

static
STATUS
_NetDispatchReceiveFrame(
    INOUT                                   PNETWORK_PORT_DEVICE        Device,
    IN                                      DWORD                       OutputBufferSize,
    OUT_WRITES_BYTES(OutputBufferSize)      PNET_RECEIVE_FRAME_OUTPUT   ReceiveOutput,
    OUT                                     QWORD*                      Information
    )
{
    STATUS status;
    INTR_STATE oldState;
    PLIST_ENTRY pListEntry;
    BOOLEAN bListEmpty;
    PFRAME_DESCRIPTOR_ENTRY pFrame;
    DWORD bufferSize;

    ASSERT( NULL != Device );
    ASSERT( OutputBufferSize >= sizeof(NET_RECEIVE_FRAME_OUTPUT) );
    ASSERT( NULL != ReceiveOutput );
    ASSERT( NULL != Information );

    status = STATUS_SUCCESS;
    pListEntry = NULL;
    bListEmpty = FALSE;
    pFrame = NULL;
    bufferSize = 0;

// warning C4127: conditional expression is constant
#pragma warning(suppress:4127)
    while (TRUE)
    {
        LockAcquire(&Device->RxData.Buffers.FramesLock, &oldState);

        pListEntry = Device->RxData.Buffers.FramesList.Flink;
        bListEmpty = (pListEntry == &Device->RxData.Buffers.FramesList);

        if (!bListEmpty)
        {
            pFrame = CONTAINING_RECORD(pListEntry, FRAME_DESCRIPTOR_ENTRY, ListEntry);
            bufferSize = pFrame->Frame.BufferSize;

            if (bufferSize > OutputBufferSize)
            {
                LOGL("Buffer received of size %u is too small. Required: %u\n", OutputBufferSize, bufferSize );
                status = STATUS_BUFFER_TOO_SMALL;
            }
            else
            {
                // actually remove element from list
                RemoveEntryList(pListEntry);
            }
        }
        else
        {
            // list empty clear event
            ExEventClearSignal(&Device->RxData.Buffers.FramesListNotEmptyEvent);
        }

        LockRelease(&Device->RxData.Buffers.FramesLock, oldState);


        if (STATUS_BUFFER_TOO_SMALL == status)
        {
            break;
        }

        ASSERT(SUCCEEDED(status));

        if (bListEmpty)
        {
            // if list is empty we need to check if the link is down or if the device RX
            // functionality is disabled
            if (!Device->Miniport->LinkUp)
            {
                LOG_WARNING("Device is not linked\n");
                status = STATUS_DEVICE_NOT_CONNECTED;
                break;
            }

            if (!Device->Miniport->DeviceStatus.RxEnabled)
            {
                LOG_WARNING("Can no longer receive packets because device RX is disabled! :(\n");
                status = STATUS_DEVICE_DISABLED;
                break;
            }

            ExEventWaitForSignal(&Device->RxData.Buffers.FramesListNotEmptyEvent);
        }
        else
        {
            break;
        }
    }

    // set output buffer size written/required
    *Information = bufferSize;

    if (SUCCEEDED(status))
    {
        ASSERT(NULL != pFrame);
        ASSERT(!bListEmpty);

        memcpy( &ReceiveOutput->Buffer,pFrame->Frame.Buffer, pFrame->Frame.BufferSize);

        NetworkPortFreeFrameDescriptor(pFrame);
        pFrame = NULL;
    }


    return status;
}

static
STATUS
_NetDispatchSendFrame(
    INOUT                                   PNETWORK_PORT_DEVICE        Device,
    IN                                      DWORD                       InputBufferSize,
    IN_READS_BYTES(InputBufferSize)         PNET_RECEIVE_FRAME_OUTPUT   SendBuffer
    )
{
    STATUS status;
    PFRAME_DESCRIPTOR_ENTRY pFrameDescriptor;
    INTR_STATE intrState;
    BOOLEAN bListWasEmpty;

    ASSERT(NULL != Device);
    ASSERT(0 != InputBufferSize);
    ASSERT(NULL != SendBuffer);

    if (!Device->Miniport->LinkUp)
    {
        LOG_WARNING("Device is not linked\n");
        return STATUS_DEVICE_NOT_CONNECTED;
    }

    if (!Device->Miniport->DeviceStatus.TxEnabled)
    {
        LOG_WARNING("Device TX is disabled, cannot send packets\n");
        return STATUS_DEVICE_DISABLED;
    }

    if (InputBufferSize > Device->TxData.Buffers.BufferSize)
    {
        LOG_ERROR("Transmit buffer size %u bytes too large for device buffer size of %u bytes\n",
             InputBufferSize, Device->TxData.Buffers.BufferSize );
        return STATUS_BUFFER_TOO_LARGE;
    }

    status = STATUS_SUCCESS;
    pFrameDescriptor = NULL;
    bListWasEmpty = FALSE;

    pFrameDescriptor = NetworkPortAllocateFrameDescriptor(InputBufferSize);
    if (NULL == pFrameDescriptor)
    {
        LOG_FUNC_ERROR_ALLOC("NetworkPortAllocateFrameDescriptor", InputBufferSize);
        return STATUS_HEAP_INSUFFICIENT_RESOURCES;
    }

    pFrameDescriptor->Frame.BufferSize = InputBufferSize;
    memcpy( pFrameDescriptor->Frame.Buffer, SendBuffer, InputBufferSize);

    LockAcquire(&Device->TxData.Buffers.FramesLock, &intrState);
    bListWasEmpty = IsListEmpty(&Device->TxData.Buffers.FramesList);
    InsertTailList(&Device->TxData.Buffers.FramesList, &pFrameDescriptor->ListEntry);
    LockRelease(&Device->TxData.Buffers.FramesLock, intrState);
    pFrameDescriptor = NULL;

    if (bListWasEmpty)
    {
        ExEventSignal(&Device->TxData.Buffers.FramesListNotEmptyEvent);
    }

    return status;
}

static
STATUS
_NetDispatchChangeDeviceStatus(
    INOUT                                   PNETWORK_PORT_DEVICE        Device,
    IN                                      PNET_GET_SET_DEVICE_STATUS  DeviceStatus
    )
{
    STATUS status;
    PNETWORK_PORT_DRIVER_DATA pDriverExtension;

    ASSERT(NULL != Device);
    ASSERT(NULL != DeviceStatus);

    status = STATUS_SUCCESS;
    pDriverExtension = NULL;

    LOG_FUNC_START;

    pDriverExtension = IoGetDriverExtension(Device->Miniport->DeviceObject);
    ASSERT(NULL != pDriverExtension);

    pDriverExtension->MiniportFunctions.MiniportChangeDeviceStatus(Device->Miniport,
                                                                   &DeviceStatus->DeviceStatus
                                                                   );
    LOGL("Miniport device successfully changed status\n");

    memcpy(&Device->Miniport->DeviceStatus, &DeviceStatus->DeviceStatus, sizeof(NETWORK_DEVICE_STATUS));

    LOG_FUNC_END;

    return status;
}