#include "hal_base.h"
#include "rtc.h"
#include "cmos.h"
#include "cpu.h"
#include "pit.h"

#define RTC_LOWEST_RATE                     3
#define RTC_HIGHEST_RATE                    15

#define RDTSC_TIMER_CONFIGURATION_SLEEP     1*MS_IN_US
#define RDTSC_TIMER_CONFIGURATION_SAMPLES   4

static QWORD                            m_tscFrequency;

static
void
_RtcInitializeTimerInterrupt(
    void
    );

static
QWORD
_RtcDetermineTscFrequency(
    IN          DWORD                   NoOfSamples,
    IN          DWORD                   SleepUsPerSample
    );

void
RtcInit(
    OUT_OPT     QWORD*          TscFrequency
    )
{
    m_tscFrequency = _RtcDetermineTscFrequency(RDTSC_TIMER_CONFIGURATION_SAMPLES,
                                               RDTSC_TIMER_CONFIGURATION_SLEEP
                                               );

    _RtcInitializeTimerInterrupt();
    
    if (NULL != TscFrequency)
    {
        *TscFrequency = m_tscFrequency;
    }
}

void
RtcAcknowledgeTimerInterrupt(
    void
    )
{
    // just throw away contents
    CmosGetValueNMI(CmosRegisterStatusC);
}

static
void
_RtcInitializeTimerInterrupt(
    void
    )
{
    INTR_STATE prevIntrState;
    BYTE prevValue;

    prevValue = 0;

    prevIntrState = CpuIntrDisable();

    // read the current value of register B
    prevValue = CmosGetValueDisableNMI(CmosRegisterStatusB);

    // write the previous value ORed with 0x10. This turns on update interrupts
    CmosWriteValueDisableNMI(CmosRegisterStatusB, prevValue | CMOS_UPDATE_INTERRUPT);

    RtcAcknowledgeTimerInterrupt();

    // restore interrupt state
    CpuIntrSetState(prevIntrState);
}

static
QWORD
_RtcDetermineTscFrequency(
    IN          DWORD                   NoOfSamples,
    IN          DWORD                   SleepUsPerSample
    )
{
    QWORD initialRdtsc;
    QWORD finalRdtsc;
    QWORD ticksElapsed;
    QWORD tickFrequency;
    DWORD i;
    QWORD minTick = MAX_QWORD;
    QWORD maxTick = 0;
    QWORD total = 0;
    QWORD tickMean;

    for (i = 0; i < NoOfSamples; ++i)
    {
        initialRdtsc = RtcGetTickCount(); 
        PitSetTimer(SleepUsPerSample, FALSE);

        PitStartTimer();

        PitWaitTimer();
        finalRdtsc = RtcGetTickCount();

        ticksElapsed = finalRdtsc - initialRdtsc;

        if (ticksElapsed < minTick)
        {
            minTick = ticksElapsed;
        }

        if (ticksElapsed > maxTick)
        {
            maxTick = ticksElapsed;
        }
        total = total + ticksElapsed;
    }

    tickMean = total / NoOfSamples;
    tickFrequency = tickMean * ( ( 1 * SEC_IN_US ) / SleepUsPerSample );

    return tickFrequency;
}