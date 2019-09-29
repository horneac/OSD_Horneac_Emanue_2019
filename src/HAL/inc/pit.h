#pragma once

WORD
PitSetTimer(
    IN      DWORD       Microseconds,
    IN      BOOLEAN     Periodic
    );

void
PitStartTimer(
    void
    );

void
PitWaitTimer(
    void
    );

void
PitSleep(
    IN      DWORD       Microseconds
    );

__declspec(deprecated)
WORD
PitGetTimerCount(
    IN      BOOLEAN     Periodic
    );

