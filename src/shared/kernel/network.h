#pragma once

#include "network_packets.h"
#include "network_device.h"

STATUS
NetSendFrame(
    IN                      BOOLEAN         SendOnAllInterfaces,
    _When_(SendOnAllInterfaces, _Reserved_)
    _When_(!SendOnAllInterfaces, IN)
                            DEVICE_ID       DeviceId,
    IN_READS_BYTES(Size)    PVOID           Buffer,
    IN                      DWORD           Size,
    IN                      MAC_ADDRESS     DestinationAddress
    );

STATUS
NetReceiveFrame(
    IN                      DEVICE_ID       DeviceId,
    OUT_WRITES_BYTES(Size)  PVOID           Buffer,
    IN                      DWORD           Size,
    OUT                     DWORD*          BytesWritten
    );

STATUS
NetGetNetworkDevices(
    OUT_WRITES_OPT(*NumberOfDevices)
                    PNETWORK_DEVICE_INFO            DeviceObjects,
    _When_(NULL == DeviceObjects, OUT)
    _When_(NULL != DeviceObjects, INOUT)
                    DWORD*                          NumberOfDevices
    );

STATUS
NetChangeDeviceStatus(
    IN              DEVICE_ID                       DeviceId,
    IN              PNETWORK_DEVICE_STATUS          DeviceStatus
    );

STATUS
NetGetNetworkDeviceStatistics(
    IN              DEVICE_ID                       DeviceId,
    OUT             PNETWORK_DEVICE_STATS           Statistics
    );