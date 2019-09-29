#pragma once

C_HEADER_START

#include <intrin.h>

#define RFLAGS_DIRECTION_BIT                        ((QWORD)1<<10)

__forceinline
extern
void
CpuClearDirectionFlag(
    void
)
{
    __writeeflags(__readeflags() & (~RFLAGS_DIRECTION_BIT));
}

__forceinline
extern
INTR_STATE
CpuIntrGetState(
    void
)
{
    return INTR_OFF;
}

__forceinline
extern
INTR_STATE
CpuIntrSetState(
    const      INTR_STATE         IntrState
)
{
    UNREFERENCED_PARAMETER(IntrState);

    return INTR_OFF;
}

__forceinline
extern
INTR_STATE
CpuIntrDisable(
    void
)
{
    return CpuIntrSetState(FALSE);
}

__forceinline
extern
INTR_STATE
CpuIntrEnable(
    void
)
{
    return CpuIntrSetState(TRUE);
}

#pragma warning(push)
// warning C4201: nonstandard extension used: nameless struct/union
#pragma warning(disable:4201)

// structure retrieved by __cpuid operations
typedef struct _CPUID_INFO
{
    union
    {
        int values[4];
        struct
        {
            DWORD eax;
            DWORD ebx;
            DWORD ecx;
            DWORD edx;
        };
    };
} CPUID_INFO, *PCPUID_INFO;
STATIC_ASSERT(sizeof(CPUID_INFO) == sizeof(DWORD) * 4);
#pragma warning(pop)


__forceinline
extern
BYTE
CpuGetApicId(
    void
)
{
    CPUID_INFO cpuId;

    __cpuid(cpuId.values, 1);

    return ( cpuId.ebx >> 24 ) & MAX_BYTE;
}

#define CURRENT_CPU_MASK        0x8000'0000'0000'0000ULL

__forceinline
extern
PVOID
CpuGetCurrent(void)
{
    // warning C4306: 'type cast': conversion from 'BYTE' to 'PVOID' of greater size
#pragma warning(suppress:4306)
    return (PVOID)(CURRENT_CPU_MASK | CpuGetApicId());
}

FUNC_AssertFunction                 UtCommonLibAssert;

void
(__cdecl UtCommonLibAssert)(
    IN_Z            char*       Message
    )
{
    printf("Assert reached\n");
    printf("Message is %s\n", Message );

    __debugbreak();
}

C_HEADER_END