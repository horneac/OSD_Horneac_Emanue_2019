#pragma once

C_HEADER_START
typedef struct _STACK* PSTACK;

typedef
DWORD
FUNC_StackGetRequiredSize(
    IN      DWORD                   MaxElements
    );

typedef FUNC_StackGetRequiredSize* PFUNC_StackGetRequiredSize;

typedef
STATUS
FUNC_StackInit(
    OUT     PSTACK                  Stack
    );

typedef FUNC_StackInit*             PFUNC_StackInit;

typedef struct _STACK_PRIVATE_INTERFACE_FUNCS
{
    PFUNC_StackGetRequiredSize      StackGetRequiredSize;

    PFUNC_StackInit                 StackInit;
} STACK_PRIVATE_INTERFACE_FUNCS, *PSTACK_PRIVATE_INTERFACE_FUNCS;
C_HEADER_END
