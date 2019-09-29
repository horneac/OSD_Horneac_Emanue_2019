#include "common_lib.h"
#include "syscall_if.h"
#include "um_lib_helper.h"

static QWORD m_secondaryThTid;

static
STATUS
(__cdecl _ThreadFunc)(
    IN_OPT      PVOID       Context
    )
{
    STATUS status;
    TID tid;

    UNREFERENCED_PARAMETER(Context);

    status = SyscallThreadGetTid(UM_INVALID_HANDLE_VALUE, &tid);
    if (!SUCCEEDED(status))
    {
        LOG_FUNC_ERROR("SyscallThreadGetTid", status);
    }
    else
    {
        m_secondaryThTid = tid;
    }

    return STATUS_SUCCESS;
}

STATUS
__main(
    DWORD       argc,
    char**      argv
)
{
    UM_HANDLE hThread;
    STATUS status;
    TID tid1;
    TID tid2;
    STATUS terminationStatus;

    UNREFERENCED_PARAMETER(argc);
    UNREFERENCED_PARAMETER(argv);

    hThread = UM_INVALID_HANDLE_VALUE;

    __try
    {
        status = SyscallThreadGetTid(UM_INVALID_HANDLE_VALUE, &tid1);
        if (!SUCCEEDED(status))
        {
            LOG_FUNC_ERROR("SyscallThreadGetTid", status);
            __leave;
        }

        status = SyscallThreadGetTid(UM_INVALID_HANDLE_VALUE, &tid2);
        if (!SUCCEEDED(status))
        {
            LOG_FUNC_ERROR("SyscallThreadGetTid", status);
            __leave;
        }

        if (tid1 != tid2)
        {
            LOG_ERROR("The thread should not randomly change its TID, tid1: 0x%X, tid2: 0x%X!\n",
                      tid1, tid2);
            __leave;
        }

        status = UmThreadCreate(_ThreadFunc, NULL, &hThread);
        if (!SUCCEEDED(status))
        {
            LOG_FUNC_ERROR("UmThreadCreate", status);
            __leave;
        }

        status = SyscallThreadGetTid(UM_INVALID_HANDLE_VALUE, &tid2);
        if (!SUCCEEDED(status))
        {
            LOG_FUNC_ERROR("SyscallThreadGetTid", status);
            __leave;
        }

        if (tid1 != tid2)
        {
            LOG_ERROR("The thread should not change its TID after it spawned a thread, tid1: 0x%X, tid2: 0x%X!\n",
                      tid1, tid2);
            __leave;
        }

        status = SyscallThreadGetTid(hThread, &tid2);
        if (!SUCCEEDED(status))
        {
            LOG_FUNC_ERROR("SyscallThreadGetTid", status);
            __leave;
        }

        if (tid1 == tid2)
        {
            LOG_ERROR("Two threads should not have equal TIDs, tid1: 0x%X, tid2: 0x%X!\n",
                      tid1, tid2);
            __leave;
        }

        status = SyscallThreadWaitForTermination(hThread, &terminationStatus);
        if (!SUCCEEDED(status))
        {
            LOG_FUNC_ERROR("SyscallThreadWaitForTermination", status);
            __leave;
        }

        if (tid2 != m_secondaryThTid)
        {
            LOG_ERROR("The TID determined by the main thread for the secondary thread differs from the one determined"
                      "by the secondary thread, tid from primary: 0x%X, tid from secondary: 0x%X\n",
                      tid2, m_secondaryThTid);
            __leave;
        }
    }
    __finally
    {
        if (hThread != UM_INVALID_HANDLE_VALUE)
        {
            SyscallThreadCloseHandle(hThread);
            hThread = UM_INVALID_HANDLE_VALUE;
        }
    }

    return STATUS_SUCCESS;
}