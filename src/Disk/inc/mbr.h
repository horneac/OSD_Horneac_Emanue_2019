#pragma once

#define MBR_PARTITION_ENTRY_SIZE            16
#define MBR_SIZE                            512
#define MBR_NO_OF_PARTITIONS                4

#define MBR_EXT_PARTITION_NO_OF_ENTRIES     2

#define CHS_ADDRESS_SIZE                    3

// other values are invalid
#define MBR_STATUS_ACTIVE                   0x80
#define MBR_STATUS_INACTIVE                 0x00

// partition types
#define MBR_PARTITION_EXTENDED_CHS          0x5
#define MBR_PARTITION_EXTENDED_LBA          0xF

#pragma pack(push,1)

#pragma warning(push)

// warning C4214: nonstandard extension used: bit field types other than int
#pragma warning(disable:4214)

typedef struct _CHS_ADDRESS
{
    BYTE            Head;
    BYTE            Sector          : 6;
    BYTE            CylinderHigh    : 2;
    BYTE            CylinderLow;
} CHS_ADDRESS, *PCHS_ADDRESS;
STATIC_ASSERT(sizeof(CHS_ADDRESS) == CHS_ADDRESS_SIZE);

typedef struct _MBR_PARTITION_ENTRY
{
    // 0x80 - active (old MBRs only accept this value as valid)
    // 0x00 - inactive
    // 0x1 - 7xF invalid
    BYTE                    Status;
    CHS_ADDRESS             FirstSectorCHS;
    BYTE                    PartitionType;
    CHS_ADDRESS             LastSectorCHS;
    DWORD                   FirstSectorLBA;
    DWORD                   NumberOfSectors;
} MBR_PARTITION_ENTRY, *PMBR_PARTITION_ENTRY;
STATIC_ASSERT(sizeof(MBR_PARTITION_ENTRY) == MBR_PARTITION_ENTRY_SIZE);

typedef struct _MBR
{
    BYTE                    BootstrapCode[446];
    MBR_PARTITION_ENTRY     Partitions[MBR_NO_OF_PARTITIONS];
    WORD                    BootSignature;
} MBR, *PMBR;
STATIC_ASSERT(sizeof(MBR) == MBR_SIZE);

#pragma warning(pop)
#pragma pack(pop)
