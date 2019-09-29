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
    PID pid1, pid2;
    UM_HANDLE hProcess;

    UNREFERENCED_PARAMETER(argc);
    UNREFERENCED_PARAMETER(argv);

    hProcess = UM_INVALID_HANDLE_VALUE;

    __try
    {
        status = SyscallProcessGetPid(UM_INVALID_HANDLE_VALUE, &pid1);
        if (!SUCCEEDED(status))
        {
            LOG_FUNC_ERROR("SyscallProcessGetPid", status);
            __leave;
        }

        status = SyscallProcessGetPid(UM_INVALID_HANDLE_VALUE, &pid2);
        if (!SUCCEEDED(status))
        {
            LOG_FUNC_ERROR("SyscallProcessGetPid", status);
            __leave;
        }

        if (pid1 != pid2)
        {
            LOG_ERROR("The process should not randomly change its PID, pid1: 0x%X, pid2: 0x%X!\n",
                      pid1, pid2);
            __leave;
        }

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

        status = SyscallProcessGetPid(UM_INVALID_HANDLE_VALUE, &pid2);
        if (!SUCCEEDED(status))
        {
            LOG_FUNC_ERROR("SyscallProcessGetPid", status);
            __leave;
        }

        if (pid1 != pid2)
        {
            LOG_ERROR("The process should not change its PID after it created a child, pid1: 0x%X, pid2: 0x%X!\n",
                      pid1, pid2);
        }

        status = SyscallProcessGetPid(hProcess, &pid2);
        if (!SUCCEEDED(status))
        {
            LOG_FUNC_ERROR("SyscallProcessGetPid", status);
            __leave;
        }

        if (pid1 == pid2)
        {
            LOG_ERROR("Two processes should not have the same PID, pid1: 0x%X, pid2: 0x%X!\n",
                      pid1, pid2);
        }
    }
    __finally
    {
        if (hProcess != UM_INVALID_HANDLE_VALUE)
        {
            STATUS termStatus;

            status = SyscallProcessWaitForTermination(hProcess, &termStatus);
            if (!SUCCEEDED(status))
            {
                LOG_FUNC_ERROR("SyscallProcessWaitForTermination", status);
            }

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