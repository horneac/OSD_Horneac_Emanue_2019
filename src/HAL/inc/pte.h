#pragma once

#define MAXPHYADDR                                  52

#define PAGING_TABLES_FIRST_LEVEL                   1
#define PAGING_TABLES_LAST_LEVEL                    4

// 63:48 must equal bit 47 for the address to be canonical
#define VA_HIGHEST_VALID_BIT                                        47

#define MASK_PML4_OFFSET(va)                        (((QWORD)(va)>>39)&0x1FF)
#define MASK_PDPTE_OFFSET(va)                       (((QWORD)(va)>>30)&0x1FF)
#define MASK_PDE_OFFSET(va)                         (((QWORD)(va)>>21)&0x1FF)
#define MASK_PTE_OFFSET(va)                         (((QWORD)(va)>>12)&0x1FF)
#define MASK_PAGE_OFFSET(va)                        ((QWORD)(va)&0xFFF)

#define SHIFT_FOR_PHYSICAL_ADDR                     PAGE_SHIFT
#define SHIFT_FOR_LARGE_PAGE                        21

#define PAGE_4KB_OFFSET                             ((QWORD)(1<<12)-1)
#define PAGE_2MB_OFFSET                             ((QWORD)(1<<21)-1)
#define PAGE_4MB_OFFSET                             ((QWORD)(1<<22)-1)
#define PAGE_1GB_OFFSET                             ((QWORD)(1<<30)-1)

#define PCID_NO_OF_BITS                             12
#define PCID_TOTAL_NO_OF_VALUES                     (1<<PCID_NO_OF_BITS)
#define PCID_FIRST_VALID_VALUE                      1

#define PCID_IS_VALID(pcid)                         (PCID_FIRST_VALID_VALUE <= (pcid) && (pcid) < PCID_TOTAL_NO_OF_VALUES)

#define MOV_TO_CR3_DO_NOT_INVALIDATE_PCID_MAPPINGS  ((QWORD)1<<63)

typedef WORD PCID;

#pragma pack(push,1)

#pragma warning(push)

// warning C4201: nonstandard extension used: nameless struct/union
#pragma warning(disable:4201)

//warning C4214: nonstandard extension used : bit field types other than int
#pragma warning(disable:4214)

// The PML4 points to the PML4_ENTRY array

// This structure is valid for CR4.PCIDE(bit 17) = 0
typedef union _PML4
{
    union
    {
        struct
        {
            QWORD           PCID                :   PCID_NO_OF_BITS;
            QWORD           PhysicalAddress     :   MAXPHYADDR - 12;
            QWORD           Reserved            :   64 - MAXPHYADDR;
        } Pcide;
        struct
        {
            QWORD           Ignored0            :   3;
            QWORD           PWT                 :   1;
            QWORD           PCD                 :   1;
            QWORD           Ignored1            :   7;
            QWORD           PhysicalAddress     :   MAXPHYADDR - 12;
            QWORD           Reserved            :   64 - MAXPHYADDR;
        } NoPcide;
    };
    QWORD           Raw;
} PML4, *PPML4;
STATIC_ASSERT(sizeof(PML4) == sizeof(QWORD));

// A PML4_ENTRY points to a PDPT
typedef struct _PML4_ENTRY
{
    QWORD           Present             :   1;
    QWORD           ReadWrite           :   1;
    QWORD           UserSupervisor      :   1;
    QWORD           PWT                 :   1;
    QWORD           PCD                 :   1;
    QWORD           Accessed            :   1;
    QWORD           Ignored0            :   1;
    QWORD           Reserved            :   1;  // Must be 0
    QWORD           Ignored1            :   4;
    QWORD           PhysicalAddress     :   MAXPHYADDR - 12;
    QWORD           Ignored2            :   11;
    QWORD           XD                  :   1;
} PML4_ENTRY, *PPML4_ENTRY;
STATIC_ASSERT(sizeof(PML4_ENTRY) == sizeof(QWORD));

// This PDPT_ENTRY points to a PD
typedef struct _PDPT_ENTRY_PD
{
    QWORD           Present             :   1;
    QWORD           ReadWrite           :   1;
    QWORD           UserSupervisor      :   1;
    QWORD           PWT                 :   1;
    QWORD           PCD                 :   1;
    QWORD           Accessed            :   1;
    QWORD           Ignored0            :   1;
    QWORD           PageSize            :   1;  // Must be 0
    QWORD           Ignored1            :   4;
    QWORD           PhysicalAddress     :   MAXPHYADDR - 12;
    QWORD           Ignored2            :   11;
    QWORD           XD                  :   1;
} PDPT_ENTRY_PD, *PPDTP_ENTRY_PD;
STATIC_ASSERT(sizeof(PDPT_ENTRY_PD) == sizeof(QWORD));

// PDPT_ENTRY that maps a 1-GByte Page
typedef struct _PDPT_ENTRY_1G
{
    QWORD           Present             :   1;
    QWORD           ReadWrite           :   1;
    QWORD           UserSupervisor      :   1;
    QWORD           PWT                 :   1;
    QWORD           PCD                 :   1;
    QWORD           Accessed            :   1;
    QWORD           Dirty               :   1;
    QWORD           PageSize            :   1;  // Must be 1
    QWORD           Global              :   1;
    QWORD           Ignored             :   3;
    QWORD           PAT                 :   1;
    QWORD           Reserved            :   17;
    QWORD           PhysicalAddress     :   MAXPHYADDR-30;
    QWORD           Ignored2            :   11;
    QWORD           XD                  :   1;
} PDPT_ENTRY_1G, *PPDTP_ENTRY_1G;
STATIC_ASSERT( sizeof( PDPT_ENTRY_1G ) == sizeof( QWORD ) );

// PD_ENTRY which references a Page Table
typedef struct _PD_ENTRY_PT
{
    QWORD           Present             :   1;
    QWORD           ReadWrite           :   1;
    QWORD           UserSupervisor      :   1;
    QWORD           PWT                 :   1;
    QWORD           PCD                 :   1;
    QWORD           Accessed            :   1;
    QWORD           Ignored0            :   1;
    QWORD           PageSize            :   1;  // Must be 0
    QWORD           Ignored1            :   4;
    QWORD           PhysicalAddress     :   MAXPHYADDR-12;
    QWORD           Ignored2            :   11;
    QWORD           XD                  :   1;
} PD_ENTRY_PT, *PPD_ENTRY_PT;
STATIC_ASSERT( sizeof( PD_ENTRY_PT ) == sizeof( QWORD ) );

// PD_ENTRY that maps a 2-MByte Page
typedef struct _PD_ENTRY_2MB
{
    QWORD           Present             :   1;
    QWORD           ReadWrite           :   1;
    QWORD           UserSupervisor      :   1;
    QWORD           PWT                 :   1;
    QWORD           PCD                 :   1;
    QWORD           Accessed            :   1;
    QWORD           Dirty               :   1;
    QWORD           PageSize            :   1;  // Must be 1
    QWORD           Global              :   1;
    QWORD           Ignored             :   3;
    QWORD           PAT                 :   1;
    QWORD           Reserved            :   8;
    QWORD           PhysicalAddress     :   MAXPHYADDR-21;
    QWORD           Ignored2            :   11;
    QWORD           XD                  :   1;
} PD_ENTRY_2MB, *PPD_ENTRY_2MB;
STATIC_ASSERT( sizeof( PD_ENTRY_2MB ) == sizeof( QWORD ) );

typedef struct _PT_ENTRY
{
    QWORD           Present             :   1;
    QWORD           ReadWrite           :   1;
    QWORD           UserSupervisor      :   1;
    QWORD           PWT                 :   1;
    QWORD           PCD                 :   1;
    QWORD           Accessed            :   1;
    QWORD           Dirty               :   1;
    QWORD           PAT                 :   1;
    QWORD           Global              :   1;
    QWORD           Ignored0            :   3;
    QWORD           PhysicalAddress     :   MAXPHYADDR-12;
    QWORD           Ignored1            :   11;
    QWORD           XD                  :   1;
} PT_ENTRY, *PPT_ENTRY;
STATIC_ASSERT( sizeof( PT_ENTRY ) == sizeof( QWORD ) );

// the next structures are only valid for PAE paging
#define MASK_PAE_PDPTE_OFFSET(va)        (((QWORD)(va)>>30)&0x3)

typedef struct _CR3_PAE_STRUCTURE
{
    QWORD           Ignored0            :   5;
    QWORD           PhysicalAddress     :   MAXPHYADDR - 5;
    QWORD           Ignored1            :   64 - MAXPHYADDR;
} CR3_PAE_STRUCTURE, *PCR3_PAE_STRUCTURE;
STATIC_ASSERT(sizeof(CR3_PAE_STRUCTURE) == sizeof(QWORD));

typedef struct _PDPT_PAE_ENTRY_PD
{
    QWORD           Present             :   1;
    QWORD           Reserved0           :   2;
    QWORD           PWT                 :   1;
    QWORD           PCD                 :   1;
    QWORD           Reserved1           :   4;
    QWORD           Ignored0            :   3;
    QWORD           PhysicalAddress     :   MAXPHYADDR-12;
    QWORD           Reserved2           :   12;
} PDPT_PAE_ENTRY_PD, *PPDPT_PAE_ENTRY_PD;
STATIC_ASSERT(sizeof(PDPT_PAE_ENTRY_PD) == sizeof(QWORD));

typedef struct _PD_PAE_ENTRY_PT
{
    QWORD           Present             :   1;
    QWORD           ReadWrite           :   1;
    QWORD           UserSupervisor      :   1;
    QWORD           PWT                 :   1;
    QWORD           PCD                 :   1;
    QWORD           Accessed            :   1;
    QWORD           Ignored0            :   1;
    QWORD           PageSize            :   1;  // Must be 0
    QWORD           Ignored1            :   4;
    QWORD           PhysicalAddress     :   MAXPHYADDR - 12;
    QWORD           Reserved            :   11;
    QWORD           XD                  :   1;
} PD_PAE_ENTRY_PT, *PPD_PAE_ENTRY_PT;
STATIC_ASSERT(sizeof(PD_PAE_ENTRY_PT) == sizeof(QWORD));

// PD_ENTRY that maps a 2-MByte Page
typedef struct _PD_PAE_ENTRY_2MB
{
    QWORD           Present             :   1;
    QWORD           ReadWrite           :   1;
    QWORD           UserSupervisor      :   1;
    QWORD           PWT                 :   1;
    QWORD           PCD                 :   1;
    QWORD           Accessed            :   1;
    QWORD           Dirty               :   1;
    QWORD           PageSize            :   1;  // Must be 1
    QWORD           Global              :   1;
    QWORD           Ignored             :   3;
    QWORD           PAT                 :   1;
    QWORD           Reserved0           :   8;
    QWORD           PhysicalAddress     :   MAXPHYADDR - 21;
    QWORD           Reserved1           :   11;
    QWORD           XD                  :   1;
} PD_PAE_ENTRY_2MB, *PPD_PAE_ENTRY_2MB;
STATIC_ASSERT(sizeof(PD_PAE_ENTRY_2MB) == sizeof(QWORD));

typedef struct _PT_PAE_ENTRY
{
    QWORD           Present             :   1;
    QWORD           ReadWrite           :   1;
    QWORD           UserSupervisor      :   1;
    QWORD           PWT                 :   1;
    QWORD           PCD                 :   1;
    QWORD           Accessed            :   1;
    QWORD           Dirty               :   1;
    QWORD           PAT                 :   1;
    QWORD           Global              :   1;
    QWORD           Ignored             :   3;
    QWORD           PhysicalAddress     :   MAXPHYADDR-12;
    QWORD           Reserved            :   11;
    QWORD           XD                  :   1;
} PT_PAE_ENTRY, *PPT_PAE_ENTRY;
STATIC_ASSERT(sizeof(PT_PAE_ENTRY) == sizeof(QWORD));

// the next structures are only valid for 32 bit paging

#define MASK_PDE32_OFFSET(va)    (((QWORD)(va)>>22)&0x3FF)
#define MASK_PTE32_OFFSET(va)    (((QWORD)(va)>>12)&0x3FF)


typedef struct _CR3_STRUCTURE
{
    DWORD            Ignored0            :    3;
    DWORD            PWT                 :    1;
    DWORD            PCD                 :    1;
    DWORD            Ignored1            :    7;
    DWORD            PhysicalAddress     :    20;
} CR3_STRUCTURE, *PCR3_STRUCTURE;
STATIC_ASSERT(sizeof(CR3_STRUCTURE) == sizeof(DWORD));

typedef struct _PD32_ENTRY_PT
{
    DWORD           Present              :    1;
    DWORD           ReadWrite            :    1;
    DWORD           UserSupervisor       :    1;
    DWORD           PWT                  :    1;
    DWORD           PCD                  :    1;
    DWORD           Accessed             :    1;
    DWORD           Ignored0             :    1;
    DWORD           PageSize             :    1;
    DWORD           Ignored1             :    4;
    DWORD           PhysicalAddress      :    20;
} PD32_ENTRY_PT, *PPD32_ENTRY_PT;
STATIC_ASSERT(sizeof(PD32_ENTRY_PT) == sizeof(DWORD));

typedef struct _PD32_ENTRY_4MB
{
    DWORD           Present              :    1;
    DWORD           ReadWrite            :    1;
    DWORD           UserSupervisor       :    1;
    DWORD           PWT                  :    1;
    DWORD           PCD                  :    1;
    DWORD           Accessed             :    1;
    DWORD           Dirty                :    1;
    DWORD           PageSize             :    1;
    DWORD           Granularity          :    1;
    DWORD           Ignored0             :    3;
    DWORD           PAT                  :    1;
    DWORD           UpperBits            :    9;
    DWORD           PhysicalAddress      :    10;
} PD32_ENTRY_4MB, *PPD32_ENTRY_4MB;
STATIC_ASSERT(sizeof(PD32_ENTRY_4MB) == sizeof(DWORD));

typedef struct _PT32_ENTRY
{
    DWORD           Present              :    1;
    DWORD           ReadWrite            :    1;
    DWORD           UserSupervisor       :    1;
    DWORD           PWT                  :    1;
    DWORD           PCD                  :    1;
    DWORD           Accessed             :    1;
    DWORD           Dirty                :    1;
    DWORD           PAT                  :    1;
    DWORD           Granularity          :    1;
    DWORD           Ignored1             :    3;
    DWORD           PhysicalAddress      :    20;
} PT32_ENTRY, *PPT32_ENTRY;
STATIC_ASSERT(sizeof(PT32_ENTRY) == sizeof(DWORD));

typedef struct _PTE_MAP_FLAGS
{
    WORD            Invalidate           :    1;
    WORD            PatIndex             :    3;
    WORD            Writable             :    1;
    WORD            Executable           :    1;
    WORD            PagingStructure      :    1;
    WORD            UserAccess           :    1;
    WORD            GlobalPage           :    1;
    WORD            __Reserved0          :    7;
} PTE_MAP_FLAGS, *PPTE_MAP_FLAGS;
STATIC_ASSERT(sizeof(PTE_MAP_FLAGS) == sizeof(WORD));
#pragma warning(pop)
#pragma pack(pop)

void
PteMap(
    IN          PVOID               PageTable,
    IN_OPT      PHYSICAL_ADDRESS    PhysicalAddress,
    IN          PTE_MAP_FLAGS       Flags
    );

void
PteUnmap(
    IN          PVOID           PageTable
    );

PHYSICAL_ADDRESS
PteGetPhysicalAddress(
    IN          PVOID           PageTable
    );

PHYSICAL_ADDRESS
PteLargePageGetPhysicalAddress(
    IN          PVOID           PageTable
    );

BOOLEAN
PteIsPresent(
    IN          PVOID           PageTable
    );

__forceinline
void
PageInvalidateTlb(
    IN          PVOID           Page
    )
{
    // This is a HACK done to prevent a Visual C compiler bug which sometimes (if 2 __invlpg are one after another)
    // causes the __invlpg to generate a swapgs instruction :|
    // Yeah, good job Microsoft...
    _ReadWriteBarrier();
    __invlpg(Page);
}
