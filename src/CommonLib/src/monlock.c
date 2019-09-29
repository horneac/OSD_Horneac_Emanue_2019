#include "common_lib.h"
#include "lock_common.h"

#ifndef _COMMONLIB_NO_LOCKS_

void
MonitorLockInit(
    OUT         PMONITOR_LOCK       Lock
    )
{
    ASSERT(NULL != Lock);

    memzero(Lock, sizeof(MONITOR_LOCK));

    _InterlockedExchange8(&Lock->Lock.State, LOCK_FREE);
}

void
MonitorLockAcquire(
    INOUT       PMONITOR_LOCK       Lock,
    OUT         INTR_STATE*         IntrState
    )
{
    PVOID pCurrentCpu;

    ASSERT(NULL != Lock);
    ASSERT(NULL != IntrState);

    *IntrState = CpuIntrDisable();

    pCurrentCpu = CpuGetCurrent();

    ASSERT_INFO(pCurrentCpu != Lock->Lock.Holder,
                "Lock initial taken by function 0x%X, now called by 0x%X\n",
                Lock->Lock.FunctionWhichTookLock,
                *((PVOID*)_AddressOfReturnAddress())
                );

// warning C4127: conditional expression is constant
#pragma warning(suppress:4127)
    while(TRUE)
    {
        _mm_monitor(Lock, 0, 0);

        if (LOCK_FREE == _InterlockedCompareExchange8(&Lock->Lock.State, LOCK_TAKEN, LOCK_FREE))
        {
            break;
        }

        _mm_mwait(0, 0);
    }

    ASSERT(NULL == Lock->Lock.FunctionWhichTookLock);
    ASSERT(NULL == Lock->Lock.Holder);

    Lock->Lock.Holder = pCurrentCpu;
    Lock->Lock.FunctionWhichTookLock = *((PVOID*)_AddressOfReturnAddress());

    ASSERT(LOCK_TAKEN == Lock->Lock.State);
}

BOOL_SUCCESS
BOOLEAN
MonitorLockTryAcquire(
    INOUT       PMONITOR_LOCK       Lock,
    OUT         INTR_STATE*         IntrState
    )
{
    BOOLEAN acquired;

    *IntrState = CpuIntrDisable();

    acquired = (LOCK_FREE == _InterlockedCompareExchange8(&Lock->Lock.State, LOCK_TAKEN, LOCK_FREE));
    if (!acquired)
    {
        CpuIntrSetState(*IntrState);
    }

    return acquired;
}

BOOLEAN
MonitorLockIsOwner(
    IN          PMONITOR_LOCK       Lock
    )
{
    return CpuGetCurrent() == Lock->Lock.Holder;
}

void
MonitorLockRelease(
    INOUT       PMONITOR_LOCK       Lock,
    IN          INTR_STATE          OldIntrState
    )
{
    PVOID pCurrentCpu = CpuGetCurrent();

    ASSERT(NULL != Lock);
    ASSERT(pCurrentCpu == Lock->Lock.Holder);
    ASSERT(INTR_OFF == CpuIntrGetState());

    Lock->Lock.Holder = NULL;
    Lock->Lock.FunctionWhichTookLock = NULL;

    _InterlockedExchange8(&Lock->Lock.State, LOCK_FREE);

    CpuIntrSetState(OldIntrState);
}

#endif // _COMMONLIB_NO_LOCKS_
