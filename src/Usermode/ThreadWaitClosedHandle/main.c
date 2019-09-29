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

    UNREFERENCED_PARAMETER(argc);
    UNREFERENCED_PARAMETER(argv);

    hThread = UM_INVALID_HANDLE_VALUE;

    __try
    {
        status = UmThreadCreate(_ThreadFunc, NULL, &hThread);
        if (!SUCCEEDED(status))
        {
            LOG_FUNC_ERROR("UmThreadCreate", status);
            __leave;
        }

        status = SyscallThreadCloseHandle(hThread);
        if (!SUCCEEDED(status))
        {
            LOG_FUNC_ERROR("SyscallThreadCloseHandle", status);
            __leave;
        }

        status = SyscallThreadWaitForTermination(hThread, &terminationStatus);
        if (SUCCEEDED(status))
        {
            LOG_ERROR("SyscallThreadWaitForTermination should have failed because thread handle was closed!");
        }
    }
    __finally
    {

    }

    return STATUS_SUCCESS;
}