#include "common_lib.h"
#include "list.h"

//  Valid list state:
//  |---------| Flink |---------| Flink |---------|
//  |         |------>|         |------>|         |
//  |  Blink  | Blink |  Entry  | Blink |  Flink  |
//  |         |<------|         |<------|         |
//  |---------|       |---------|       |---------|
static
__forceinline
BOOLEAN
_ValidateListEntry(
    IN  PLIST_ENTRY Entry
    )
{
    // check that the backward element points to this entry
    if (Entry->Flink->Blink != Entry)
    {
        return FALSE;
    }

    // check that the forward element points to this entry
    if (Entry->Blink->Flink != Entry)
    {
        return FALSE;
    }

    return TRUE;
}

void
InitializeListHead(
    OUT PLIST_ENTRY ListHead
    )
{
    ListHead->Flink = ListHead->Blink = ListHead;
}


BOOLEAN
IsListEmpty(
    IN  PLIST_ENTRY ListHead
    )
{
#ifdef DEBUG
    ASSERT(_ValidateListEntry(ListHead));
#endif

    return (BOOLEAN)(ListHead->Flink == ListHead);
}

BOOLEAN
RemoveEntryList(
    INOUT PLIST_ENTRY Entry
    )
{
    PLIST_ENTRY Blink;
    PLIST_ENTRY Flink;

#ifdef DEBUG
    ASSERT(_ValidateListEntry(Entry));
#endif

    Flink = Entry->Flink;
    Blink = Entry->Blink;
    Blink->Flink = Flink;
    Flink->Blink = Blink;
    return (BOOLEAN)(Flink == Blink);
}

PLIST_ENTRY
RemoveHeadList(
    INOUT PLIST_ENTRY ListHead
    )
{
    PLIST_ENTRY Flink;
    PLIST_ENTRY Entry;

#ifdef DEBUG
    ASSERT(_ValidateListEntry(ListHead));
#endif

    Entry = ListHead->Flink;
    Flink = Entry->Flink;
    ListHead->Flink = Flink;
    Flink->Blink = ListHead;
    return Entry;
}



PLIST_ENTRY
RemoveTailList(
    INOUT PLIST_ENTRY ListHead
    )
{
    PLIST_ENTRY Blink;
    PLIST_ENTRY Entry;

#ifdef DEBUG
    ASSERT(_ValidateListEntry(ListHead));
#endif

    Entry = ListHead->Blink;
    Blink = Entry->Blink;
    ListHead->Blink = Blink;
    Blink->Flink = ListHead;
    return Entry;
}


void
InsertTailList(
    INOUT PLIST_ENTRY ListHead,
    INOUT PLIST_ENTRY Entry
    )
{
    PLIST_ENTRY Blink;

#ifdef DEBUG
    ASSERT(_ValidateListEntry(ListHead));
#endif

    Blink = ListHead->Blink;
    Entry->Flink = ListHead;
    Entry->Blink = Blink;
    Blink->Flink = Entry;
    ListHead->Blink = Entry;
}


void
InsertHeadList(
    INOUT PLIST_ENTRY ListHead,
    INOUT PLIST_ENTRY Entry
    )
{
    PLIST_ENTRY Flink;

#ifdef DEBUG
    ASSERT(_ValidateListEntry(ListHead));
#endif

    Flink = ListHead->Flink;
    Entry->Flink = Flink;
    Entry->Blink = ListHead;
    Flink->Blink = Entry;
    ListHead->Flink = Entry;
}

void
InsertOrderedList(
    INOUT   PLIST_ENTRY             ListHead,
    INOUT   PLIST_ENTRY             Entry,
    IN      PFUNC_CompareFunction   CompareFunction,
    IN_OPT  PVOID                   Context
    )
{
    PLIST_ENTRY pCurrentEntry;

#ifdef DEBUG
    ASSERT(_ValidateListEntry(ListHead));
#endif

    for (pCurrentEntry = ListHead->Flink;
         pCurrentEntry != ListHead;
         pCurrentEntry = pCurrentEntry->Flink
         )
    {
        if (CompareFunction(Entry, pCurrentEntry, Context) < 0)
        {
            // entry to insert is smaller than current entry
            break;
        }
    }

    InsertTailList(pCurrentEntry, Entry);
}

PTR_SUCCESS
PLIST_ENTRY
GetListElemByIndex(
    IN      PLIST_ENTRY ListHead,
    IN      DWORD       ListIndex
    )
{
    LIST_ENTRY* pCurEntry;
    DWORD index;

    if (NULL == ListHead)
    {
        return NULL;
    }

#ifdef DEBUG
    ASSERT(_ValidateListEntry(ListHead));
#endif

    pCurEntry = ListHead->Flink;

    index = 0;
    while (pCurEntry != ListHead)
    {
        ASSERT_INFO(NULL != pCurEntry, "ListEntry is NULL\n");
        if (index == ListIndex)
        {
            return pCurEntry;
        }

        index = index + 1;
        pCurEntry = pCurEntry->Flink;
    }

    return NULL;
}

SIZE_SUCCESS
DWORD
ListSize(
    IN      PLIST_ENTRY ListHead
    )
{
    DWORD count;
    LIST_ENTRY* pCurEntry;

    if (NULL == ListHead)
    {
        return INVALID_LIST_SIZE;
    }

#ifdef DEBUG
    ASSERT(_ValidateListEntry(ListHead));
#endif

    pCurEntry = ListHead->Flink;

    count = 0;
    while (pCurEntry != ListHead)
    {
        ASSERT_INFO(NULL != pCurEntry, "List entry is NULL\n");

        count = count + 1;
        pCurEntry = pCurEntry->Flink;
    }

    return count;
}

STATUS
ForEachElementExecute(
    IN      PLIST_ENTRY         ListHead,
    IN      PFUNC_ListFunction  Function,
    IN_OPT  PVOID               Context,
    IN      BOOLEAN             AllMustSucceed
    )
{
    STATUS status;
    LIST_ENTRY* pCurListEntry;

    if (NULL == ListHead)
    {
        return STATUS_INVALID_PARAMETER1;
    }

    if (NULL == Function)
    {
        return STATUS_INVALID_PARAMETER2;
    }

#ifdef DEBUG
    ASSERT(_ValidateListEntry(ListHead));
#endif

    status = STATUS_SUCCESS;

    pCurListEntry = ListHead->Flink;
    while (pCurListEntry != ListHead)
    {
        ASSERT_INFO(NULL != pCurListEntry, "List entry is NULL\n");
        status = Function(pCurListEntry, Context);
        if (AllMustSucceed)
        {
            if (!SUCCEEDED(status))
            {
                return status;
            }
        }
        pCurListEntry = pCurListEntry->Flink;
    }

    return status;
}

PTR_SUCCESS
PLIST_ENTRY
ListSearchForElement(
    IN      PLIST_ENTRY             ListHead,
    IN      PLIST_ENTRY             ElementToSearchFor,
    IN      BOOLEAN                 IsListOrdered,
    IN      PFUNC_CompareFunction   CompareFunction,
    IN_OPT  PVOID                   Context
    )
{
    PLIST_ENTRY pCurEntry;

    if (NULL == ListHead)
    {
        return NULL;
    }

    if (NULL == ElementToSearchFor)
    {
        return NULL;
    }

    if (NULL == CompareFunction)
    {
        return NULL;
    }

#ifdef DEBUG
    ASSERT(_ValidateListEntry(ListHead));
#endif

    pCurEntry = ListHead->Flink;
    while (pCurEntry != ListHead)
    {
        INT64 cmpResult = CompareFunction(pCurEntry, ElementToSearchFor, Context);

        if (cmpResult == 0) return pCurEntry;
        if (IsListOrdered && (cmpResult > 0)) return NULL;

        pCurEntry = pCurEntry->Flink;
    }

    return NULL;
}

void
ListIteratorInit(
    IN      PLIST_ENTRY         List,
    OUT     PLIST_ITERATOR      ListIterator
    )
{
    ASSERT(List != NULL);
    ASSERT(ListIterator != NULL);

    ListIterator->ListHead = List;
    ListIterator->CurrentEntry = List->Flink;
}

PLIST_ENTRY
ListIteratorNext(
    INOUT   PLIST_ITERATOR      ListIterator
    )
{
    PLIST_ENTRY pResult;

    ASSERT(ListIterator != NULL);

    pResult = NULL;

    if (ListIterator->CurrentEntry != ListIterator->ListHead)
    {
        pResult = ListIterator->CurrentEntry;

        // advance current entry
        ListIterator->CurrentEntry = ListIterator->CurrentEntry->Flink;
    }

    return pResult;
}
