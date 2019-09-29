#pragma once

#include "gdt.h"

#define PREDEFINED_TSS_SIZE     104

#define NO_OF_IST               7

#pragma pack(push,1)

#pragma warning(push)

//warning C4214: nonstandard extension used : bit field types other than int
#pragma warning(disable:4214)

// 7.7, Vol 3, No. 56
typedef struct _TSS
{
    DWORD           Reserved0;

    QWORD           Rsp[NO_OF_PRIVILLEGE_LEVELS];       // RSP0,1,2 (for Ring3 we don't have a stack)

    QWORD           Reserved1;                          // Probably because of IST[0] which needs to be NULL

    QWORD           IST[NO_OF_IST];                     // IST[1..7]

    QWORD           Reserved2;

    WORD            Reserved3;

    WORD            IOMapBaseAddress;

} TSS, *PTSS;
STATIC_ASSERT( sizeof( TSS ) == PREDEFINED_TSS_SIZE );

// because TSS_DESCRIPTORs are cooler
// 7.2.3, Vol 3., No. 56
typedef struct _TSS_DESCRIPTOR
{
    // 15:0
    WORD                SegmentLimitLow;            // Segment Limit 15:0

    // 31:16
    WORD                BaseAddressLowWord;         // Base Address 15:0

    // 39:32
    BYTE                BaseAddressMidDword;        // Base Address 23:16

    // 43:40
    BYTE                Type                : 4;

    // 44
    BYTE                Zero0               : 1;

    // 46:45
    BYTE                DPL                 : 2;

    // 47
    BYTE                Present             : 1;

    // 51:48
    BYTE                SegmentLimitHigh    : 4;    // Segment Limit 19:16

    // 52
    BYTE                AVL                 : 1;    // Available for use by system SW

    // 53
    BYTE                Zero1               : 1;

    // 54
    BYTE                Zero2               : 1;

    // 55
    BYTE                G                   : 1;     // granularity

    // 63:56
    BYTE                BaseAddressHighDword;       // Base Address 31:24

    // 95:64
    DWORD               BaseAddressHighQword;

    // 127:96
    DWORD               Reserved;

} TSS_DESCRIPTOR, *PTSS_DESCRIPTOR;
STATIC_ASSERT(sizeof(TSS_DESCRIPTOR) == PREDEFINED_TSS_DESC_SIZE);
#pragma warning(pop)
#pragma pack(pop)

//******************************************************************************
// Function:     TssInstall
// Description:  Install a TSS descriptor in the GDT at the specified index, and
//               loads the task register.
// Returns:      STATUS
// Parameter:    OUT PTSS Tss
// Parameter:    IN WORD GdtIndex - BYTE-counted index in the GDT
// Parameter:    IN PVOID StackBase - Stack to use in TSS
// Parameter:    IN BYTE StackIndex - Index to use for stack
//******************************************************************************
STATUS
TssInstall(
    OUT     PTSS        Tss,
    IN      WORD        GdtIndex,
    IN_RANGE(1,NO_OF_IST)
            BYTE        NumberOfStacks,
    IN_READS(NumberOfStacks)
            PVOID*      Stacks
    );