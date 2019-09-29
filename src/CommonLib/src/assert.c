#include "common_lib.h"
#include "assert.h"
#include "lock_common.h"

#define ASSERT_BUFFER_SIZE              512

static char                             m_assertBuffer[ASSERT_BUFFER_SIZE];
static PFUNC_AssertFunction             m_pAssertFunction = NULL;

#ifndef _COMMONLIB_NO_LOCKS_
static LOCK                             m_assertLock;
#endif // _COMMONLIB_NO_LOCKS_

// This function is called in case no assert function is registered
static FUNC_AssertFunction              _AssertDefaultFunction;

void
AssertInfo(
    IN_Z            char*       Message,
    ...
    )
{
    STATUS status;
    va_list va;
#ifndef _COMMONLIB_NO_LOCKS_
    INTR_STATE oldState;
#endif // _COMMONLIB_NO_LOCKS_

    status = STATUS_SUCCESS;

    va_start(va, Message);

#ifndef _COMMONLIB_NO_LOCKS_
    LockAcquire(&m_assertLock, &oldState );
#endif // _COMMONLIB_NO_LOCKS_

    // we really have to assume the call is successful
    // because even if it fails we have nothing to do:
    // 1. If we ASSERT again this function will be called recursively => BAD
    // 2. If we return from the function => the ASSERT will have no effect => BAD
    status = vsnprintf(m_assertBuffer, ASSERT_BUFFER_SIZE, Message, va);
    _Analysis_assume_(SUCCEEDED(status));

    // call the assert function after the string was formated

    // if the AssertFunction is NULL we will use the default assert function
    if (NULL == m_pAssertFunction)
    {
        _AssertDefaultFunction(m_assertBuffer);
    }
    else
    {
        m_pAssertFunction(m_assertBuffer);
    }

#ifndef _COMMONLIB_NO_LOCKS_
    LockRelease(&m_assertLock, oldState );
#endif // _COMMONLIB_NO_LOCKS_
}

void
AssertSetFunction(
    IN              PFUNC_AssertFunction    AssertFunction
    )
{
    m_pAssertFunction = AssertFunction;

#ifndef _COMMONLIB_NO_LOCKS_
    LockInit(&m_assertLock);
#endif // _COMMONLIB_NO_LOCKS_
}

void
(__cdecl _AssertDefaultFunction)(
    IN_Z            char*           Message
    )
{
    UNREFERENCED_PARAMETER(Message);

    __halt();
}

#ifndef _COMMONLIB_NO_LOCKS_
REQUIRES_EXCL_LOCK(m_assertLock)
RELEASES_EXCL_AND_NON_REENTRANT_LOCK(m_assertLock)
void
AssertFreeLock(
    void
    )
{
    _Analysis_assume_lock_acquired_(m_assertLock);

    LockRelease(&m_assertLock, INTR_OFF );
}
#endif // _COMMONLIB_NO_LOCKS_
