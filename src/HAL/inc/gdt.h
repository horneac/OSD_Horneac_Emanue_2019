#pragma once

#define PREDEFINED_GDT_SIZE                     10
#define PREDEFINED_GDT32_SIZE                   6
#define PREDEFINED_TSS_DESC_SIZE                16
#define PREDEFINED_SEG_DESC_SIZE                8


// Privilege levels
#define RING_ZERO_PL                0
#define RING_THREE_PL               3

#define NO_OF_PRIVILLEGE_LEVELS     3

// If the S bit is 0 these apply to the Type field of the descriptor
// 3.5, Vol 3, No. 56
typedef enum _SYSTEM_SEGMENT_TYPES
{
    SystemSegmentLDT = 2,
    SystemSegment64BitTssAvailable = 9,
    SystemSegment64BitTssBusy = 11,
    SystemSegment64BitTaskGate = 12,
    SystemSegment64BitInterruptGate = 14,
    SystemSegment64BitTrapGate = 15
} SYSTEM_SEGMENT_TYPES;

// If the S bit is 1 these apply to the Type field of the descriptor
// 3.4.5.1, Vol 3, No. 56
typedef enum _NON_SYSTEM_SEGMENT_TYPES
{
    // data segments
    NonSystemSegmentDataReadOnly,
    NonSystemSegmentDataReadOnlyAccessed,
    NonSystemSegmentDataReadWrite,
    NonSystemSegmentDataReadWriteAccessed,
    NonSystemSegmentDataReadOnlyExpandDown,
    NonSystemSegmentDataReadOnlyExpandDownAccessed,
    NonSystemSegmentDataReadWriteExpandDown,
    NonSystemSegmentDataReadWriteExpandDownAccessed,

    // code segments
    NonSystemSegmentCodeExecuteOnly,
    NonSystemSegmentCodeExecuteOnlyAccessed,
    NonSystemSegmentCodeExecuteRead,
    NonSystemSegmentCodeExecuteReadAccessed,
    NonSystemSegmentCodeExecuteOnlyConforming,
    NonSystemSegmentCodeExecuteOnlyConformingAccessed,
    NonSystemSegmentCodeExecuteReadConforming,
    NonSystemSegmentCodeExecuteReadConformingAccessed
} NON_SYSTEM_SEGMENT_TYPES;

#pragma pack(push,1)

#pragma warning(push)

//warning C4214: nonstandard extension used : bit field types other than int
#pragma warning(disable:4214)

// 3.4.5, Vol 3, No. 56
typedef struct _SEGMENT_DESCRIPTOR
{
    // 15:0
    WORD                SegmentLimitLow;            // Segment Limit 15:0

                                                    // 31:16
    WORD                BaseAddressLow;             // Base Address 15:0

                                                    // 39:32
    BYTE                BaseAddressMid;             // Base Address 23:16

                                                    // 43:40
    BYTE                Type                : 4;

    // 44
    BYTE                DescriptorType      : 1;    // (0 = System; 1 = code or data)

                                                    // 46:45
    BYTE                DPL                 : 2;

    // 47
    BYTE                Present             : 1;

    // 51:48
    BYTE                SegmentLimitHigh    : 4;    // Segment Limit 19:16

                                                    // 52
    BYTE                AVL                 : 1;    // Available for use by system SW

                                                    // 53
    BYTE                L                   : 1;    // 1 = 64-bit code segment

                                                    // 54
    BYTE                D_B                 : 1;    // Default Operation Size

                                                    // 55
    BYTE                G                   : 1;    // granularity

                                                    // 63:56
    BYTE                BaseAddressHigh;            // Base Address 31:24

} SEGMENT_DESCRIPTOR, *PSEGMENT_DESCRIPTOR;
STATIC_ASSERT(sizeof(SEGMENT_DESCRIPTOR) == PREDEFINED_SEG_DESC_SIZE);

typedef struct _GDT32
{
    WORD                Limit;

    DWORD               Base;
} GDT32, *PGDT32;
STATIC_ASSERT(sizeof(GDT32) == PREDEFINED_GDT32_SIZE);

typedef struct _GDT
{
    // IDTR(Limit) <-- SRC[0:15];
    WORD                    Limit;

    // IDTR(Base)  <-- SRC[16:79];
    SEGMENT_DESCRIPTOR*     Base;
} GDT, *PGDT;
STATIC_ASSERT(sizeof(GDT) == PREDEFINED_GDT_SIZE);
#pragma warning(pop)
#pragma pack(pop)

typedef enum _SEGMENT_DESCRIPTOR_FLAGS
{
    SegmentDescriptorFlagRing0,
    SegmentDescriptorLongMode,
    SegmentDescriptor32bitOperation,
    SegmentDescriptorSystemDescriptor
} SEGMENT_DESCRIPTOR_FLAGS;


//******************************************************************************
// Function:     GdtInitialize
// Description:  Initializes HALs empty GDT table without loading it for
//               system use.
// Returns:      STATUS
// Parameter:    void
//******************************************************************************
STATUS
GdtInitialize(
    void
    );

//******************************************************************************
// Function:     GdtInstallDescriptor
// Description:  Installs a new descriptor in the GDT table at the specified
//               index. The limit is automatically set to MAX_DWORD for non-TSS
//               entries and to the size of the TSS in case such a segment is
//               installed.
// Returns:      STATUS
// Parameter:    IN WORD GdtIndex - BYTE-counted index in the GDT.
// Parameter:    IN QWORD Base - Should be 0 for non-TSS descriptors.
// Parameter:    IN BYTE Type
// Parameter:    IN SEGMENT_DESCRIPTOR_FLAGS Flags
//******************************************************************************
STATUS
GdtInstallDescriptor(
    IN          WORD                        GdtIndex,
    IN          QWORD                       Base,
    IN          BYTE                        Type,
    IN          SEGMENT_DESCRIPTOR_FLAGS    Flags
    );

//******************************************************************************
// Function:     GdtIsSegmentPrivileged
// Description:  Checks if a segment selector is privileged (ring 0) or not.
// Returns:      BOOLEAN - TRUE if the selector is privileged, FALSE otherwise
// Parameter:    IN WORD Selector
//******************************************************************************
BOOLEAN
GdtIsSegmentPrivileged(
    IN          WORD            Selector
    );

//******************************************************************************
// Function:     GdtReload
// Description:  Reloads GDT with specified code and data selector.
// Returns:      void
// Parameter:    IN WORD CodeSelector
// Parameter:    IN WORD DataSelector
//******************************************************************************
void
GdtReload(
    IN          WORD            CodeSelector,
    IN          WORD            DataSelector
    );
