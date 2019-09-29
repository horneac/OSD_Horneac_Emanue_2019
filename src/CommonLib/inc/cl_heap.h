#pragma once

C_HEADER_START
#include "list.h"

// memory alignment in case none is specified by the caller
#define HEAP_DEFAULT_ALIGNMENT          16

#define PoolAllocateZeroMemory          0x2 // memory allocated will be zeroed
#define PoolAllocatePanicIfFail         0x4 // system will PANIC in case the allocation will fail

typedef struct _HEAP_HEADER
{
    DWORD               Magic;              // used for error checking
    QWORD               HeapSizeMaximum;    // the maximum size of the HEAP
    QWORD               HeapSizeRemaining;  // the size remaining to allocated
    QWORD               BaseAddress;        // heap structure base address
    QWORD               FreeAddress;        // address from which to start search
    QWORD               HeapNumberOfAllocations;

    PLIST_ENTRY         EntryToRestartSearch;

    // list of heap allocations
    // this list is always ordered by address
    LIST_ENTRY          HeapAllocations;
} HEAP_HEADER, *PHEAP_HEADER;

//******************************************************************************
// Function:    HeapInit
// Description: Initializes the heap system. Needs to be called before any
//              allocations are made.
// Returns:     STATUS
// Parameter:   IN PVOID BaseAddress     - address from which allocations will
//                                         start
// Parameter:   IN QWORD MemoryAvailable - bytes available for the heap
//******************************************************************************
STATUS
ClHeapInit(
    _Notnull_                           PVOID                   BaseAddress,
    IN                                  QWORD                   MemoryAvailable,
    OUT_PTR                             PHEAP_HEADER*           HeapHeader
    );

//******************************************************************************
// Function:    HeapAllocatePoolWithTag
// Description: Allocates a number of bytes with a specified tag and returns
//              the address.
// Returns:     PVOID - The newly allocated memory region
// Parameter:   IN DWORD Flags
// Parameter:   IN DWORD AllocationSize - Number of bytes to allocate
// Parameter:   IN DWORD Tag
// Parameter:   IN DWORD AllocationAlignment
//******************************************************************************
_Always_(_When_(IsBooleanFlagOn(Flags,PoolAllocatePanicIfFail), RET_NOT_NULL))
PTR_SUCCESS
PVOID
ClHeapAllocatePoolWithTag(
    INOUT   PHEAP_HEADER            HeapHeader,
    IN      DWORD                   Flags,
    IN      DWORD                   AllocationSize,
    IN      DWORD                   Tag,
    IN      DWORD                   AllocationAlignment
    );

//******************************************************************************
// Function:    HeapFreePoolWithTag
// Description: Frees a previously allocated memory region.
// Returns:     void
// Parameter:   IN PVOID MemoryAddress
// Parameter:   IN DWORD Tag - MUST match tag used for allocation
//******************************************************************************
void
ClHeapFreePoolWithTag(
    INOUT   PHEAP_HEADER            HeapHeader,
    _Pre_notnull_ _Post_ptr_invalid_
            PVOID                   MemoryAddress,
    IN      DWORD                   Tag
    );
C_HEADER_END
