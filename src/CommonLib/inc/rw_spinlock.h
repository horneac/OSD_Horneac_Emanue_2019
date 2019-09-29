#pragma once

C_HEADER_START
#pragma pack(push,16)
typedef struct _RW_SPINLOCK
{
    volatile WORD   WaitingWriters;
    volatile WORD   ActiveWriter;
    volatile WORD   ActiveReaders;
} RW_SPINLOCK, *PRW_SPINLOCK;
STATIC_ASSERT(FIELD_OFFSET(RW_SPINLOCK,WaitingWriters) + sizeof(WORD) == FIELD_OFFSET(RW_SPINLOCK, ActiveWriter));
STATIC_ASSERT(FIELD_OFFSET(RW_SPINLOCK,ActiveWriter) + sizeof(WORD) == FIELD_OFFSET(RW_SPINLOCK, ActiveReaders));
#pragma pack(pop)

void
RwSpinlockInit(
    OUT     PRW_SPINLOCK    Spinlock
    );

REQUIRES_NOT_HELD_LOCK(*Spinlock)
_When_(Exclusive, ACQUIRES_EXCL_AND_NON_REENTRANT_LOCK(*Spinlock))
_When_(!Exclusive, ACQUIRES_SHARED_AND_NON_REENTRANT_LOCK(*Spinlock))
void
RwSpinlockAcquire(
    INOUT   RW_SPINLOCK     *Spinlock,
    OUT     INTR_STATE*     IntrState,
    IN      BOOLEAN         Exclusive
    );

#define RwSpinlockAcquireShared(Lck,Intr)      RwSpinlockAcquire((Lck),(Intr),FALSE)
#define RwSpinlockAcquireExclusive(Lck,Intr)   RwSpinlockAcquire((Lck),(Intr),TRUE)

_When_(Exclusive, REQUIRES_EXCL_LOCK(*Spinlock) RELEASES_EXCL_AND_NON_REENTRANT_LOCK(*Spinlock))
_When_(!Exclusive, REQUIRES_SHARED_LOCK(*Spinlock) RELEASES_SHARED_AND_NON_REENTRANT_LOCK(*Spinlock))
void
RwSpinlockRelease(
    INOUT   RW_SPINLOCK     *Spinlock,
    IN      INTR_STATE      IntrState,
    IN      BOOLEAN         Exclusive
    );

#define RwSpinlockReleaseShared(Lck,Intr)      RwSpinlockRelease((Lck),(Intr),FALSE)
#define RwSpinlockReleaseExclusive(Lck,Intr)   RwSpinlockRelease((Lck),(Intr),TRUE)
C_HEADER_END
