#include "common_lib.h"
#include "stack_dynamic.h"

typedef struct _STACK
{
    CL_SLIST_ENTRY             TopOfStack;

    DWORD                   NumberOfElements;
} STACK, *PSTACK;

BOOLEAN
(__cdecl StackDynamicPush)(
    INOUT   PSTACK          Stack,
    IN      PSTACK_ITEM     Item
    )
{
    ASSERT(Stack != NULL);
    ASSERT(Item != NULL);

    ClPushEntryList( &Stack->TopOfStack, &Item->Next );

    ASSERT(Stack->NumberOfElements < MAX_DWORD);
    Stack->NumberOfElements++;

    return TRUE;
}

_Must_inspect_result_
_Success_(return != NULL)
STACK_ITEM*
(__cdecl StackDynamicPop)(
    INOUT   PSTACK          Stack
    )
{
    ASSERT(Stack != NULL);

    CL_SLIST_ENTRY *pStackEntry = ClPopEntryList(&Stack->TopOfStack);

    if (pStackEntry == NULL) return NULL;

    ASSERT(Stack->NumberOfElements > 0);
    Stack->NumberOfElements--;

    return CONTAINING_RECORD(pStackEntry, STACK_ITEM, Next);
}

_Must_inspect_result_
_Success_(return != NULL)
STACK_ITEM*
(__cdecl StackDynamicPeek)(
    IN      PSTACK          Stack,
    IN      DWORD           Index
    )
{
    DWORD currentIndex = 0;
    PCL_SLIST_ENTRY pItem = NULL;

    ASSERT(Stack != NULL);

    if (Index >= Stack->NumberOfElements)
    {
        return NULL;
    }

    for(pItem = Stack->TopOfStack.Next;
        pItem != NULL && currentIndex < Index;
        pItem = pItem->Next )
    {
        currentIndex++;
    }

    ASSERT(pItem != NULL);

    return CONTAINING_RECORD(pItem, STACK_ITEM, Next);
}

void
(__cdecl StackDynamicClear)(
    INOUT   PSTACK              Stack,
    IN_OPT  PFUNC_FreeFunction  FreeFunction,
    IN_OPT  PVOID               FreeContext
    )
{
    ASSERT(Stack != NULL);

    while (!StackDynamicIsEmpty(Stack))
    {
        PSTACK_ITEM pItem = StackDynamicPop(Stack);
        ASSERT(pItem != NULL);

        if (FreeFunction != NULL)
        {
            FreeFunction(pItem, FreeContext);
        }
    }
}


BOOLEAN
(__cdecl StackDynamicIsEmpty)(
    INOUT   PSTACK          Stack
    )
{
    ASSERT(Stack != NULL);

    return Stack->TopOfStack.Next == NULL;
}

DWORD
(__cdecl StackDynamicSize)(
    INOUT   PSTACK          Stack
    )
{
    ASSERT(Stack != NULL);

    return Stack->NumberOfElements;
}

DWORD
StackDynamicGetRequiredSize(
    IN      DWORD                   MaxElements
    )
{
    UNREFERENCED_PARAMETER(MaxElements);

    return sizeof(STACK);
}

STATUS
StackDynamicInit(
    OUT     PSTACK                  Stack
    )
{
    cl_memzero(Stack, sizeof(STACK));

    ClInitializeSListHead(&Stack->TopOfStack);
    Stack->NumberOfElements = 0;

    return STATUS_SUCCESS;
}
