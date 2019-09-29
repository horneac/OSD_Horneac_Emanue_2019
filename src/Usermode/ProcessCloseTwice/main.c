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
    STATUS termStatus;

    UNREFERENCED_PARAMETER(argc);
    UNREFERENCED_PARAMETER(argv);

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

        status = SyscallProcessWaitForTermination(hProcess, &termStatus);
        if (!SUCCEEDED(status))
        {
            LOG_FUNC_ERROR("SyscallThreadWaitForTermination", status);
            __leave;
        }

        status = SyscallProcessCloseHandle(hProcess);
        if (!SUCCEEDED(status))
        {
            LOG_FUNC_ERROR("SyscallProcessCloseHandle", status);
            __leave;
        }

        status = SyscallProcessCloseHandle(hProcess);
        if (SUCCEEDED(status))
        {
            LOG_ERROR("SyscallProcessCloseHandle should have failed on the second close attempt!\n");
            __leave;
        }
    }
    __finally
    {

    }

    return STATUS_SUCCESS;
}