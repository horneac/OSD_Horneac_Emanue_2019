#pragma once

typedef struct _DISK_OBJECT
{
    QWORD                       NumberOfSectors;
    PDEVICE_OBJECT              DiskDeviceController;

    PDISK_LAYOUT_INFORMATION    DiskLayout;
} DISK_OBJECT, *PDISK_OBJECT;

typedef struct _VOLUME_LIST_ENTRY
{
    LIST_ENTRY              ListEntry;
    PDEVICE_OBJECT          Volume;
} VOLUME_LIST_ENTRY, *PVOLUME_LIST_ENTRY;