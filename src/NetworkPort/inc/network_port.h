#pragma once

typedef struct _MINIPORT_DEVICE
{
    // IN - completed by NetworkPortRegisterMiniportDriver
    PDEVICE_OBJECT                  DeviceObject;
    PVOID                           DeviceExtension;

    // OUT - completed by the FUNC_NetworkMiniportInitializeDevice
    // function
    MAC_ADDRESS                     PhysicalAddress;

    NETWORK_DEVICE_STATUS           DeviceStatus;
    volatile BOOLEAN                LinkUp;
} MINIPORT_DEVICE, *PMINIPORT_DEVICE;

typedef struct _MINIPORT_BUFFER_INITIALIZATION
{
    DWORD                           NumberOfBuffers;
    PHYSICAL_ADDRESS*               Buffers;
    PVOID                           RingBuffer;
    WORD                            BufferSize;
} MINIPORT_BUFFER_INITIALIZATION, *PMINIPORT_BUFFER_INITIALIZATION;

typedef struct _MINIPORT_DEVICE_INITIALIZATION
{
    PPCI_BAR                        PciBar;

    MINIPORT_BUFFER_INITIALIZATION  RxBuffers;
    MINIPORT_BUFFER_INITIALIZATION  TxBuffers;
} MINIPORT_DEVICE_INITIALIZATION, *PMINIPORT_DEVICE_INITIALIZATION;

typedef
STATUS
(__cdecl FUNC_NetworkMiniportInitializeDevice)(
    INOUT                           PMINIPORT_DEVICE                    MiniportDevice,
    IN                              PMINIPORT_DEVICE_INITIALIZATION     MiniportInitialization
    );

typedef FUNC_NetworkMiniportInitializeDevice*   PFUNC_NetworkMiniportInitializeDevice;

typedef
STATUS
(__cdecl FUNC_NetworkMiniportUninitializeDevice)(
    INOUT                           PMINIPORT_DEVICE            MiniportDevice
    );

typedef FUNC_NetworkMiniportUninitializeDevice* PFUNC_NetworkMiniportUninitializeDevice;

typedef
STATUS
(__cdecl FUNC_NetworkMiniportSendBuffer)(
    IN  PMINIPORT_DEVICE            MiniportDevice,
    IN  WORD                        DesccriptorIndex,
    IN  WORD                        Length
    );

typedef FUNC_NetworkMiniportSendBuffer*         PFUNC_NetworkMiniportSendBuffer;

typedef
BOOLEAN
(__cdecl FUNC_NetworkMiniportInterruptHandler)(
    IN  PMINIPORT_DEVICE            MiniportDevice
    );

typedef FUNC_NetworkMiniportInterruptHandler*   PFUNC_NetworkMiniportInterruptHandler;

typedef
void
(__cdecl FUNC_NetworkMiniportChangeDeviceStatus)(
    IN  PMINIPORT_DEVICE            MiniportDevice,
    IN  PNETWORK_DEVICE_STATUS      DeviceStatus
    );

typedef FUNC_NetworkMiniportChangeDeviceStatus* PFUNC_NetworkMiniportChangeDeviceStatus;

typedef struct _MINIPORT_FUNCTIONS
{
    PFUNC_NetworkMiniportInitializeDevice       MiniportInitializeDevice;

    PFUNC_NetworkMiniportUninitializeDevice     MiniportUninitializeDevice;

    PFUNC_NetworkMiniportSendBuffer             MiniportSendBuffer;

    PFUNC_NetworkMiniportInterruptHandler       MiniportInterruptHandler;

    PFUNC_NetworkMiniportChangeDeviceStatus     MiniportChangeDeviceStatus;
} MINIPORT_FUNCTIONS, *PMINIPORT_FUNCTIONS;

typedef struct _MINIPORT_BUFFER_DESCRIPTION
{
    DWORD                                       NumberOfBuffers;
    DWORD                                       DescriptorSize;
    WORD                                        BufferSize;
} MINIPORT_BUFFER_DESCRIPTION, *PMINIPORT_BUFFER_DESCRIPTION;

typedef struct _MINIPORT_REGISTRATION
{
    PCI_SPEC                                    Specification;

    DWORD                                       DeviceContextSize;

    MINIPORT_BUFFER_DESCRIPTION                 RxBuffers;
    MINIPORT_BUFFER_DESCRIPTION                 TxBuffers;

    MINIPORT_FUNCTIONS                          MiniportFunctions;
} MINIPORT_REGISTRATION, *PMINIPORT_REGISTRATION;

STATUS
NetworkPortRegisterMiniportDriver(
    IN      PDRIVER_OBJECT          DriverObject,
    IN      PMINIPORT_REGISTRATION  MiniportRegistration
    );

PTR_SUCCESS
PVOID
NetworkPortGetMiniportExtension(
    IN      PMINIPORT_DEVICE        Device
    );

STATUS
NetworkPortNotifyReceiveBuffer(
    IN                          PMINIPORT_DEVICE        Device,
    IN                          DWORD                   DesciptorIndex,
    IN                          DWORD                   BufferSize
    );

void
NetworkPortNotifyTxDescriptorAvailable(
    IN                          PMINIPORT_DEVICE        Device
    );

void
NetworkPortNotifyTxQueueFull(
    IN                          PMINIPORT_DEVICE        Device
    );

void
NetworkPortNotifyLinkStatusChange(
    IN                          PMINIPORT_DEVICE        Device,
    IN                          BOOLEAN                 LinkUp
    );