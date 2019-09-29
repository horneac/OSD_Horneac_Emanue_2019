#include "hal_base.h"
#include "hw_fpu.h"
#include "cpu.h"

// The pragma optimize off is required for all the functions which are called from
// HalActivateFpu - this is because we don't want the compiler to generate any SSE
// instructions before we actually enabled the FP unit :)
#pragma optimize( "", off )
__forceinline
static
BOOLEAN
_HalCheckBasicFpuFeatures(
    void
    )
{
    CPUID_INFO cpuidFeatInfo;

    __cpuid(cpuidFeatInfo.values, CpuidIdxFeatureInformation);

    // Check for floating point unit on chip
    // Check for FXSAVE/FXRSTOR support
    // Check for SSE/SSE2 support
    // Check for XSAVE/XRSTOR support
    return
        (
         cpuidFeatInfo.FeatureInformation.edx.FPU   &&
         cpuidFeatInfo.FeatureInformation.edx.FXSR  &&
         cpuidFeatInfo.FeatureInformation.edx.SSE   &&
         cpuidFeatInfo.FeatureInformation.edx.SSE2  &&
         cpuidFeatInfo.FeatureInformation.ecx.XSAVE
            );
}

__forceinline
static
void
_HalEnableFpu(
    void
    )
{
    // make sure emulation is disabled in CR0
    __writecr0(__readcr0() &~(CR0_EM));

    // Enable
    //  ->  FXSAVE and FXRSTOR instructions (needed - I have no idea why exactly)
    //  ->  Operating System Support for Unmasked SIMD Floating-Point Exceptions
    //  ->  XSAVE and Processor Extended States-Enable Bit
    __writecr4(
        __readcr4() |
        (CR4_OSFXSR | CR4_OSXMMEXCPT | CR4_OSXSAVE)
    );
}

void
HalActivateFpu(
    void
    )
{
#if INCLUDE_FP_SUPPORT
    // We have nothing here, no commonlib, no runtime support if something
    // goes wrong => we need to halt
    if (!_HalCheckBasicFpuFeatures()) __halt();

    _HalEnableFpu();
#endif
}
#pragma optimize( "", on )

__forceinline
static
BOOLEAN
_HalCheckRequestedFpuFeatures(
    _In_        XCR0_SAVED_STATE            RequestedFeatures
    )
{
    CPUID_INFO cpuidFeatInfo;
    XCR0_SAVED_STATE availableFeatures;

    // x87 FPU/MMX state must be 1!
    if (!IsBooleanFlagOn(RequestedFeatures, XCR0_SAVED_STATE_x87_MMX)) return FALSE;

    __cpuidex(cpuidFeatInfo.values, CpuidIdxExtendedStateEnumerationMainLeaf, 0x0);

    availableFeatures = DWORDS_TO_QWORD(
        cpuidFeatInfo.ExtendedStateMainLeaf.Xcr0FeatureSupportHigh,
        cpuidFeatInfo.ExtendedStateMainLeaf.Xcr0FeatureSupportLow);

    return IsBooleanFlagOn(
        availableFeatures,
        RequestedFeatures
    );
}

__forceinline
static
void
_HalSetRequestedFpuFeatures(
    _In_        XCR0_SAVED_STATE            Features
    )
{
    _xsetbv(XCR0_INDEX, Features);
}

__forceinline
static
BOOLEAN
_HalCheckEnabledFeaturesSaveSize(
    _In_        DWORD                       MaxSizeSupported
    )
{
    CPUID_INFO cpuidFeatInfo;

    __cpuidex(cpuidFeatInfo.values, CpuidIdxExtendedStateEnumerationMainLeaf, 0x0);

    return (cpuidFeatInfo.ExtendedStateMainLeaf.MaxSizeRequiredByFeaturesInXcr0 <= MaxSizeSupported);
}

STATUS
HalSetActiveFpuFeatures(
    _In_        XCR0_SAVED_STATE            Features
    )
{
#if INCLUDE_FP_SUPPORT
    if (!_HalCheckRequestedFpuFeatures(Features))
    {
        return STATUS_CPU_UNSUPPORTED_FEATURE;
    }

    _HalSetRequestedFpuFeatures(Features);

    if (!_HalCheckEnabledFeaturesSaveSize(HAL_MAX_SUPPORTED_XSAVE_AREA_SIZE))
    {
        return STATUS_CPU_UNSUPPORED_XSAVE_FEATURE_SIZE;
    }
#else
    UNREFERENCED_PARAMETER(Features);
#endif // INCLUDE_FP_SUPPORT

    return STATUS_SUCCESS;
}

XCR0_SAVED_STATE
HalGetActiveFpuFeatures(
    _Out_opt_   DWORD*                      ActivatedFeaturesSaveSize,
    _Out_opt_   DWORD*                      AvailableFeaturesSaveSize
    )
{
#if INCLUDE_FP_SUPPORT
    if (ActivatedFeaturesSaveSize != NULL || AvailableFeaturesSaveSize != NULL)
    {
        CPUID_INFO cpuidFeatInfo;

        __cpuidex(cpuidFeatInfo.values, CpuidIdxExtendedStateEnumerationMainLeaf, 0x0);

        if (ActivatedFeaturesSaveSize != NULL)
        {
            *ActivatedFeaturesSaveSize = cpuidFeatInfo.ExtendedStateMainLeaf.MaxSizeRequiredByFeaturesInXcr0;
        }

        if (AvailableFeaturesSaveSize != NULL)
        {
            *AvailableFeaturesSaveSize = cpuidFeatInfo.ExtendedStateMainLeaf.MaxSizeRequiredByFeaturesSupportedByCpu;
        }
    }

    return _xgetbv(XCR0_INDEX);
#else
    if (ActivatedFeaturesSaveSize != NULL) *ActivatedFeaturesSaveSize = 0;
    if (AvailableFeaturesSaveSize != NULL) *AvailableFeaturesSaveSize = 0;

    return MAX_QWORD;
#endif // INCLUDE_FP_SUPPORT
}
