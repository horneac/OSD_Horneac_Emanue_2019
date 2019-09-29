#pragma once

void
RtcInit(
    OUT_OPT     QWORD*          TscFrequency
    );

void
RtcAcknowledgeTimerInterrupt(
    void
    );

__forceinline
QWORD
RtcGetTickCount(
    void
    )
{
    _mm_lfence();
    return __rdtsc();
}