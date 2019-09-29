#pragma once

C_HEADER_START
#include "rw_spinlock.h"

#pragma pack(push,16)
typedef struct _REC_RW_SPINLOCK
{
    RW_SPINLOCK     RwSpinlock;

    // we don't use BYTE because we want to conserve memory,
    // we use BYTE because there is NO REASON why a lock
    // should be acquired more than 255 times by the same thread
    BYTE            CurrentRecursivityDepth;
    BYTE            MaxRecursivityDepth;

    PVOID           Holder;
} REC_RW_SPINLOCK, *PREC_RW_SPINLOCK;
#pragma pack(pop)


//******************************************************************************
// Function:     RecRwSpinlockInit
// Description:  
// Returns:      void
// Parameter:    IN BYTE RecursivityDepth - If parameter is 0 it will default
//                                          to 2.
// Parameter:    OUT PREC_RW_SPINLOCK Spinlock
//******************************************************************************
void
RecRwSpinlockInit(
    IN      BYTE                    RecursivityDepth,
    OUT     PREC_RW_SPINLOCK        Spinlock
    );

REQUIRES_NOT_HELD_LOCK(*Spinlock)
_When_(Exclusive, ACQUIRES_EXCL_AND_REENTRANT_LOCK(*Spinlock))
_When_(!Exclusive, ACQUIRES_SHARED_AND_NON_REENTRANT_LOCK(*Spinlock))
void
RecRwSpinlockAcquire(
    INOUT   REC_RW_SPINLOCK     *Spinlock,
    OUT     INTR_STATE*         IntrState,
    IN      BOOLEAN             Exclusive
    );

#define RecRwSpinlockAcquireShared(Lck,Intr)      RecRwSpinlockAcquire((Lck),(Intr),FALSE)
#define RecRwSpinlockAcquireExclusive(Lck,Intr)   RecRwSpinlockAcquire((Lck),(Intr),TRUE)

_When_(Exclusive, REQUIRES_EXCL_LOCK(*Spinlock) RELEASES_EXCL_AND_REENTRANT_LOCK(*Spinlock))
_When_(!Exclusive, REQUIRES_SHARED_LOCK(*Spinlock) RELEASES_SHARED_AND_NON_REENTRANT_LOCK(*Spinlock))
void
RecRwSpinlockRelease(
    INOUT   REC_RW_SPINLOCK     *Spinlock,
    IN      INTR_STATE          IntrState,
    IN      BOOLEAN             Exclusive
    );

#define RecRwSpinlockReleaseShared(Lck,Intr)      RecRwSpinlockRelease((Lck),(Intr),FALSE)
#define RecRwSpinlockReleaseExclusive(Lck,Intr)   RecRwSpinlockRelease((Lck),(Intr),TRUE)
C_HEADER_END
