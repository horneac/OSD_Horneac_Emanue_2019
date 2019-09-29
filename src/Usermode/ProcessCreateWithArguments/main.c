#include "common_lib.h"
#include "syscall_if.h"
#include "um_lib_helper.h"

STATUS
__main(
    DWORD       argc,
    char**      argv
)
{
    STATUS status;
    UM_HANDLE hProcessHandle;
    STATUS terminationStatus;

    UNREFERENCED_PARAMETER(argc);
    UNREFERENCED_PARAMETER(argv);

    hProcessHandle = UM_INVALID_HANDLE_VALUE;

    __try
    {
        status = SyscallProcessCreate("args.exe",
                                      sizeof("args.exe"),
                                      "Let's get some arguments printed!",
                                      sizeof("Let's get some arguments printed!"),
                                      &hProcessHandle);
        if (!SUCCEEDED(status))
        {
            LOG_FUNC_ERROR("SyscallProcessCreate", status);
        }

        status = SyscallProcessWaitForTermination(hProcessHandle, &terminationStatus);
        if (!SUCCEEDED(status))
        {
            LOG_FUNC_ERROR("SyscallProcessWaitForTermination", status);
        }
    }
    __finally
    {
        if (hProcessHandle != UM_INVALID_HANDLE_VALUE)
        {
            status = SyscallProcessCloseHandle(hProcessHandle);
            if (!SUCCEEDED(status))
            {
                LOG_FUNC_ERROR("SyscallProcessCloseHandle", status);
            }

            hProcessHandle = UM_INVALID_HANDLE_VALUE;
        }
    }

    return STATUS_SUCCESS;
}