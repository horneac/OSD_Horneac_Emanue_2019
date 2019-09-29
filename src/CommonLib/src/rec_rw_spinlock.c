#include "common_lib.h"
#include "rec_rw_spinlock.h"
#include "lock_common.h"

#ifndef _COMMONLIB_NO_LOCKS_

#define RW_DEFAULT_RECURSIVITY_DEPTH_MAX                2

void
RecRwSpinlockInit(
    IN      BYTE                    RecursivityDepth,
    OUT     PREC_RW_SPINLOCK        Spinlock
    )
{
    BYTE recursivityDepth;

    ASSERT(NULL != Spinlock);

    memzero(Spinlock, sizeof(REC_RW_SPINLOCK));

    recursivityDepth = (0 == RecursivityDepth) ? RW_DEFAULT_RECURSIVITY_DEPTH_MAX : RecursivityDepth;

    RwSpinlockInit(&Spinlock->RwSpinlock);

    Spinlock->MaxRecursivityDepth = recursivityDepth;
}

// warning C26165 : Possibly failing to release lock '* Spinlock' in function 'RecRwSpinlockAcquire'.
// well we're not actually attempting to release a lock in this function...
#pragma warning(push)
#pragma warning(disable:26165)

REQUIRES_NOT_HELD_LOCK(*Spinlock)
_When_(Exclusive, ACQUIRES_EXCL_AND_REENTRANT_LOCK(*Spinlock))
_When_(!Exclusive, ACQUIRES_SHARED_AND_NON_REENTRANT_LOCK(*Spinlock))
void
RecRwSpinlockAcquire(
    INOUT   REC_RW_SPINLOCK     *Spinlock,
    OUT     INTR_STATE*         IntrState,
    IN      BOOLEAN             Exclusive
    )
{
    PVOID pCurrentCpu;
    INTR_STATE dummy;

    ASSERT(NULL != Spinlock);
    ASSERT(NULL != IntrState);

    *IntrState = CpuIntrDisable();
    pCurrentCpu = CpuGetCurrent();

    if (Exclusive)
    {
        if (Spinlock->Holder == pCurrentCpu)
        {
            ASSERT_INFO( Spinlock->CurrentRecursivityDepth < Spinlock->MaxRecursivityDepth,
                        "Current recursivity depth: %u\nMax recursivity depth: %u\n",
                        Spinlock->CurrentRecursivityDepth,
                        Spinlock->MaxRecursivityDepth
                        );

            _Analysis_assume_lock_acquired_(*Spinlock);
            Spinlock->CurrentRecursivityDepth++;
            return;
        }
    }

    // if we get here => we need to call RwSpinlockAcquire
    RwSpinlockAcquire(&Spinlock->RwSpinlock, &dummy, Exclusive );

    ASSERT( NULL == Spinlock->Holder );
    ASSERT( 0 == Spinlock->CurrentRecursivityDepth );

    if (Exclusive)
    {
        Spinlock->CurrentRecursivityDepth = 1;
        Spinlock->Holder = CpuGetCurrent();
    }
}
#pragma warning(pop)

// warning C26167: Possibly releasing unheld lock '* Spinlock' in function 'RecRwSpinlockRelease'.
// FALSE, even if we tell SAL we used reentrant locks it's still stupid, and wants a release on
// each call, so we need to tell him we release the lock in the first branch, and as a result
// there's a situation in which he thinks we release the lock twice, one on the first branch
// and second on the RwSpinlockRelease
#pragma warning(push)
#pragma warning(disable:26167)

_When_(Exclusive, REQUIRES_EXCL_LOCK(*Spinlock) RELEASES_EXCL_AND_REENTRANT_LOCK(*Spinlock))
_When_(!Exclusive, REQUIRES_SHARED_LOCK(*Spinlock) RELEASES_SHARED_AND_NON_REENTRANT_LOCK(*Spinlock))
void
RecRwSpinlockRelease(
    INOUT   REC_RW_SPINLOCK     *Spinlock,
    IN      INTR_STATE          IntrState,
    IN      BOOLEAN             Exclusive
    )
{
    PVOID pCurrentCpu;

    ASSERT( NULL != Spinlock );
    ASSERT(INTR_OFF == CpuIntrGetState());

    pCurrentCpu = CpuGetCurrent();

    if (Exclusive)
    {
        ASSERT(pCurrentCpu == Spinlock->Holder);

        // if we're past the previous assert => lock is held by this CPU, SAL doesn't realize it, so assume it
        _Analysis_assume_lock_held_(*Spinlock);
        Spinlock->CurrentRecursivityDepth--;
        
        _Analysis_assume_lock_released_(*Spinlock);
    }

    if (0 == Spinlock->CurrentRecursivityDepth || !Exclusive)
    {
        Spinlock->Holder = NULL;

        _Analysis_assume_lock_held_(*Spinlock);
        RwSpinlockRelease(&Spinlock->RwSpinlock, IntrState, Exclusive );
    }
}

#pragma warning(pop)

#endif // _COMMONLIB_NO_LOCKS_
