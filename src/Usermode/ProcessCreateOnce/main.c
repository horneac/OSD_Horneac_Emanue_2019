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
    UM_HANDLE hProcess;
    STATUS terminationStatus;

    UNREFERENCED_PARAMETER(argc);
    UNREFERENCED_PARAMETER(argv);

    hProcess = UM_INVALID_HANDLE_VALUE;

    __try
    {
        status = SyscallProcessCreate("dummy.exe",
                                      sizeof("dummy.exe"),
                                      NULL,
                                      0,
                                      &hProcess);
        if (!SUCCEEDED(status))
        {
            LOG_FUNC_ERROR("SyscallProcessCreate", status);
            __leave;
        }

        if (hProcess == UM_INVALID_HANDLE_VALUE)
        {
            LOG_ERROR("0x%X is not a valid handle value for a process!\n", UM_INVALID_HANDLE_VALUE);
        }

        status = SyscallProcessWaitForTermination(hProcess, &terminationStatus);
        if (!SUCCEEDED(status))
        {
            LOG_FUNC_ERROR("SyscallProcessWaitForTermination", status);
        }
    }
    __finally
    {
        if (hProcess != UM_INVALID_HANDLE_VALUE)
        {
            status = SyscallProcessCloseHandle(hProcess);
            if (!SUCCEEDED(status))
            {
                LOG_FUNC_ERROR("SyscallProcessCloseHandle", status);
            }
            hProcess = UM_INVALID_HANDLE_VALUE;
        }
    }

    return STATUS_SUCCESS;
}