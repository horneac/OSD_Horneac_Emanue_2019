#pragma once

#include "time.h"

/// never do this at home kids
#include "..\..\HAL9000\headers\mutex.h"

#define SHORT_NAME_MAX_LENGTH       16

// common packing
#pragma pack(push,8)

#pragma warning(push)

// warning C4201: nonstandard extension used: nameless struct/union
#pragma warning(disable:4201)

/////////////////////////////////////////////////////////////////////////////////////////////////
/////////                        IRP_MJ_QUERY_INFORMATION                               /////////
/////////////////////////////////////////////////////////////////////////////////////////////////


// IRP_MN_INFORMATION_FILE_INFORMATION
#define FILE_ATTRIBUTE_NORMAL       0x01
#define FILE_ATTRIBUTE_DIRECTORY    0x02
#define FILE_ATTRIBUTE_VOLUME       0x04

typedef struct _FILE_INFORMATION
{
    DATETIME                        CreationTime;
    DATETIME                        LastWriteTime;

    // file size on disk
    QWORD                           FileSize;
    DWORD                           FileAttributes;
} FILE_INFORMATION, *PFILE_INFORMATION;


/////////////////////////////////////////////////////////////////////////////////////////////////
/////////                        IRP_MJ_DIRECTORY_CONTROL                               /////////
/////////////////////////////////////////////////////////////////////////////////////////////////

// IRP_MN_QUERY_DIRECTORY
// warning C4200: nonstandard extension used: zero-sized array in struct/union
#pragma warning(disable:4200)
typedef
_Struct_size_bytes_(sizeof(FILE_DIRECTORY_INFORMATION) + FilenameLength)
struct _FILE_DIRECTORY_INFORMATION
{
    DWORD                           NextEntryOffset;
    FILE_INFORMATION                BasicFileInformation;
    char                            ShortFilename[SHORT_NAME_MAX_LENGTH];
    DWORD                           FilenameLength;
    char                            Filename[0];
} FILE_DIRECTORY_INFORMATION, *PFILE_DIRECTORY_INFORMATION;

/////////////////////////////////////////////////////////////////////////////////////////////////
/////////                        FUNDAMENTAL STRUCTURES                                 /////////
/////////////////////////////////////////////////////////////////////////////////////////////////

#define IRP_MJ_CREATE                               0
#define IRP_MJ_CLOSE                                1
#define IRP_MJ_READ                                 2
#define IRP_MJ_WRITE                                3
#define IRP_MJ_QUERY_INFORMATION                    4
#define IRP_MJ_DIRECTORY_CONTROL                    5
#define IRP_MJ_DEVICE_CONTROL                       6
#define IRP_MJ_MAX                                  IRP_MJ_DEVICE_CONTROL+1

// used with  IRP_MJ_QUERY_INFORMATION
#define IRP_MN_INFORMATION_FILE_INFORMATION         0

// used with IRP_MJ_DIRECTORY_CONTROL
#define IRP_MN_QUERY_DIRECTORY                      0

typedef enum _DEVICE_TYPE
{
    DeviceTypeMin,
    DeviceTypeSystem = DeviceTypeMin,
    DeviceTypeHarddiskController,
    DeviceTypeDisk,
    DeviceTypeVolume,
    DeviceTypeFilesystem,
    DeviceTypePhysicalNetcard,
    DeviceTypeMax = DeviceTypePhysicalNetcard
} DEVICE_TYPE;

typedef struct _DEVICE_OBJECT
{
    struct _DRIVER_OBJECT*  DriverObject;
    PVOID                   DeviceExtension;
    DWORD                   DeviceExtensionSize;
    DEVICE_TYPE             DeviceType;

    BYTE                    StackSize;

    // The alignment the device requires for
    // the R/W parameters (length and offset)
    DWORD                   DeviceAlignment;

    MUTEX                   DeviceLock;

    // valid only for volume and file system devices
    struct _VPB*            Vpb;

    // Next device belonging to this driver
    LIST_ENTRY              NextDevice;

    // device to which we are attached
    struct _DEVICE_OBJECT*  AttachedDevice;
} DEVICE_OBJECT, *PDEVICE_OBJECT;

typedef
BOOLEAN
(__cdecl FUNC_InterruptFunction)(
    IN      PDEVICE_OBJECT  Device
    );

typedef FUNC_InterruptFunction*        PFUNC_InterruptFunction;

typedef enum _IO_INTERRUPT_TYPE
{
    IoInterruptTypeLegacy,
    IoInterruptTypeLapic,
    IoInterruptTypePci
} IO_INTERRUPT_TYPE;

typedef struct _IO_INTERRUPT
{
    IO_INTERRUPT_TYPE           Type;
    IRQL                        Irql;
    BOOLEAN                     Exclusive;
    PFUNC_InterruptFunction     ServiceRoutine;
    BOOLEAN                     BroadcastInterrupt;
    union
    {
        struct
        {
            BYTE                Irq;
        } Legacy;
        struct
        {
            BYTE                __Reserved0;
        } Lapic;
        struct
        {
            PPCI_DEVICE_DESCRIPTION         PciDevice;
        } Pci;
    };
} IO_INTERRUPT, *PIO_INTERRUPT;

typedef struct _IO_STACK_LOCATION
{
    BYTE            MajorFunction;
    BYTE            MinorFunction;
    union {
        struct {
            QWORD               Length;
            QWORD               Offset;
        } ReadWrite;
        struct {
            DWORD               IoControlCode;

            // the Buffer stored in the IRP structure
            // is used for input
            DWORD               InputBufferLength;
            DWORD               OutputBufferLength;
            PVOID               OutputBuffer;
        } DeviceControl;
        struct {
            DWORD               Length;
        } QueryFile;
        struct {
            DWORD               Length;
            DWORD               FileIndex;
        } QueryDirectory;
    } Parameters;

    PDEVICE_OBJECT  DeviceObject;
    PFILE_OBJECT    FileObject;
} IO_STACK_LOCATION, *PIO_STACK_LOCATION;

typedef struct _IRP_FLAGS
{
    DWORD           Completed       :  1;
    DWORD           Asynchronous    :  1;
    DWORD           Reserved        : 30;
} IRP_FLAGS, *PIRP_FLAGS;

typedef struct _IO_STATUS_BLOCK
{
    QWORD           Information;
    STATUS          Status;
} IO_STATUS_BLOCK, *PIO_STATUS_BLOCK;

// warning C4200: nonstandard extension used: zero-sized array in struct/union
#pragma warning(disable:4200)
typedef struct _IRP
{
    PVOID               Buffer;
    IO_STATUS_BLOCK     IoStatus;
    IRP_FLAGS           Flags;
    BYTE                CurrentStackLocation;

    struct _MDL*        Mdl;

    IO_STACK_LOCATION   StackLocations[0];
} IRP, *PIRP;

typedef struct _MDL_TRANSLATION_PAIR
{
    PHYSICAL_ADDRESS    Address;
    DWORD               NumberOfBytes;
} MDL_TRANSLATION_PAIR, *PMDL_TRANSLATION_PAIR;

// warning C4200: nonstandard extension used: zero-sized array in struct/union
#pragma warning(disable:4200)
typedef
_Struct_size_bytes_(sizeof(MDL) + NumberOfTranslationPairs * sizeof(MDL_TRANSLATION_PAIR))
struct _MDL
{
    PBYTE                       StartVa;

    DWORD                       ByteCount;
    DWORD                       ByteOffset;

    DWORD                       NumberOfTranslationPairs;

    MDL_TRANSLATION_PAIR        Translations[0];
} MDL, *PMDL;

typedef
STATUS
(__cdecl FUNC_DriverDispatch)(
    INOUT       PDEVICE_OBJECT      DeviceObject,
    INOUT       PIRP                Irp
    );

typedef FUNC_DriverDispatch*        PFUNC_DriverDispatch;

typedef struct _DRIVER_OBJECT
{
    PVOID                   DriverExtension;
    char*                   DriverName;

    DWORD                   NoOfDevices;
    LIST_ENTRY              DeviceList;

    LIST_ENTRY              NextDriver;

    PFUNC_DriverDispatch    DispatchFunctions[IRP_MJ_MAX];
} DRIVER_OBJECT, *PDRIVER_OBJECT;

typedef
STATUS
(__cdecl FUNC_DriverEntry)(
    INOUT       PDRIVER_OBJECT      DriverObject
    );

typedef FUNC_DriverEntry*           PFUNC_DriverEntry;

/////////////////////////////////////////////////////////////////////////////////////////////////
/////////                        IRP_MJ_DEVICE_CONTROL                                  /////////
/////////////////////////////////////////////////////////////////////////////////////////////////

#pragma warning(disable:4200)

// IOCTL_DISK_GET_LENGTH_INFO
typedef struct _GET_LENGTH_INFORMATION
{
    QWORD           Length;
} GET_LENGTH_INFORMATION, *PGET_LENGTH_INFORMATION;

#define PARTITION_TYPE_NTFS                         0x7
#define PARTITION_TYPE_FAT_CHS                      0xB
#define PARTITION_TYPE_FAT_LBA                      0xC
#define PARTITION_TYPE_FAT16_LBA                    0xE
#define PARTITION_TYPE_LINUX_SWAP                   0x82
#define PARTITION_TYPE_LINUX_NATIVE                 0x83
#define PARTITION_TYPE_MICROSOFT_PROTECTIVE_MBR     0xEE
#define PARTITION_TYPE_INTEL_PROTECTIVE_MBR         0xEF

// IOCTL_VOLUME_PARTITION_INFO
typedef struct _PARTITION_INFORMATION
{
    QWORD           OffsetInDisk;

    // Partition size in sectors
    QWORD           PartitionSize;
    BYTE            PartitionType;
    BOOLEAN         Bootable;
} PARTITION_INFORMATION, *PPARTITION_INFORMATION;

// IOCTL_DISK_LAYOUT_INFO
typedef
_Struct_size_bytes_(sizeof(DISK_LAYOUT_INFORMATION) + NumberOfPartitions * sizeof(PARTITION_INFORMATION))
struct _DISK_LAYOUT_INFORMATION
{
    DWORD                   NumberOfPartitions;
    PARTITION_INFORMATION   Partitions[0];
} DISK_LAYOUT_INFORMATION, *PDISK_LAYOUT_INFORMATION;


// IOCTL_NET_RECEIVE_FRAME
typedef struct _NET_RECEIVE_FRAME_OUTPUT
{
    ETHERNET_FRAME          Buffer;
} NET_RECEIVE_FRAME_OUTPUT, *PNET_RECEIVE_FRAME_OUTPUT;

typedef struct _NET_GET_SET_PHYSICAL_ADDRESS
{
    MAC_ADDRESS             Address;
} NET_GET_SET_PHYSICAL_ADDRESS, *PNET_GET_SET_PHYSICAL_ADDRESS;

typedef struct _NET_GET_SET_DEVICE_STATUS
{
    NETWORK_DEVICE_STATUS   DeviceStatus;
} NET_GET_SET_DEVICE_STATUS, *PNET_GET_SET_DEVICE_STATUS;

typedef struct _NET_GET_LINK_STATUS
{
    BOOLEAN                 LinkUp;
} NET_GET_LINK_STATUS, *PNET_GET_LINK_STATUS;

#define IOCTL_DISK_GET_LENGTH_INFO          0x0
#define IOCTL_DISK_LAYOUT_INFO              0x1
#define IOCTL_VOLUME_PARTITION_INFO         0x2
#define IOCTL_NET_RECEIVE_FRAME             0x3
#define IOCTL_NET_GET_PHYSICAL_ADDRESS      0x4
#define IOCTL_NET_SET_PHYSICAL_ADDRESS      0x5
#define IOCTL_NET_SEND_FRAME                0x6
#define IOCTL_NET_GET_DEVICE_STATUS         0x7
#define IOCTL_NET_SET_DEVICE_STATUS         0x8
#define IOCTL_NET_GET_LINK_STATUS           0x9

// end of common packing
#pragma warning(pop)
#pragma pack(pop)
