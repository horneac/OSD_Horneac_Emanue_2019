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

    UNREFERENCED_PARAMETER(argc);
    UNREFERENCED_PARAMETER(argv);

    status = SyscallProcessCreate("dummy.exe",
                                  sizeof("dummy.exe"),
                                  NULL,
                                  0,
                                  &hProcess);
    if (!SUCCEEDED(status))
    {
        LOG_FUNC_ERROR("SyscallProcessCreate", status);
    }
    else
    {
        STATUS termStatus;

        status = SyscallProcessWaitForTermination(hProcess, &termStatus);
        if (!SUCCEEDED(status))
        {
            LOG_FUNC_ERROR("SyscallThreadWaitForTermination", status);
        }

        status = SyscallProcessCloseHandle(hProcess);
        if (!SUCCEEDED(status))
        {
            LOG_FUNC_ERROR("SyscallProcessCloseHandle", status);
        }
    }

    return STATUS_SUCCESS;
}