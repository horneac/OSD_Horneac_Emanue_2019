#pragma once

#include "lock_common.h"
#include "ex_event.h"

#pragma warning(push)

// warning C4200: nonstandard extension used: zero-sized array in struct/union
#pragma warning(disable: 4200)
typedef struct _NETWORK_PORT_DRIVER_DATA
{
    MINIPORT_FUNCTIONS              MiniportFunctions;
} NETWORK_PORT_DRIVER_DATA, *PNETWORK_PORT_DRIVER_DATA;

typedef struct _PORT_BUFFERS
{
    DWORD                       NumberOfBuffers;
    WORD                        BufferSize;

    PVOID*                      Buffers;

    LOCK                        FramesLock;
    LIST_ENTRY                  FramesList;
    EX_EVENT                    FramesListNotEmptyEvent;

    volatile QWORD              NumberOfFramesTransferred;
} PORT_BUFFERS, *PPORT_BUFFERS;

typedef struct _RX_DATA
{
    PORT_BUFFERS                Buffers;
} RX_DATA, *PRX_DATA;

typedef struct _TX_DATA
{
    PORT_BUFFERS                Buffers;

    EX_EVENT                    DescriptorsAvailable;

    struct _THREAD*             TransmitWorkerThread;
    WORD                        CurrentTxIndex;
} TX_DATA, *PTX_DATA;

typedef struct _NETWORK_PORT_DEVICE
{
    PMINIPORT_DEVICE            Miniport;

    RX_DATA                     RxData;
    TX_DATA                     TxData;
} NETWORK_PORT_DEVICE, *PNETWORK_PORT_DEVICE;

typedef struct _FRAME_DESCRIPTOR
{
    DWORD               BufferSize;
    BYTE                Buffer[0];
} FRAME_DESCRIPTOR, *PFRAME_DESCRIPTOR;
STATIC_ASSERT_INFO(sizeof(FRAME_DESCRIPTOR) == FIELD_OFFSET(FRAME_DESCRIPTOR, Buffer),
                   "Buffer must always be the last element in the structure");

typedef struct _FRAME_DESCRIPTOR_ENTRY
{
    LIST_ENTRY          ListEntry;
    BYTE                __Padding[4];
    FRAME_DESCRIPTOR    Frame;
} FRAME_DESCRIPTOR_ENTRY, *PFRAME_DESCRIPTOR_ENTRY;
STATIC_ASSERT_INFO(sizeof(FRAME_DESCRIPTOR_ENTRY) - sizeof(FRAME_DESCRIPTOR) == FIELD_OFFSET(FRAME_DESCRIPTOR_ENTRY, Frame),
                   "Frame must always be the last element in the structure");

#pragma warning(pop)

void
NetworkPortDevicePreinit(
    OUT         PNETWORK_PORT_DEVICE    PortDevice
    );

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
    );

void
NetworkPortDeviceUninit(
    INOUT       PNETWORK_PORT_DEVICE    PortDevice
    );

PTR_SUCCESS
PFRAME_DESCRIPTOR_ENTRY
NetworkPortAllocateFrameDescriptor(
    IN          DWORD                   BufferSize
    );

void
NetworkPortFreeFrameDescriptor(
    IN          PFRAME_DESCRIPTOR_ENTRY Descriptor
    );