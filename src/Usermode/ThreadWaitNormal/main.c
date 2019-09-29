#include "common_lib.h"
#include "syscall_if.h"
#include "um_lib_helper.h"

static
STATUS
(__cdecl _ThreadFunc)(
    IN_OPT      PVOID       Context
    )
{
    UNREFERENCED_PARAMETER(Context);

    LOG("Hello from secondary thread!");

    return STATUS_SUCCESS;
}

static
STATUS
(__cdecl _ThreadFailFunc)(
    IN_OPT      PVOID       Context
    )
{
    UNREFERENCED_PARAMETER(Context);

    LOG("Hello from secondary thread!");

    return STATUS_UNSUCCESSFUL;
}

STATUS
__main(
    DWORD       argc,
    char**      argv
)
{
    UM_HANDLE hThread1;
    UM_HANDLE hThread2;
    STATUS status;
    STATUS terminationStatus;

    UNREFERENCED_PARAMETER(argc);
    UNREFERENCED_PARAMETER(argv);

    hThread1 = UM_INVALID_HANDLE_VALUE;
    hThread2 = UM_INVALID_HANDLE_VALUE;

    __try
    {
        status = UmThreadCreate(_ThreadFunc, NULL, &hThread1);
        if (!SUCCEEDED(status))
        {
            LOG_FUNC_ERROR("UmThreadCreate", status);
            __leave;
        }

        status = SyscallThreadWaitForTermination(hThread1, &terminationStatus);
        if (!SUCCEEDED(status))
        {
            LOG_FUNC_ERROR("SyscallThreadWaitForTermination", status);
            __leave;
        }

        LOG("First secondary thread has finished execution!");

        if (terminationStatus != STATUS_SUCCESS)
        {
            LOG_ERROR("Thread should have terminated with status 0x%x, however it terminated with status 0x%x\n",
                      STATUS_SUCCESS, terminationStatus);
            __leave;
        }

        status = UmThreadCreate(_ThreadFailFunc, NULL, &hThread2);
        if (!SUCCEEDED(status))
        {
            LOG_FUNC_ERROR("UmThreadCreate", status);
            __leave;
        }

        status = SyscallThreadWaitForTermination(hThread2, &terminationStatus);
        if (!SUCCEEDED(status))
        {
            LOG_FUNC_ERROR("SyscallThreadWaitForTermination", status);
            __leave;
        }

        LOG("Second secondary thread has finished execution!");

        if (terminationStatus != STATUS_UNSUCCESSFUL)
        {
            LOG_ERROR("Thread should have terminated with status 0x%x, however it terminated with status 0x%x\n",
                      STATUS_UNSUCCESSFUL, terminationStatus);
            __leave;
        }
    }
    __finally
    {
        if (hThread1 != UM_INVALID_HANDLE_VALUE)
        {
            SyscallThreadCloseHandle(hThread1);
            hThread1 = UM_INVALID_HANDLE_VALUE;
        }

        if (hThread2 != UM_INVALID_HANDLE_VALUE)
        {
            SyscallThreadCloseHandle(hThread2);
            hThread2 = UM_INVALID_HANDLE_VALUE;
        }
    }

    return STATUS_SUCCESS;
}