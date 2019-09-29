#pragma once

STATUS
EthInitializeDevice(
    IN_READS(ETH_NO_OF_BARS_USED)   PPCI_BAR        Bars,
    INOUT                           PETH_DEVICE     Device
    );

_No_competing_thread_
STATUS
EthReceiveFrame(
    IN                              PETH_DEVICE     Device,
    IN_OPT                          WORD            MaximumNumberOfFrames
    );

_No_competing_thread_
STATUS
EthSendFrame(
    IN                              PETH_DEVICE     Device,
    IN                              WORD            DescriptorIndex,
    IN                              WORD            Length
    );

_No_competing_thread_
BOOLEAN
EthHandleInterrupt(
    IN                              PETH_DEVICE     Device
    );

_No_competing_thread_
void
EthChangeDeviceStatus(
    IN                              PETH_DEVICE             Device,
    IN                              PNETWORK_DEVICE_STATUS  DeviceStatus
    );