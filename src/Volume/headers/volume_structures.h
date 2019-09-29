#pragma once

typedef struct _VOLUME {
    PDEVICE_OBJECT          DiskDevice;
    PARTITION_INFORMATION   PartitionInformation;
} VOLUME, *PVOLUME;