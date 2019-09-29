#pragma once

C_HEADER_START
#include "slist.h"
#include "ref_cnt.h"

typedef struct _STACK* PSTACK;

typedef struct _STACK_ITEM
{
    CL_SLIST_ENTRY     Next;
} STACK_ITEM, *PSTACK_ITEM;

typedef
BOOLEAN
(__cdecl FUNC_StackPush)(
    INOUT   PSTACK          Stack,
    IN      PSTACK_ITEM     Item
    );

typedef FUNC_StackPush*         PFUNC_StackPush;

typedef
_Must_inspect_result_
_Success_(return != NULL)
STACK_ITEM*
(__cdecl FUNC_StackPop)(
    INOUT   PSTACK          Stack
    );

typedef FUNC_StackPop*          PFUNC_StackPop;

typedef
_Must_inspect_result_
_Success_(return != NULL)
STACK_ITEM*
(__cdecl FUNC_StackPeek)(
    IN      PSTACK          Stack,
    IN      DWORD           Index
    );

typedef FUNC_StackPeek*         PFUNC_StackPeek;

typedef
void
(__cdecl FUNC_StackClear)(
    INOUT   PSTACK              Stack,
    IN_OPT  PFUNC_FreeFunction  FreeFunction,
    IN_OPT  PVOID               FreeContext
    );

typedef FUNC_StackClear*        PFUNC_StackClear;

typedef
BOOLEAN
(__cdecl FUNC_StackIsEmpty)(
        INOUT   PSTACK          Stack
        );

typedef FUNC_StackIsEmpty*      PFUNC_StackIsEmpty;

typedef
DWORD
(__cdecl FUNC_StackSize)(
    INOUT   PSTACK          Stack
    );

typedef FUNC_StackSize*         PFUNC_StackSize;


typedef struct _STACK_INTERFACE_FUNCS
{
    PFUNC_StackPush                 Push;
    PFUNC_StackPop                  Pop;
    PFUNC_StackPeek                 Peek;
    PFUNC_StackClear                Clear;

    PFUNC_StackIsEmpty              IsEmpty;
    PFUNC_StackSize                 Size;
} STACK_INTERFACE_FUNCS, *PSTACK_INTERFACE_FUNCS;

typedef struct _STACK_INTERFACE
{
    STACK_INTERFACE_FUNCS           Funcs;

    PSTACK                          Stack;
    DWORD                           MaxElements;
} STACK_INTERFACE, *PSTACK_INTERFACE;

typedef enum _STACK_TYPE
{
    StackTypeDynamic,
    StackTypeInterlocked,

    StackTypeReserved = StackTypeInterlocked + 1
} STACK_TYPE;

DWORD
StackGetRequiredSize(
    IN      DWORD                   MaxElements,
    IN      STACK_TYPE              Type
    );

STATUS
StackCreate(
    OUT     PSTACK_INTERFACE        StackInterface,
    IN      STACK_TYPE              Type,
    OUT     PSTACK                  Stack
    );
C_HEADER_END
