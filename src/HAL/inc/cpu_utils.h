#pragma once

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
    return IsBooleanFlagOn(__readeflags(), RFLAGS_INTERRUPT_FLAG_BIT);
}

__forceinline
extern
INTR_STATE
CpuIntrSetState(
    const      INTR_STATE         IntrState
    )
{
    QWORD rFlags = __readeflags();
    QWORD newFlags = IntrState ? ( rFlags | RFLAGS_INTERRUPT_FLAG_BIT ) : ( rFlags & ( ~RFLAGS_INTERRUPT_FLAG_BIT));

    __writeeflags(newFlags);

    return IsBooleanFlagOn(rFlags, RFLAGS_INTERRUPT_FLAG_BIT);
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

typedef BYTE APIC_ID;

__forceinline
extern
APIC_ID
CpuGetApicId(
    void
    )
{
    CPUID_INFO cpuId;

    __cpuid(cpuId.values, CpuidIdxFeatureInformation);

    return cpuId.FeatureInformation.ebx.ApicId;
}

__forceinline
extern
BOOLEAN
CpuIsIntel(
    void
    )
{
    CPUID_INFO cpuId;

    __cpuid(cpuId.values, CpuidIdxBasicInformation);

    return ( cpuId.ebx == 'uneG' &&
             cpuId.edx == 'Ieni' &&
             cpuId.ecx == 'letn' );
}
