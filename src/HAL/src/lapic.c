#include "hal_base.h"
#include "lapic.h"
#include "pit.h"
#include "msr.h"
#include "lapic_registers.h"

__forceinline
static
BOOLEAN
_LapicIsCpuBsp(
    void
    )
{
    return IsBooleanFlagOn( __readmsr(IA32_APIC_BASE_MSR), IA32_APIC_BSP_FLAG );
}

static
void
_LapicConfigureLocalInts(
    IN      PLAPIC                          Apic
    );

static
void
_LapicConfigureErrorRegister(
    IN      PLAPIC                          Apic,
    IN      BYTE                            ErrorInterruptVector
    );

void
LapicInitialize(
    IN      PVOID                           ApicBaseAddress
    )
{
    QWORD apicMsr;
    PLAPIC pLapic;

    pLapic = ApicBaseAddress;

    ASSERT(NULL != pLapic);

    // make sure APIC is enabled and
    // not XAPIC
    apicMsr = __readmsr(IA32_APIC_BASE_MSR);

    apicMsr |= IA32_APIC_BASE_ENABLE_FLAG;
    apicMsr &= (~IA32_APIC_EXT_ENABLE_FLAG);

    __writemsr(IA32_APIC_BASE_MSR, apicMsr);
}

void
LapicSetLogicalApicId(
    IN      PVOID                           ApicBaseAddress,
    IN      _Strict_type_match_
            APIC_ID                         LogicalApicId,
    IN      BYTE                            DestinationFormat
    )
{
    PLAPIC pLapic;
    LDR_REGISTER ldrRegister;
    DFR_REGISTER dfrRegister;

    pLapic = (PLAPIC)ApicBaseAddress;

    ASSERT (NULL != pLapic);

    memzero(&ldrRegister, sizeof(LDR_REGISTER));

    dfrRegister.Raw = MAX_DWORD;
    dfrRegister.Model = DestinationFormat;

    pLapic->DestinationFormat.Value = dfrRegister.Raw;

    ldrRegister.LogicalApicId = LogicalApicId;

    pLapic->LogicalDestination.Value = ldrRegister.Raw;
}

void
LapicSetState(
    IN      PVOID                           ApicBaseAddress,
    IN      BYTE                            SpuriousInterruptVector,
    IN      BOOLEAN                         Enable
    )
{
    SVR_REGISTER svrValue;
    PLAPIC pLapic;

    pLapic = (PLAPIC) ApicBaseAddress;

    ASSERT (NULL != pLapic);

    memzero(&svrValue, sizeof(SVR_REGISTER));

    svrValue.Vector = SpuriousInterruptVector;
    svrValue.ApicEnable = Enable;
    pLapic->SpuriousInterruptVector.Value = svrValue.Raw;
}

BOOLEAN
LapicGetState(
    IN      PVOID                           ApicBaseAddress
    )
{
    SVR_REGISTER svrValue;
    PLAPIC pLapic;

    pLapic = (PLAPIC)ApicBaseAddress;

    ASSERT(NULL != pLapic);

    svrValue.Raw = pLapic->SpuriousInterruptVector.Value;

    return (BOOLEAN) svrValue.ApicEnable;
}

void
LapicSendEOI(
    IN      PVOID                           ApicBaseAddress,
    IN      BYTE                            Vector
    )
{
    PLAPIC pLapic;

    pLapic = (PLAPIC)ApicBaseAddress;

    ASSERT(NULL != pLapic);

    pLapic->EOI.Value = Vector;
}

void
LapicSendIpi(
    IN      PVOID                           ApicBaseAddress,
    IN      _Strict_type_match_
            APIC_ID                         ApicId,
    IN      _Strict_type_match_
            APIC_DELIVERY_MODE              DeliveryMode,
    IN      _Strict_type_match_
            APIC_DESTINATION_SHORTHAND      DestinationShorthand,
    IN      _Strict_type_match_
            APIC_DESTINATION_MODE           DestinationMode,
    IN_OPT  BYTE*                           Vector
    )
{
    ICR_LOW_REGISTER lowIcrValue = { 0 };
    ICR_HIGH_REGISTER highIcrValue = { 0 };
    PLAPIC pLapic;

    pLapic = (PLAPIC)ApicBaseAddress;

    ASSERT(NULL != pLapic);

    if ((ApicDestinationShorthandSelf == DestinationShorthand) || (ApicDestinationShorthandAll == DestinationShorthand))
    {
        // in this case only FIXED IPI can be sent
        ASSERT(ApicDeliveryModeFixed == DeliveryMode);
    }

    highIcrValue.Destination = ApicId;

    if (NULL != Vector)
    {
        lowIcrValue.Vector = *Vector;
    }

    lowIcrValue.DeliveryMode = DeliveryMode;

    // ASSERT
    lowIcrValue.Level = 1;

    lowIcrValue.DestinationMode = DestinationMode;
    lowIcrValue.DestinationShorthand = DestinationShorthand;

    pLapic->IcrHigh.Value = highIcrValue.Raw;
    pLapic->IcrLow.Value = lowIcrValue.Raw;
}

BYTE
LapicGetPpr(
    IN      PVOID                           ApicBaseAddress
    )
{
    PLAPIC pLapic;

    pLapic = (PLAPIC)ApicBaseAddress;

    ASSERT(NULL != pLapic);

    return (BYTE) ( pLapic->PPR.Value >> 4 );
}

DWORD
LapicGetTimerCount(
    IN      PVOID                           ApicBaseAddress
    )
{
    PLAPIC pLapic;

    pLapic = (PLAPIC)ApicBaseAddress;

    ASSERT(NULL != pLapic);

    return pLapic->TimerCurrentCount.Value;
}

void
LapicConfigureTimer(
    IN      PVOID                           ApicBaseAddress,
    IN      BYTE                            TimerInterruptVector,
    IN      _Strict_type_match_
            APIC_DIVIDE_VALUE               DivideValue
    )
{
    LVT_REGISTER timerRegister;
    PLAPIC pLapic;

    pLapic = (PLAPIC)ApicBaseAddress;

    ASSERT(NULL != pLapic);
    ASSERT( ApicDivideReserved != DivideValue );

    memzero(&timerRegister, sizeof(LVT_REGISTER));

    timerRegister.Vector = TimerInterruptVector;

    // set initial count to 0 (disable timer)
    pLapic->TimerInitialCount.Value = 0;

    // set divide value
    pLapic->TimerDivideConfiguration.Value = DivideValue;

    // un-mask timer interrupts
    timerRegister.TimerMode = APIC_TIMER_PERIOD_MODE;
    timerRegister.Masked = FALSE;

    pLapic->LvtTimer.Value = timerRegister.Raw;
}

void
LapicSetTimerInterval(
    IN      PVOID                           ApicBaseAddress,
    IN      DWORD                           TimerCount
    )
{
    PLAPIC pLapic;

    pLapic = (PLAPIC)ApicBaseAddress;

    ASSERT(NULL != pLapic);

    pLapic->TimerInitialCount.Value = TimerCount;
}

void
LapicConfigureLvtRegisters(
    IN      PVOID                           ApicBaseAddress,
    IN      BYTE                            ErrorInterruptVector
    )
{
    PLAPIC pLapic;

    pLapic = (PLAPIC)ApicBaseAddress;

    ASSERT(NULL != pLapic);

    // configure LINT vectors
    _LapicConfigureLocalInts(pLapic);

    // configure error register
    _LapicConfigureErrorRegister(pLapic, ErrorInterruptVector);

    // configure future LVT registers
    // ...
}

PHYSICAL_ADDRESS
LapicGetBasePhysicalAddress(
    void
    )
{
    QWORD apicBaseRegister;

    apicBaseRegister = __readmsr(IA32_APIC_BASE_MSR);

    return (PHYSICAL_ADDRESS)IA32_APIC_BASE_MASK(apicBaseRegister);
}

void
LapicDetermineDividedBusFrequency(
    IN      PVOID                           ApicBaseAddress,
    OUT     DWORD*                          BusFrequency
    )
{
    LVT_REGISTER timerRegister;
    PLAPIC pLapic;
    DWORD cpuBusFrequency;
    DWORD currentCount;

    pLapic = (PLAPIC)ApicBaseAddress;

    ASSERT(NULL != pLapic);
    ASSERT(NULL != BusFrequency);

    memzero(&timerRegister, sizeof(LVT_REGISTER));
    currentCount = 0;

    // divisor rate = 1
    pLapic->TimerDivideConfiguration.Value = ApicDivideBy1;

    timerRegister.TimerMode = APIC_TIMER_ONE_SHOT_MODE; // one shot-mode
    timerRegister.Masked = 1;

    pLapic->LvtTimer.Value = timerRegister.Raw;

    PitSetTimer(APIC_TIMER_CONFIGURATION_SLEEP, FALSE);

    PitStartTimer();

    pLapic->TimerInitialCount.Value = MAX_DWORD;

    PitWaitTimer();

    currentCount = pLapic->TimerCurrentCount.Value;

    cpuBusFrequency = ((MAX_DWORD - currentCount) + 1) * 1 * 100;

    // set initial count to 0 (disable timer)
    pLapic->TimerInitialCount.Value = 0;

    *BusFrequency = cpuBusFrequency;
}

DWORD
LapicGetErrorRegister(
    IN      PVOID                           ApicBaseAddress
    )
{
    PLAPIC pLapic;

    pLapic = (PLAPIC)ApicBaseAddress;

    ASSERT(NULL != pLapic);

    // 10.5.3
    // The ESR is a read write  register. Before attempt to read from
    // the ESR, software should first write it. The value written does not
    // affect the values read subsequently
    pLapic->ErrorStatus.Value = 0;

    return pLapic->ErrorStatus.Value;
}

BOOLEAN
LapicIsInterruptServiced(
    IN      PVOID                           ApicBaseAddress,
    IN      BYTE                            Vector
    )
{
    PLAPIC pLapic;
    DWORD isrValue;

    pLapic = (PLAPIC)ApicBaseAddress;

    ASSERT( NULL != pLapic );
    ASSERT( Vector >= APIC_FIRST_USABLE_INTERRUPT_INDEX );

    isrValue = pLapic->ISR[Vector / APIC_VECTORS_PER_ISR_REGISTER].Value;

    return ( isrValue & ( 1 << (Vector % APIC_VECTORS_PER_ISR_REGISTER))) != 0;
}

static
void
_LapicConfigureLocalInts(
    IN      PLAPIC                      Apic
    )
{
    LVT_REGISTER lvtTemp;

    ASSERT(NULL != Apic);

    // disable LINT0
    // This is required to exit virtual-wire mode through LAPIC and enter symmetric mode
    // i.e. the PIC will no longer be used for interrupt delivery
    lvtTemp.Raw = 0;
    lvtTemp.Masked = TRUE;

    Apic->LvtLINT0.Value = lvtTemp.Raw;

    // enable LINT1 (NMI)
    lvtTemp.Raw = 0;

    lvtTemp.Masked = FALSE;
    lvtTemp.DeliveryMode = ApicDeliveryModeNMI;

    Apic->LvtLINT1.Value = lvtTemp.Raw;
}

static
void
_LapicConfigureErrorRegister(
    IN      PLAPIC                          Apic,
    IN      BYTE                            ErrorInterruptVector
    )
{
    LVT_REGISTER lvt;
    DWORD error;

    ASSERT( NULL != Apic );

    // set Error LVT
    lvt.Raw = 0;
    lvt.Masked = FALSE;
    lvt.Vector = ErrorInterruptVector;

    Apic->LvtError.Value = lvt.Raw;

    Apic->ErrorStatus.Value = 0;

    /// we need to write 0 TWICE in the register to properly record any
    /// future errors
    // Assumption: first zero write causes all APIC errors which occurred
    // before our configuration to be recorded => we need to write another 0
    // to only hold the errors we will cause
    error = LapicGetErrorRegister(Apic);

    // make sure no errors occurred
    ASSERT_INFO(0 == error,
                "Lapic ERROR status: 0x%x\n",
                error
                );
}