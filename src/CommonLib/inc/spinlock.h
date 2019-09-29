#pragma once

C_HEADER_START
#pragma pack(push,16)
typedef struct _SPINLOCK
{
    volatile BYTE       State;
    PVOID               Holder;
    PVOID               FunctionWhichTookLock;
} SPINLOCK, *PSPINLOCK;
#pragma pack(pop)

//******************************************************************************
// Function:     SpinlockInit
// Description:  Initializes a spinlock. No other spinlock* function can be used
//               before this function is called.
// Returns:      void
// Parameter:    OUT PSPINLOCK Lock
//******************************************************************************
void
SpinlockInit(
    OUT         PSPINLOCK       Lock
    );

//******************************************************************************
// Function:     SpinlockAcquire
// Description:  Spins until the Lock is acquired. On return interrupts will be
//               disabled and IntrState will hold the previous interruptibility
//               state.
// Returns:      void
// Parameter:    INOUT PSPINLOCK Lock
// Parameter:    OUT INTR_STATE * IntrState
//******************************************************************************
void
SpinlockAcquire(
    INOUT       PSPINLOCK       Lock,
    OUT         INTR_STATE*     IntrState
    );

//******************************************************************************
// Function:     SpinlockTryAcquire
// Description:  Attempts to acquire the Lock. If it is free then the function
//               will take the lock and return with the interrupts disabled and
//               IntrState will hold the previous interruptibility state.
// Returns:      BOOLEAN - TRUE if the lock was acquired, FALSE otherwise
// Parameter:    INOUT PSPINLOCK Lock
// Parameter:    OUT INTR_STATE * IntrState
//******************************************************************************
BOOL_SUCCESS
BOOLEAN
SpinlockTryAcquire(
    INOUT       PSPINLOCK       Lock,
    OUT         INTR_STATE*     IntrState
    );

//******************************************************************************
// Function:     SpinlockIsOwner
// Description:  Checks if the current CPU is the lock owner.
// Returns:      BOOLEAN
// Parameter:    IN PSPINLOCK Lock
//******************************************************************************
BOOLEAN
SpinlockIsOwner(
    IN          PSPINLOCK       Lock
    );

//******************************************************************************
// Function:     SpinlockRelease
// Description:  Releases a previously acquired Lock. OldIntrState should hold
//               the value previous returned by SpinlockAcquire or
//               SpinlockTryAcquire.
// Returns:      void
// Parameter:    INOUT PSPINLOCK Lock
// Parameter:    IN INTR_STATE OldIntrState
//******************************************************************************
void
SpinlockRelease(
    INOUT       PSPINLOCK       Lock,
    IN          INTR_STATE      OldIntrState
    );
C_HEADER_END
