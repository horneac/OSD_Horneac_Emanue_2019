#include "common_lib.h"
#include "lock_common.h"

#ifndef _COMMONLIB_NO_LOCKS_

PFUNC_LockInit           LockInit = NULL;

PFUNC_LockAcquire        LockAcquire = NULL;

PFUNC_LockTryAcquire     LockTryAcquire = NULL;

PFUNC_LockRelease        LockRelease = NULL;

PFUNC_LockIsOwner        LockIsOwner = NULL;

#pragma warning(push)
// warning C4028: formal parameter 1 different from declaration
#pragma warning(disable:4028)

void
LockSystemInit(
    IN      BOOLEAN             MonitorSupport
    )
{

    if (MonitorSupport)
    {
        // we have monitor support
        LockInit = MonitorLockInit;
        LockAcquire = MonitorLockAcquire;
        LockTryAcquire = MonitorLockTryAcquire;
        LockIsOwner = MonitorLockIsOwner;
        LockRelease = MonitorLockRelease;
    }
    else
    {
        // use classic spinlock
        LockInit = SpinlockInit;
        LockAcquire = SpinlockAcquire;
        LockTryAcquire = SpinlockTryAcquire;
        LockIsOwner = SpinlockIsOwner;
        LockRelease = SpinlockRelease;
    }
}
#pragma warning(pop)

#endif // _COMMONLIB_NO_LOCKS_
