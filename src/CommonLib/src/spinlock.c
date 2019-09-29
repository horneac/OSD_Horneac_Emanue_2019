#include "common_lib.h"
#include "lock_common.h"

#ifndef _COMMONLIB_NO_LOCKS_

void
SpinlockInit(
    OUT         PSPINLOCK       Lock
    )
{
    ASSERT(NULL != Lock);

    memzero(Lock, sizeof(SPINLOCK));

    _InterlockedExchange8(&Lock->State, LOCK_FREE);
}

void
SpinlockAcquire(
    INOUT       PSPINLOCK       Lock,
    OUT         INTR_STATE*     IntrState
    )
{
    PVOID pCurrentCpu;

    ASSERT(NULL != Lock);
    ASSERT(NULL != IntrState);

    *IntrState = CpuIntrDisable();

    pCurrentCpu = CpuGetCurrent();

    ASSERT_INFO(pCurrentCpu != Lock->Holder,
                "Lock initial taken by function 0x%X, now called by 0x%X\n",
                Lock->FunctionWhichTookLock,
                *((PVOID*)_AddressOfReturnAddress())
                );

    while (LOCK_TAKEN == _InterlockedCompareExchange8(&Lock->State, LOCK_TAKEN, LOCK_FREE))
    {
        _mm_pause();
    }

    ASSERT(NULL == Lock->FunctionWhichTookLock);
    ASSERT(NULL == Lock->Holder);

    Lock->Holder = pCurrentCpu;
    Lock->FunctionWhichTookLock = *( (PVOID*) _AddressOfReturnAddress() );

    ASSERT(LOCK_TAKEN == Lock->State);
}

BOOL_SUCCESS
BOOLEAN
SpinlockTryAcquire(
    INOUT       PSPINLOCK       Lock,
    OUT         INTR_STATE*     IntrState
    )
{
    BOOLEAN acquired;

    *IntrState = CpuIntrDisable();

    acquired = (LOCK_FREE == _InterlockedCompareExchange8(&Lock->State, LOCK_TAKEN, LOCK_FREE));
    if (!acquired)
    {
        CpuIntrSetState(*IntrState);
    }

    return acquired;
}

BOOLEAN
SpinlockIsOwner(
    IN          PSPINLOCK       Lock
    )
{
    return CpuGetCurrent() == Lock->Holder;
}

void
SpinlockRelease(
    INOUT       PSPINLOCK       Lock,
    IN          INTR_STATE      OldIntrState
    )
{
    PVOID pCurrentCpu = CpuGetCurrent();

    ASSERT(NULL != Lock);
    ASSERT_INFO(pCurrentCpu == Lock->Holder,
                "LockTaken by CPU: 0x%X in function: 0x%X\nNow release by CPU: 0x%X in function: 0x%X\n",
                Lock->Holder, Lock->FunctionWhichTookLock,
                pCurrentCpu, *( (PVOID*) _AddressOfReturnAddress() ) );
    ASSERT(INTR_OFF == CpuIntrGetState());

    Lock->Holder = NULL;
    Lock->FunctionWhichTookLock = NULL;

    _InterlockedExchange8(&Lock->State, LOCK_FREE);

    CpuIntrSetState(OldIntrState);
}

#endif // _COMMONLIB_NO_LOCKS_
