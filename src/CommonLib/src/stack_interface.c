#include "common_lib.h"
#include "stack_interface.h"
#include "stack_internal.h"

#include "stack_dynamic.h"

typedef struct _STACK_COMPLETE_FUNCS
{
    STACK_INTERFACE_FUNCS           PublicFuncs;

    STACK_PRIVATE_INTERFACE_FUNCS   PrivateFuncs;
} STACK_COMPLETE_FUNCS, *PSTACK_COMPLETE_FUNCS;

static const STACK_COMPLETE_FUNCS STACK_FUNCS[StackTypeReserved] =
{
    {// StackTypeDynamic
        StackDynamicPush,
        StackDynamicPop,
        StackDynamicPeek,
        StackDynamicClear,
        StackDynamicIsEmpty,
        StackDynamicSize,
        StackDynamicGetRequiredSize,
        StackDynamicInit
    },

    { NULL, NULL, NULL, NULL, NULL },       // StackTypeInterlocked
};

DWORD
StackGetRequiredSize(
    IN      DWORD                   MaxElements,
    IN      STACK_TYPE              Type
    )
{
    ASSERT(MaxElements > 0);
    ASSERT(Type < StackTypeReserved );

    return STACK_FUNCS[Type].PrivateFuncs.StackGetRequiredSize(MaxElements);
}

STATUS
StackCreate(
    OUT     PSTACK_INTERFACE        StackInterface,
    IN      STACK_TYPE              Type,
    OUT     PSTACK                  Stack
    )
{
    STATUS status;

    if (StackInterface == NULL)
    {
        return STATUS_INVALID_PARAMETER1;
    }

    if (Type >= StackTypeReserved)
    {
        return STATUS_INVALID_PARAMETER2;
    }

    if (Stack == NULL)
    {
        return STATUS_INVALID_PARAMETER3;
    }

    status = STACK_FUNCS[Type].PrivateFuncs.StackInit(Stack);
    if (!SUCCEEDED(status))
    {
        return status;
    }

    StackInterface->Stack = Stack;

    cl_memcpy(&StackInterface->Funcs, (PVOID) &STACK_FUNCS[Type].PublicFuncs, sizeof(STACK_INTERFACE_FUNCS));

    return STATUS_SUCCESS;
}