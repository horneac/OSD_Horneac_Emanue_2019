#include "common_lib.h"
#include "cl_heap.h"

// 64KB is the minimum heap size required to initialize the system
#define HEAP_MINIMUM_SIZE               (64*KB_SIZE)

// random HEAP_MAGIC number
#define HEAP_MAGIC                      0xACE2302E

#define HEAP_FREE_PATTERN               0xAF
#define HEAP_TAIL_SIZE                  sizeof(DWORD)

typedef struct _HEAP_TAIL
{
    DWORD               Magic;
} HEAP_TAIL, *PHEAP_TAIL;
STATIC_ASSERT(sizeof(HEAP_TAIL) == HEAP_TAIL_SIZE);

/*
----------------------------------------------------------------
-           Magic
-           Tag
-           Size
-           Offset
-           ListEntry
-           Data






-           Magic
----------------------------------------------------------------
*/
typedef
_Struct_size_bytes_(sizeof(HEAP_ENTRY) + Size + sizeof(HEAP_TAIL))
struct _HEAP_ENTRY
{
    DWORD               Magic;          // 0x0
    DWORD               Tag;            // 0x4
    DWORD               Size;           // 0x8  (sizeof actual data allocated(without header) and without MAGIC at the end of the data allocated)
    DWORD               Offset;         // 0xC  (offset to the data(may depend on the alignment)
    LIST_ENTRY          ListEntry;      // 0x10
} HEAP_ENTRY, *PHEAP_ENTRY;             // sizeof(HEAP_ENTRY) = 0x20

//******************************************************************************
// Function:    InitHeapEntry
// Description: Initializes the memory area for the new allocation and updates
//              the global HEAP_HEADER structure.
// Returns:     QWORD
// Parameter:   OUT PHEAP_ENTRY * HeapEntry
// Parameter:   IN DWORD Tag
// Parameter:   IN DWORD Size
// Parameter:   IN DWORD Alignment
// Parameter:   IN BOOLEAN AddToLinkedList
// Parameter:   IN QWORD SizeAvailable
//******************************************************************************
static
QWORD
_InitHeapEntry(
    INOUT    PHEAP_HEADER    HeapHeader,
    INOUT    PHEAP_ENTRY*    HeapEntry,
    IN       DWORD           Tag,
    IN       DWORD           Size,
    IN       DWORD           Alignment,
    IN       BOOLEAN         AddToLinkedList,
    IN       QWORD           SizeAvailable
    );

static
BOOL_SUCCESS
BOOLEAN
_ValidateHeapEntry(
    IN      PHEAP_ENTRY     HeapEntry,
    IN      DWORD           Tag
    );

STATUS
ClHeapInit(
    _Notnull_                           PVOID                   BaseAddress,
    IN                                  QWORD                   MemoryAvailable,
    OUT_PTR                             PHEAP_HEADER*           HeapHeader
    )
{
    PHEAP_HEADER pHeapHeader;

    if (BaseAddress == NULL)
    {
        return STATUS_INVALID_PARAMETER1;
    }

    if (MemoryAvailable < HEAP_MINIMUM_SIZE)
    {
        return STATUS_HEAP_TOO_SMALL;
    }

    if (HeapHeader == NULL)
    {
        return STATUS_INVALID_PARAMETER3;
    }

    pHeapHeader = BaseAddress;

    pHeapHeader->Magic = HEAP_MAGIC;
    pHeapHeader->BaseAddress = ( QWORD ) BaseAddress;
    pHeapHeader->HeapSizeMaximum = MemoryAvailable;
    pHeapHeader->HeapSizeRemaining = pHeapHeader->HeapSizeMaximum - sizeof( HEAP_HEADER );
    pHeapHeader->HeapNumberOfAllocations = 0;
    pHeapHeader->EntryToRestartSearch = &pHeapHeader->HeapAllocations;

    pHeapHeader->FreeAddress = pHeapHeader->BaseAddress + sizeof( HEAP_HEADER );
    InitializeListHead( &pHeapHeader->HeapAllocations );

    *HeapHeader = pHeapHeader;

    return STATUS_SUCCESS;
}

_Always_(_When_(IsBooleanFlagOn(Flags, PoolAllocatePanicIfFail), RET_NOT_NULL))
PTR_SUCCESS
PVOID
ClHeapAllocatePoolWithTag(
    INOUT   PHEAP_HEADER            HeapHeader,
    IN      DWORD                   Flags,
    IN      DWORD                   AllocationSize,
    IN      DWORD                   Tag,
    IN      DWORD                   AllocationAlignment
    )
{
    STATUS status;
    INT64 remainingSizeUntilEnd;
    INT64 remainingSizeBeforeCurrent;
    QWORD sizeRequired;
    QWORD startAddress;
    DWORD alignment;
    INT64 sizeBetweenEntries;
    PVOID mappedAddress;
    HEAP_ENTRY* pNewHeapEntry;
    BOOLEAN found;
    QWORD tempAddress;

    LIST_ENTRY* pCurEntry;
    HEAP_ENTRY* pCurHeapEntry;

    LIST_ENTRY* pPreviousEntry;
    HEAP_ENTRY* pPreviousHeapEntry;

    ASSERT( NULL != HeapHeader );

    found = FALSE;
    status = STATUS_SUCCESS;
    startAddress = 0;
    pNewHeapEntry = NULL;
    mappedAddress = NULL;

    tempAddress = 0;
    sizeBetweenEntries = 0;
    pCurEntry = NULL;
    pCurHeapEntry = NULL;
    pPreviousEntry = NULL;
    pPreviousHeapEntry = NULL;
    remainingSizeUntilEnd = 0;
    remainingSizeBeforeCurrent = 0;

    __try
    {
        if (0 == AllocationSize)
        {
            status = STATUS_INVALID_PARAMETER2;
            __leave;
        }

        if (0 == Tag)
        {
            // we need a tag
            status = STATUS_INVALID_PARAMETER3;
            __leave;
        }

        if (0 == AllocationAlignment)
        {
            alignment = HEAP_DEFAULT_ALIGNMENT;
        }
        else
        {
            alignment = AllocationAlignment;
        }

        sizeRequired = AllocationSize + sizeof(HEAP_ENTRY) + sizeof(HEAP_TAIL);

        if (sizeRequired > HeapHeader->HeapSizeRemaining)
        {
            // we clearly have no chance of allocating more space
            status = STATUS_HEAP_NO_MORE_MEMORY;
            __leave;
        }

        // this is how much size for data we have available until the end of the heap
        remainingSizeUntilEnd = HeapHeader->BaseAddress + HeapHeader->HeapSizeMaximum - HeapHeader->FreeAddress - sizeof(HEAP_ENTRY);
        if ((INT64)sizeRequired <= remainingSizeUntilEnd)
        {
            // it's ok we'll place it here, this is the easier case
            pNewHeapEntry = (HEAP_ENTRY*)HeapHeader->FreeAddress;

            tempAddress = _InitHeapEntry(HeapHeader, &pNewHeapEntry, Tag, AllocationSize, alignment, TRUE, remainingSizeUntilEnd);
            if (0 != tempAddress)
            {
                HeapHeader->FreeAddress = tempAddress;
                found = TRUE;
            }
        }

        if (!found)
        {
            // so we don't have space at the end
            // let's check if there's a chance for us to have space before our current location

            // this is the rest of the space we have
            remainingSizeBeforeCurrent = HeapHeader->HeapSizeRemaining - remainingSizeUntilEnd - sizeof(HEAP_ENTRY);
            if ((INT64)sizeRequired > remainingSizeBeforeCurrent)
            {
                // we have nowhere to allocate memory
                status = STATUS_HEAP_NO_MORE_MEMORY;
                __leave;
            }
            else
            {
                // we need to go to through the Linked List and find a free spot between 2 entries


                // there is no way the list can be empty and we still don't have nay memory remaining
                ASSERT(!IsListEmpty(&(HeapHeader->HeapAllocations)));

                pPreviousEntry = HeapHeader->EntryToRestartSearch->Flink;
                pPreviousHeapEntry = CONTAINING_RECORD(pPreviousEntry, HEAP_ENTRY, ListEntry);
                pCurEntry = pPreviousEntry->Flink;

                while (HeapHeader->EntryToRestartSearch != pCurEntry)
                {
                    if ((pCurEntry == &HeapHeader->HeapAllocations) ||
                        (pPreviousEntry == &HeapHeader->HeapAllocations))
                    {
                        goto next_loop;
                    }

                    ASSERT(NULL != pPreviousHeapEntry);

                    pCurHeapEntry = CONTAINING_RECORD(pCurEntry, HEAP_ENTRY, ListEntry);
                    startAddress = ((QWORD)pPreviousHeapEntry + sizeof(HEAP_ENTRY) + pPreviousHeapEntry->Size + sizeof(HEAP_TAIL));
                    sizeBetweenEntries = (INT64)((QWORD)pCurHeapEntry - startAddress);

                    if (sizeBetweenEntries >= (INT64)sizeRequired)
                    {
                        // we can squeeze this entry between these 2
                        pNewHeapEntry = (HEAP_ENTRY*)startAddress;

                        // here we need to add it manually to the linked list
                        // => FALSE AddToLinkedList parameter, we only need the memory pointer
                        // and to update the heap structures
                        tempAddress = _InitHeapEntry(HeapHeader, &pNewHeapEntry, Tag, AllocationSize, alignment, FALSE, sizeBetweenEntries);
                        if (0 == tempAddress)
                        {
                            // _InitHeapEntry also calculates alignment requirements
                            // => it's possible in this space that we still couldn't fit the data
                            // but maybe we'll be able to on the next iteration
                            goto next_loop;
                        }

                        ASSERT(_ValidateHeapEntry(pPreviousHeapEntry, pPreviousHeapEntry->Tag));

                        // now we insert it in the appropriate position
                        InsertHeadList(pPreviousEntry, &(pNewHeapEntry->ListEntry));

                        HeapHeader->EntryToRestartSearch = &pNewHeapEntry->ListEntry;
                        found = TRUE;
                    }

                    if (found)
                    {
                        break;
                    }

                next_loop:
                    // we update the pointers
                    pPreviousEntry = pCurEntry;
                    pPreviousHeapEntry = pCurHeapEntry;
                    pCurEntry = pCurEntry->Flink;
                }

                if (!found)
                {
                    status = STATUS_HEAP_NO_MORE_MEMORY;
                    __leave;
                }
            }
        }

        mappedAddress = ((BYTE*)pNewHeapEntry) + pNewHeapEntry->Offset;
    }
    __finally
    {
        if (IsFlagOn(Flags, PoolAllocatePanicIfFail))
        {
            // we must succeed
            ASSERT_INFO(SUCCEEDED(status), "Operation failed with status: 0x%x\n", status);
            ASSERT(NULL != mappedAddress);
        }

        if (SUCCEEDED(status))
        {
            if (IsFlagOn(Flags, PoolAllocateZeroMemory))
            {
                memzero(mappedAddress, AllocationSize);
            }
        }
    }

    return mappedAddress;
}

void
ClHeapFreePoolWithTag(
    INOUT   PHEAP_HEADER            HeapHeader,
    _Pre_notnull_ _Post_ptr_invalid_
            PVOID                   MemoryAddress,
    IN      DWORD                   Tag
    )
{
    HEAP_ENTRY* pHeapEntry;
    LIST_ENTRY* pListEntry;
    HEAP_ENTRY* pPreviousHeapEntry;
    QWORD endAddress;
    QWORD previousAddress;
    QWORD heapEntrySize;

    ASSERT( NULL != HeapHeader );
    ASSERT( NULL != MemoryAddress );
    ASSERT( 0 != Tag );

    pPreviousHeapEntry = NULL;
    pListEntry = NULL;
    previousAddress = 0;

    pHeapEntry = ( HEAP_ENTRY* ) ( ( BYTE*) MemoryAddress - sizeof( HEAP_ENTRY ) );

    endAddress = ( QWORD ) MemoryAddress + pHeapEntry->Size + sizeof( HEAP_TAIL );

    // sanity checks
    ASSERT(_ValidateHeapEntry(pHeapEntry,Tag));

    pListEntry = pHeapEntry->ListEntry.Blink;

    if( &( HeapHeader->HeapAllocations ) == pListEntry )
    {
        pListEntry = NULL;
    }

    if( NULL != pListEntry )
    {
        pPreviousHeapEntry = CONTAINING_RECORD( pListEntry, HEAP_ENTRY, ListEntry );
        previousAddress = ( QWORD) pPreviousHeapEntry + sizeof( HEAP_ENTRY ) + pPreviousHeapEntry->Size + sizeof( HEAP_TAIL );
    }
    else
    {
        // the list of allocations is empty
        previousAddress = HeapHeader->BaseAddress + sizeof( HEAP_HEADER );
    }

    // remove the element from the list of allocations
    RemoveEntryList( &( pHeapEntry->ListEntry ) );
    heapEntrySize = pHeapEntry->Size + sizeof(HEAP_ENTRY) + sizeof(HEAP_TAIL);

    // memzero is done only for easier debugging
    ASSERT( heapEntrySize <= MAX_DWORD );
    memset( pHeapEntry, HEAP_FREE_PATTERN, (DWORD) heapEntrySize );

    // we should really increment remaining heap size
    HeapHeader->HeapSizeRemaining = HeapHeader->HeapSizeRemaining + heapEntrySize;
    HeapHeader->HeapNumberOfAllocations = HeapHeader->HeapNumberOfAllocations - 1;

    if( endAddress == HeapHeader->FreeAddress )
    {
        // we were the last ones here => we hand the staff to the previous entry
        HeapHeader->FreeAddress = previousAddress;
    }
}

static
QWORD
_InitHeapEntry(
    INOUT   PHEAP_HEADER    HeapHeader,
    INOUT   PHEAP_ENTRY*    HeapEntry,
    IN      DWORD           Tag,
    IN      DWORD           Size,
    IN      DWORD           Alignment,
    IN      BOOLEAN         AddToLinkedList,
    IN      QWORD           SizeAvailable
    )
{
    QWORD unalignedDataAddress;
    QWORD dataAddress;
    PHEAP_TAIL pHeapTail;
    HEAP_ENTRY* pHeapEntry;
    DWORD heapEntrySize;

    pHeapTail = NULL;
    pHeapEntry = *HeapEntry;

    unalignedDataAddress = ( QWORD ) ( ( (BYTE*) pHeapEntry ) + sizeof( HEAP_ENTRY ) );

    // the address needs to be aligned
    dataAddress = AlignAddressUpper( unalignedDataAddress, Alignment );

    heapEntrySize = Size + sizeof(HEAP_ENTRY) + sizeof(HEAP_TAIL);

    if( SizeAvailable < ( dataAddress - unalignedDataAddress + heapEntrySize ) )
    {
        // we don't have enough space to allocate this entry
        return 0;
    }

    // we need to remap the HeapEntry to be just before the new address
    pHeapEntry = ( HEAP_ENTRY* ) ( dataAddress - sizeof( HEAP_ENTRY ) );
    *HeapEntry = pHeapEntry;

    pHeapEntry->Magic = HEAP_MAGIC;
    pHeapEntry->Size = Size;
    pHeapEntry->Tag = Tag;

    // we insert the new element to the list
    if( AddToLinkedList )
    {
        InsertTailList( &( HeapHeader->HeapAllocations ), &(pHeapEntry->ListEntry) );
    }


    pHeapEntry->Offset = ( DWORD ) ( dataAddress - ( QWORD ) pHeapEntry );

    // we also have a magic field to append at the end
    pHeapTail = ( PHEAP_TAIL )( dataAddress + pHeapEntry->Size );

    // set the magic field
    pHeapTail->Magic = HEAP_MAGIC;

    HeapHeader->HeapSizeRemaining = HeapHeader->HeapSizeRemaining - heapEntrySize;
    HeapHeader->HeapNumberOfAllocations = HeapHeader->HeapNumberOfAllocations + 1;

    // has the possibly of becoming the first free entry list
    return (QWORD) pHeapTail + sizeof(HEAP_TAIL);
}

static
BOOL_SUCCESS
BOOLEAN
_ValidateHeapEntry(
    IN      PHEAP_ENTRY     HeapEntry,
    IN      DWORD           Tag
    )
{
    PHEAP_TAIL pHeapTail;
    BOOLEAN bResult;
    DWORD totalSize;

    ASSERT( NULL != HeapEntry );
    ASSERT( 0 != Tag );

    bResult = TRUE;

    if (HEAP_MAGIC != HeapEntry->Magic)
    {
        bResult = FALSE;
    }

    totalSize = sizeof(HEAP_ENTRY) + HeapEntry->Size + sizeof(HEAP_TAIL);
    pHeapTail = (PHEAP_TAIL)((PBYTE)HeapEntry + totalSize - sizeof(HEAP_TAIL));

    if (HEAP_MAGIC != pHeapTail->Magic)
    {
        bResult = FALSE;
    }

    if (0 == HeapEntry->Tag)
    {
        bResult = FALSE;
    }

    if (Tag != HeapEntry->Tag)
    {
        bResult = FALSE;
    }

    return bResult;
}
