#pragma once
//******************************************************************************
// Doubly linked list.
//
// This implementation of a doubly linked list does not require use of
// dynamically allocated memory. Instead, each structure that is a potential
// list element must embed a LIST_ENTRY member. All of the list functions
// operate on these LIST_ENTRY elements. The CONTAINING_RECORD macro allows
// conversion from a LIST_ENTRY element back to a structure object that contains
// it.
//
// For example, suppose there is a need for a list of FOO structures. The FOO
// structure should contain a LIST_ENTRY member, like so:
//
// typedef struct _FOO
// {
//      DWORD           SomeData;
//      LIST_ENTRY      Element;
//      DWORD           MoreData;
// } FOO, *PFOO;
//
// Then a list of FOO structures can be be declared and initialized like so:
//
// LIST_ENTRY fooList;
//
// InitializeListHead(&fooList);
//
// Iteration is a typical situation where it is necessary to convert from a
// LIST_ENTRY structure back to its enclosing structure. Here's an example
// where the fooList is iterated and all the elements whose SomeData field
// equal DataToSearch are deleted:
//
// LIST_ITERATOR it;
// ListIteratorInit(&fooList, &it);
//
// PLIST_ENTRY pEntry;
// while ((pEntry = ListIteratorNext(&it)) != NULL)
// {
//      PFOO pFoo = CONTAINING_RECORD(pEntry, FOO, Element);
//      if (pFoo->SomeData == DataToSearch) RemoveEntryList(pEntry);
// }
//
// The interface for this list is inspired by the LIST_ENTRY implementation
// from Microsoft. If you're familiar with those LIST_ENTRY structures, you
// should find this easy to use.
//******************************************************************************

C_HEADER_START

#define INVALID_LIST_SIZE       MAX_DWORD

#pragma pack(push,16)
typedef struct _LIST_ENTRY
{
    struct _LIST_ENTRY*     Flink;
    struct _LIST_ENTRY*     Blink;
} LIST_ENTRY, *PLIST_ENTRY;
#pragma pack(pop)

typedef struct _LIST_ITERATOR
{
    PLIST_ENTRY                 ListHead;

    PLIST_ENTRY                 CurrentEntry;
} LIST_ITERATOR, *PLIST_ITERATOR;

typedef
STATUS
(__cdecl FUNC_ListFunction) (
    IN      PLIST_ENTRY     ListEntry,
    IN_OPT  PVOID           FunctionContext
    );

typedef FUNC_ListFunction*      PFUNC_ListFunction;

//******************************************************************************
// Function:     FUNC_CompareFunction
// Description:  Compares two list elements.
// Returns:      INT64 - Returns a negative value if FirstElem is smaller than
//               SecondElem, a positive value if FirstElem is greater than
//               SecondElem and zero otherwise.
// Parameter:    IN PLIST_ENTRY FirstElem
// Parameter:    IN PLIST_ENTRY SecondElem
//******************************************************************************
typedef
INT64
(__cdecl FUNC_CompareFunction) (
    IN      PLIST_ENTRY     FirstElem,
    IN      PLIST_ENTRY     SecondElem,
    IN_OPT  PVOID           Context
    );

typedef FUNC_CompareFunction*   PFUNC_CompareFunction;

//******************************************************************************
// Function:     InitializeListHead
// Description:  Initializes the head of a doubly linked list.
// Returns:      void
// Parameter:    OUT PLIST_ENTRY ListHead
// NOTE:         Must be called before any other operation is performed on the
//               list.
//******************************************************************************
void
InitializeListHead(
    OUT     PLIST_ENTRY ListHead
    );


//******************************************************************************
// Function:     IsListEmpty
// Description:
// Returns:      BOOLEAN - TRUE if the list is currently empty, FALSE otherwise.
// Parameter:    IN PLIST_ENTRY ListHead
//******************************************************************************
BOOLEAN
IsListEmpty(
    IN      PLIST_ENTRY ListHead
    );

//******************************************************************************
// Function:     IsListEmptyDirty
// Description:  Performs a quick check to see if the list is empty without
//               validating the list head pointers. Should be used only in
//               time-critical situations.
// Returns:      BOOLEAN
// Parameter:    IN PLIST_ENTRY ListHead
//******************************************************************************
__forceinline
BOOLEAN
IsListEmptyDirty(
    IN      PLIST_ENTRY ListHead
    )
{
    return (BOOLEAN)(ListHead->Flink == ListHead);
}

//******************************************************************************
// Function:     RemoveEntryList
// Description:  Removes an entry from the linked list
// Returns:      BOOLEAN - if TRUE the list is empty after the removal
// Parameter:    INOUT PLIST_ENTRY Entry
//******************************************************************************
BOOLEAN
RemoveEntryList(
    INOUT   PLIST_ENTRY Entry
    );

//******************************************************************************
// Function:     RemoveHeadList
// Description:  Removes the head of the linked list
// Returns:      PLIST_ENTRY - pointer to the entry removed; If the list was
//               empty it returns a pointer to the list head
// Parameter:    INOUT PLIST_ENTRY ListHead
//******************************************************************************
PLIST_ENTRY
RemoveHeadList(
    INOUT   PLIST_ENTRY ListHead
    );

//******************************************************************************
// Function:     RemoveTailList
// Description:  Removes the tail of the linked list
// Returns:      PLIST_ENTRY - pointer to the entry removed; If the list was
//               empty it returns a pointer to the list head
// Parameter:    INOUT PLIST_ENTRY ListHead
//******************************************************************************
PLIST_ENTRY
RemoveTailList(
    INOUT   PLIST_ENTRY ListHead
    );

//******************************************************************************
// Function:     InsertTailList
// Description:  Inserts an element to the tail of the list.
// Returns:      void
// Parameter:    INOUT PLIST_ENTRY ListHead
// Parameter:    INOUT PLIST_ENTRY Entry
//******************************************************************************
void
InsertTailList(
    INOUT   PLIST_ENTRY ListHead,
    INOUT   PLIST_ENTRY Entry
    );

//******************************************************************************
// Function:     InsertHeadList
// Description:  Inserts an element to the head of the list
// Returns:      void
// Parameter:    INOUT PLIST_ENTRY ListHead
// Parameter:    INOUT PLIST_ENTRY Entry
//******************************************************************************
void
InsertHeadList(
    INOUT   PLIST_ENTRY ListHead,
    INOUT   PLIST_ENTRY Entry
    );

//******************************************************************************
// Function:     InsertOrderedList
// Description:  Inserts an element into the list following the ordering given
//               by the CompareFunction function.
// Returns:      void
// Parameter:    INOUT PLIST_ENTRY ListHead
// Parameter:    INOUT PLIST_ENTRY Entry
// Parameter:    IN PFUNC_CompareFunction CompareFunction
//******************************************************************************
void
InsertOrderedList(
    INOUT   PLIST_ENTRY             ListHead,
    INOUT   PLIST_ENTRY             Entry,
    IN      PFUNC_CompareFunction   CompareFunction,
    IN_OPT  PVOID                   Context
    );

//******************************************************************************
// Function:     GetListElemByIndex
// Description:  Returns the i-th element from the list.
// Returns:      PLIST_ENTRY - pointer to the ith element; If the list contains
//               less than i+1 elements it returns NULL
// Parameter:    IN PLIST_ENTRY ListHead
// Parameter:    IN DWORD ListIndex
//******************************************************************************
PTR_SUCCESS
PLIST_ENTRY
GetListElemByIndex(
    IN      PLIST_ENTRY ListHead,
    IN      DWORD       ListIndex
    );

//******************************************************************************
// Function:     ListSize
// Description:  Returns the number of elements in the list.
// Returns:      DWORD
// Parameter:    IN PLIST_ENTRY ListHead
// NOTE:         This function is very slow for large lists due to the fact that
//               the whole list is traversed to determine the number of
//               elements.
//******************************************************************************
SIZE_SUCCESS
DWORD
ListSize(
    IN      PLIST_ENTRY ListHead
    );

//******************************************************************************
// Function:      ForEachElementExecute
// Description:   Executes a function with each element of the list as argument.
// Returns:       STATUS
// Parameter:     IN PLIST_ENTRY ListHead   - List on which to execute function.
// Parameter:     IN LIST_FUNCTION Function - Function to execute.
// Parameter:     IN_OPT PVOID Context      - additional context to pass to the
//                                          function.
// Parameter:     IN BOOLEAN AllMustSucceed - if TRUE returns SUCCESS only if
//                                          all function calls succeeded, else
//                                          it's enough for one to succeed.
//******************************************************************************
STATUS
ForEachElementExecute(
    IN      PLIST_ENTRY             ListHead,
    IN      PFUNC_ListFunction      Function,
    IN_OPT  PVOID                   Context,
    IN      BOOLEAN                 AllMustSucceed
    );

//******************************************************************************
// Function:     ListSearchForElement
// Description:  Searches for an element in the list using the CompareFunction
//               function to identify it.
// Returns:      PLIST_ENTRY
// Parameter:    IN PLIST_ENTRY ListHead
// Parameter:    IN PLIST_ENTRY ElementToSearchFor
// Parameter:    IN PFUNC_CompareFunction CompareFunction
//******************************************************************************
PTR_SUCCESS
PLIST_ENTRY
ListSearchForElement(
    IN      PLIST_ENTRY             ListHead,
    IN      PLIST_ENTRY             ElementToSearchFor,
    IN      BOOLEAN                 IsListOrdered,
    IN      PFUNC_CompareFunction   CompareFunction,
    IN_OPT  PVOID                   Context
    );

void
ListIteratorInit(
    IN      PLIST_ENTRY         List,
    OUT     PLIST_ITERATOR      ListIterator
    );

PLIST_ENTRY
ListIteratorNext(
    INOUT   PLIST_ITERATOR      ListIterator
    );
C_HEADER_END
