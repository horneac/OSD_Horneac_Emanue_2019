#pragma once

C_HEADER_START
typedef struct _CL_SLIST_ENTRY
{
    struct _CL_SLIST_ENTRY*    Next;
} CL_SLIST_ENTRY, *PCL_SLIST_ENTRY;

__forceinline
void
ClInitializeSListHead(
    OUT     PCL_SLIST_ENTRY ListHead
    )
{
    ListHead->Next = NULL;
}

__forceinline
PCL_SLIST_ENTRY
ClPopEntryList(
    INOUT PCL_SLIST_ENTRY ListHead
)
{
    PCL_SLIST_ENTRY FirstEntry;

    FirstEntry = ListHead->Next;
    if (FirstEntry != NULL)
    {
        ListHead->Next = FirstEntry->Next;
    }

    return FirstEntry;
}

__forceinline
void
ClPushEntryList(
    INOUT PCL_SLIST_ENTRY ListHead,
    INOUT PCL_SLIST_ENTRY Entry
)
{
    Entry->Next = ListHead->Next;
    ListHead->Next = Entry;
    return;
}
C_HEADER_END
