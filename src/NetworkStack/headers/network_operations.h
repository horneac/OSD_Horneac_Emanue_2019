#pragma once

STATUS
NetOpGetPhysicalAddress(
    IN          PDEVICE_OBJECT          DeviceObject,
    OUT         PMAC_ADDRESS            Address
    );

PTR_SUCCESS
PNETWORK_DEVICE
NetOpGetDeviceById(
    IN          DEVICE_ID               Device
    );

STATUS
NetOpGetDeviceStatus(
    IN          PDEVICE_OBJECT          DeviceObject,
    OUT         PNETWORK_DEVICE_STATUS  DeviceStatus
    );

STATUS
NetOpSetDeviceStatus(
    IN          PDEVICE_OBJECT          DeviceObject,
    IN          PNETWORK_DEVICE_STATUS  DeviceStatus
    );

STATUS
NetOpGetLinkStatus(
    IN          PDEVICE_OBJECT          DeviceObject,
    OUT         BOOLEAN*                LinkStatus
    );