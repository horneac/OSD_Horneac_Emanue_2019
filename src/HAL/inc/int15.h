#pragma once

#pragma pack(push,1)
typedef enum _MEMORY_MAP_TYPE
{
    MemoryMapTypeUsableRAM = 1,
    MemoryMapTypeReserved,
    MemoryMapTypeACPIReclaimable,
    MemoryMapTypeACPINVSMemory,
    MemoryMapTypeBadMemory,
    MemoryMapTypeMax = MemoryMapTypeBadMemory + 1
} MEMORY_MAP_TYPE;

#define MEMORY_MAP_ENTRY_EA_VALID_ENTRY             ((DWORD)1<<0)
#define MEMORY_MAP_ENTRY_EA_NON_VOLATILE            ((DWORD)1<<1)

typedef struct _INT15_MEMORY_MAP_ENTRY
{
    QWORD           BaseAddress;
    QWORD           Length;
    DWORD           Type;
    DWORD           ExtendedAttributes;
} INT15_MEMORY_MAP_ENTRY, *PINT15_MEMORY_MAP_ENTRY;
#pragma pack(pop)