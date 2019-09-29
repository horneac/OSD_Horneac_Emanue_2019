#include "test_common.h"
#include "test_thread.h"
#include "test_priority_scheduler.h"
#include "ex_event.h"
#include "mutex.h"
#include "thread_internal.h"

#define PRIORITY_SCHEDULER_NO_OF_ITERATIONS             16

#pragma warning(push)

// warning C4200: nonstandard extension used: zero-sized array in struct/union
#pragma warning(disable:4200)
typedef struct _TEST_PRIORITY_EXEC_CTX
{
    BOOLEAN                 MultipleThreads;

    LOCK                    Lock;
    DWORD                   CurrentIndex;
    DWORD                   MaxIndex;

    TID                     WakeupTids[0];
} TEST_PRIORITY_EXEC_CTX, *PTEST_PRIORITY_EXEC_CTX;
#pragma warning(pop)

void
(__cdecl TestPrepareMutex)(
    OUT_OPT_PTR     PVOID*              Context,
    IN              DWORD               NumberOfThreads,
    IN              PVOID               PrepareContext
    )
{
    PMUTEX pMutex;
    BOOLEAN acquireMutex;

    ASSERT( NULL != Context );

    UNREFERENCED_PARAMETER(NumberOfThreads);

    // warning C4305: 'type cast': truncation from 'const PVOID' to 'BOOLEAN'
#pragma warning(suppress:4305)
    acquireMutex = (BOOLEAN) PrepareContext;

    pMutex = ExAllocatePoolWithTag(PoolAllocateZeroMemory | PoolAllocatePanicIfFail, sizeof(MUTEX), HEAP_TEST_TAG, 0 );

    MutexInit(pMutex, FALSE);

    if (acquireMutex)
    {
        MutexAcquire(pMutex);
    }

    *Context = pMutex;
}

void
(__cdecl TestThreadPostCreateMutex)(
    IN              PVOID               Context
    )
{
    PMUTEX pMutex;

    pMutex = (PMUTEX)Context;
    ASSERT(pMutex != NULL);

    MutexRelease(pMutex);
}

STATUS
(__cdecl TestThreadPriorityMutex)(
    IN_OPT      PVOID       Context
    )
{
    PMUTEX pMutex;

    pMutex = (PMUTEX)Context;
    ASSERT(pMutex != NULL);

    MutexAcquire(pMutex);

    LOG_TEST_LOG("Thread [%s] with priority %u received MUTEX!\n",
                 ThreadGetName(NULL), ThreadGetPriority(NULL));

    MutexRelease(pMutex);

    return STATUS_SUCCESS;
}

void
(__cdecl TestThreadPrepareWakeupEvent)(
    OUT_OPT_PTR     PVOID*              Context,
    IN              DWORD               NumberOfThreads,
    IN              PVOID               PrepareContext
    )
{
    STATUS status;
    PEX_EVENT pWakeupEvent;

    ASSERT( NULL != Context );
    ASSERT(PrepareContext == NULL);

    UNREFERENCED_PARAMETER(NumberOfThreads);

    pWakeupEvent = ExAllocatePoolWithTag(PoolAllocateZeroMemory | PoolAllocatePanicIfFail,
                                         sizeof(EX_EVENT),
                                         HEAP_TEST_TAG, 0 );

    status = ExEventInit(pWakeupEvent,
                         ExEventTypeSynchronization,
                         FALSE);
    ASSERT(SUCCEEDED(status));

    *Context = pWakeupEvent;
}

void
(__cdecl TestThreadPostCreateWakeup)(
    IN              PVOID               Context
    )
{
    PEX_EVENT pWakeupEvent;

    pWakeupEvent = (PEX_EVENT) Context;
    ASSERT(pWakeupEvent != NULL);

    ExEventSignal(pWakeupEvent);
}

STATUS
(__cdecl TestThreadPriorityWakeup)(
    IN_OPT      PVOID       Context
    )
{
    PEX_EVENT pWakeupEvent;

    pWakeupEvent = (PEX_EVENT) Context;
    ASSERT(pWakeupEvent != NULL);

    ExEventWaitForSignal(pWakeupEvent);

    LOG_TEST_LOG("Thread [%s] with priority %u woke up!\n",
                 ThreadGetName(NULL), ThreadGetPriority(NULL));

    ExEventSignal(pWakeupEvent);

    return STATUS_SUCCESS;
}

STATUS
(__cdecl TestThreadPriorityExecution)(
    IN_OPT      PVOID       Context
    )
{
    PTEST_PRIORITY_EXEC_CTX pCtx;
    TID tid;
    THREAD_PRIORITY priority;
    BOOLEAN bFailed;

    ASSERT(Context != NULL);

    pCtx = (PTEST_PRIORITY_EXEC_CTX) Context;

    tid = ThreadGetId(NULL);
    priority = ThreadGetPriority(NULL);
    bFailed = FALSE;

    for (QWORD i = 0; i < PRIORITY_SCHEDULER_NO_OF_ITERATIONS; ++i)
    {
        QWORD uninterruptedTicks = GetCurrentThread()->UninterruptedTicks;
        INTR_STATE oldState;

        LockAcquire(&pCtx->Lock, &oldState);
        pCtx->WakeupTids[pCtx->CurrentIndex++] = tid;
        LockRelease(&pCtx->Lock, oldState);
        ThreadYield();

        if (pCtx->MultipleThreads)
        {
            if (uninterruptedTicks != 0)
            {
                LOG_ERROR("The thread should not have any uninterrupted ticks, it should have yielded the CPU"
                          "in a RR fashion to the next thread in list!\n");
                bFailed = TRUE;
                break;
            }
        }
        else
        {
            if (uninterruptedTicks < i)
            {
                LOG_ERROR("The thread has %U uninterrupted ticks and it should have at least %U\n",
                          uninterruptedTicks, i);
                bFailed = TRUE;
                break;
            }
        }
    }

    if (!pCtx->MultipleThreads && !bFailed)
    {
        // In the case of the round-robin test we still need to make some checks from the perl .check script
        // however, in the case of the single thread high priority we can determine if it was uninterrupted that it
        // was not de-scheduled when it yielded the CPU
        LOG_TEST_PASS;
    }

    return STATUS_SUCCESS;
}

void
(__cdecl TestThreadPreparePriorityExecution)(
    OUT_OPT_PTR     PVOID*              Context,
    IN              DWORD               NumberOfThreads,
    IN              PVOID               PrepareContext
    )
{
    BOOLEAN bMultipleThreads;
    PTEST_PRIORITY_EXEC_CTX pNewContext;

    ASSERT(Context != NULL);

    // warning C4305: 'type cast': truncation from 'const PVOID' to 'BOOLEAN'
#pragma warning(suppress:4305)
    bMultipleThreads = (BOOLEAN) PrepareContext;

    pNewContext = ExAllocatePoolWithTag(PoolAllocateZeroMemory | PoolAllocatePanicIfFail,
                                        sizeof(TEST_PRIORITY_EXEC_CTX) + sizeof(TID) * NumberOfThreads *PRIORITY_SCHEDULER_NO_OF_ITERATIONS,
                                        HEAP_TEST_TAG,
                                        0);

    pNewContext->CurrentIndex = 0;
    pNewContext->MaxIndex = NumberOfThreads * PRIORITY_SCHEDULER_NO_OF_ITERATIONS;
    pNewContext->MultipleThreads = bMultipleThreads;
    LockInit(&pNewContext->Lock);

    *Context = pNewContext;
}

void
(__cdecl TestThreadPostPriorityExecution)(
    IN              PVOID               Context,
    IN              DWORD               NumberOfThreads
    )
{
    PTEST_PRIORITY_EXEC_CTX pCtx;

    UNREFERENCED_PARAMETER(NumberOfThreads);

    pCtx = (PTEST_PRIORITY_EXEC_CTX) Context;
    ASSERT(pCtx != NULL);

    if (pCtx->MultipleThreads)
    {
        for (DWORD i = 0; i < pCtx->MaxIndex; ++i)
        {
            LOG_TEST_LOG("Thread 0x%X with priority %u has %u uninterrupted ticks!\n",
                         pCtx->WakeupTids[i], ThreadPriorityMaximum, 0);
        }
    }
}