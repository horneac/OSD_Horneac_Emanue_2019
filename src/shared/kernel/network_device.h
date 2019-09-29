#pragma once

typedef DWORD DEVICE_ID;

typedef struct _NETWORK_DEVICE_STATUS
{
    BOOLEAN                         RxEnabled;
    BOOLEAN                         TxEnabled;
} NETWORK_DEVICE_STATUS, *PNETWORK_DEVICE_STATUS;

typedef struct _NETWORK_DEVICE_INFO
{
    DEVICE_ID               DeviceId;

    MAC_ADDRESS             PhysicalAddress;

    NETWORK_DEVICE_STATUS   DeviceStatus;
    BOOLEAN                 LinkStatus;
} NETWORK_DEVICE_INFO, *PNETWORK_DEVICE_INFO;

typedef struct _NETWORK_FRAME_STATS
{
    QWORD                   NumberOfFrames;
    QWORD                   TotalBytes;
    QWORD                   SmallestPacket;
    QWORD                   LargestPacket;
} NETWORK_FRAME_STATS, *PNETWORK_FRAME_STATS;

typedef struct _NETWORK_DEVICE_STATS
{
    NETWORK_FRAME_STATS     RxStats;
    NETWORK_FRAME_STATS     TxStats;
} NETWORK_DEVICE_STATS, *PNETWORK_DEVICE_STATS;