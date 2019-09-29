#pragma once

#include "lock_common.h"
#include "network_device.h"

typedef struct _NETWORK_DEVICE
{
    PDEVICE_OBJECT              PhysicalDevice;
    LIST_ENTRY                  NextDevice;

    NETWORK_DEVICE_INFO         Info;
} NETWORK_DEVICE, *PNETWORK_DEVICE;

typedef struct _NETWORK_STACK_DATA
{
    BOOLEAN                     NetworkingEnabled;

    /// should be executive synch mechanism
    RW_SPINLOCK                 DeviceLock;

    _Guarded_by_(DeviceLock)
    DWORD                       NumberOfDevices;

    _Guarded_by_(DeviceLock)
    LIST_ENTRY                  NetworkDeviceList;             
} NETWORK_STACK_DATA, *PNETWORK_STACK_DATA;

_No_competing_thread_
void
NetworkDevicePreinit(
    OUT     PNETWORK_DEVICE     Device,
    IN      PDEVICE_OBJECT      DeviceObject,
    IN      DEVICE_ID           DeviceId
    );

_No_competing_thread_
STATUS
NetworkDeviceInitialize(
    INOUT    PNETWORK_DEVICE    Device
    );

extern NETWORK_STACK_DATA m_netStackData;