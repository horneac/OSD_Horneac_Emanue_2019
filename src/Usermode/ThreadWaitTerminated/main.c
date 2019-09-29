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

STATUS
__main(
    DWORD       argc,
    char**      argv
)
{
    UM_HANDLE hThread;
    STATUS status;
    STATUS terminationStatus;
    volatile QWORD test;

    UNREFERENCED_PARAMETER(argc);
    UNREFERENCED_PARAMETER(argv);

    hThread = UM_INVALID_HANDLE_VALUE;
    test = 0;

    __try
    {
        status = UmThreadCreate(_ThreadFunc, NULL, &hThread);
        if (!SUCCEEDED(status))
        {
            LOG_FUNC_ERROR("UmThreadCreate", status);
            __leave;
        }

        // wait for a period of time to make sure the newly spawned thread has terminated
        for (DWORD i = 0; i < 0x10000; ++i)
        {
            test += i * i;
        }

        status = SyscallThreadWaitForTermination(hThread, &terminationStatus);
        if (!SUCCEEDED(status))
        {
            LOG_FUNC_ERROR("SyscallThreadWaitForTermination", status);
            __leave;
        }

        LOG("Secondary thread has finished execution!");

        if (terminationStatus != STATUS_SUCCESS)
        {
            LOG_ERROR("Thread should have terminated with status 0x%x, however it terminated with status 0x%x\n",
                      STATUS_SUCCESS, terminationStatus);
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