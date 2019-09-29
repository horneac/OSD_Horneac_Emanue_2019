#pragma once

#include "register.h"
#include "hw_fpu.h"
#include "cpuid_leaf.h"
#include "cpu_if.h"

typedef
QWORD
(__cdecl FUNC_ReadFsQword)(
    IN              DWORD           Offset
    );

extern FUNC_ReadFsQword                     __HALreadfsqword;

#define NO_OF_TOTAL_INTERRUPTS              256
#define NO_OF_RESERVED_EXCEPTIONS           32

#define NO_OF_USABLE_INTERRUPTS             (NO_OF_TOTAL_INTERRUPTS - NO_OF_RESERVED_EXCEPTIONS)

#pragma pack(push,1)

#pragma warning(push)

// warning C4201: nonstandard extension used : nameless struct/union
#pragma warning(disable:4201)

typedef struct _REGISTER_AREA
{
    QWORD                                           RegisterValues[RegisterR15 + 1];

    QWORD                                           Rip;

    QWORD                                           Rflags;
} REGISTER_AREA, *PREGISTER_AREA;

typedef struct _COMPLETE_PROCESSOR_STATE
{
    XSAVE_AREA                                      XsaveArea;

    REGISTER_AREA                                   RegisterArea;
} COMPLETE_PROCESSOR_STATE, *PCOMPLETE_PROCESSOR_STATE;
#pragma warning(pop)
#pragma pack(pop)

#include "cpu_utils.h"
