#include "um_lib_base.h"
#include "um_lib_helper.h"

#define LOG_BUF_MAX_SIZE                512

#define UM_MAX_NO_OF_THREADS            128

typedef struct _UM_START_THREAD_CTX
{
    PFUNC_ThreadStart           StartFunction;
    PVOID                       OriginalContext;
} UM_START_THREAD_CTX, *PUM_START_THREAD_CTX;

UM_START_THREAD_CTX m_thStartContext[UM_MAX_NO_OF_THREADS];

_Interlocked_
volatile DWORD m_freeStartCtxIdx;

static FUNC_ThreadStart __start_thread;

void
LogBuffer(
    IN_Z    char*                   FormatBuffer,
    ...
    )
{
    char logBuffer[LOG_BUF_MAX_SIZE];
    va_list va;
    STATUS status;
    QWORD bytesWritten;

    va_start(va, FormatBuffer);

    // resolve formatted buffer
    status = vsnprintf(logBuffer, LOG_BUF_MAX_SIZE, FormatBuffer, va);
    ASSERT(SUCCEEDED(status));

    status = SyscallFileWrite(UM_FILE_HANDLE_STDOUT,
                              logBuffer,
                              strlen(logBuffer) + 1,
                              &bytesWritten);
    ASSERT(SUCCEEDED(status));
    ASSERT(bytesWritten == strlen(logBuffer) + 1);
}

STATUS
UmThreadCreate(
    IN      PFUNC_ThreadStart       StartFunction,
    IN_OPT  PVOID                   Context,
    OUT     UM_HANDLE*              ThreadHandle
    )
{
    DWORD thCtxIdx = _InterlockedIncrement(&m_freeStartCtxIdx) - 1;
    PUM_START_THREAD_CTX pCtx;

    if (thCtxIdx >= UM_MAX_NO_OF_THREADS)
    {
        return STATUS_LIMIT_REACHED;
    }

    pCtx = &m_thStartContext[thCtxIdx];

    pCtx->StartFunction = StartFunction;
    pCtx->OriginalContext = Context;

    return SyscallThreadCreate(__start_thread, pCtx, ThreadHandle);
}

static
STATUS
(__cdecl __start_thread)(
    IN_OPT      PVOID       Context
    )
{
    PUM_START_THREAD_CTX pCtx;
    STATUS status;

    pCtx = (PUM_START_THREAD_CTX)Context;
    ASSERT(pCtx != NULL);

    status = pCtx->StartFunction(pCtx->OriginalContext);

    SyscallThreadExit(status);
    NOT_REACHED;

    return status;
}