#include "common_lib.h"
#include "lock_common.h"

#ifndef _COMMONLIB_NO_LOCKS_

void
RwSpinlockInit(
    OUT     PRW_SPINLOCK    Spinlock
    )
{
    ASSERT( NULL != Spinlock );

    memzero( Spinlock, sizeof(RW_SPINLOCK));
}

REQUIRES_NOT_HELD_LOCK(*Spinlock)
_When_(Exclusive, ACQUIRES_EXCL_AND_NON_REENTRANT_LOCK(*Spinlock))
_When_(!Exclusive, ACQUIRES_SHARED_AND_NON_REENTRANT_LOCK(*Spinlock))
void
RwSpinlockAcquire(
    INOUT   RW_SPINLOCK     *Spinlock,
    OUT     INTR_STATE*     IntrState,
    IN      BOOLEAN         Exclusive
    )
{
    ASSERT( NULL != Spinlock );
    ASSERT( NULL != IntrState );

    *IntrState = CpuIntrDisable();

    if (Exclusive)
    {
        _InterlockedIncrement16(&Spinlock->WaitingWriters);

        // because this is done on DWORD it will affect ActiveWrite and ActiveReaders
        while (0 != _InterlockedCompareExchange((volatile DWORD*) &Spinlock->ActiveWriter, 1, 0))
        {
            _mm_pause();
        }

        // we're here => we're the active writer
        // => we're no longer a waiting writer
        _InterlockedDecrement16(&Spinlock->WaitingWriters);
        _Analysis_assume_lock_acquired_(*Spinlock);
    }
    else
    {
        DWORD pseudoActiveWriter = MAX_WORD + 1;

        // shared

        // pretend to take lock exclusively
        // but instead of checking ActiveWriter and ActiveReaders
        // check WaitingWriters and ActiveWriter (so writers will have priority)
        while (0 != _InterlockedCompareExchange((volatile DWORD*) &Spinlock->WaitingWriters, pseudoActiveWriter, 0))
        {
            _mm_pause();
        }

        // we're here => we're an active reader
        _InterlockedIncrement16(&Spinlock->ActiveReaders);

        ASSERT( 1 == Spinlock->ActiveWriter );

        // let's be honest now and tell the world
        // we're not a writer
        _InterlockedDecrement16(&Spinlock->ActiveWriter);
        _Analysis_assume_lock_acquired_(*Spinlock);
    }
}

_When_(Exclusive, REQUIRES_EXCL_LOCK(*Spinlock) RELEASES_EXCL_AND_NON_REENTRANT_LOCK(*Spinlock))
_When_(!Exclusive, REQUIRES_SHARED_LOCK(*Spinlock) RELEASES_SHARED_AND_NON_REENTRANT_LOCK(*Spinlock))
void
RwSpinlockRelease(
    INOUT   RW_SPINLOCK     *Spinlock,
    IN      INTR_STATE      IntrState,
    IN      BOOLEAN         Exclusive
    )
{
    ASSERT( NULL != Spinlock );

    if (Exclusive)
    {
        ASSERT( 1 == Spinlock->ActiveWriter );

        _InterlockedDecrement16(&Spinlock->ActiveWriter);
        _Analysis_assume_lock_released_(*Spinlock);
    }
    else
    {
        // shared

        // in this case may have 1 pseudo-active writer as well until
        // the reader 'confesses' to the world it is not a writer
        _InterlockedDecrement16(&Spinlock->ActiveReaders);
        _Analysis_assume_lock_released_(*Spinlock);
    }

    CpuIntrSetState(IntrState);
}

#endif // _COMMONLIB_NO_LOCKS_
