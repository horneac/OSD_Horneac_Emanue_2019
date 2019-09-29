#include "common_lib.h"
#include "syscall_if.h"
#include "um_lib_helper.h"

#define PROCESSES_TO_CREATE         10

STATUS
__main(
    DWORD       argc,
    char**      argv
)
{
    STATUS status;
    UM_HANDLE hProcess[PROCESSES_TO_CREATE];

    UNREFERENCED_PARAMETER(argc);
    UNREFERENCED_PARAMETER(argv);

    for (DWORD i = 0; i < PROCESSES_TO_CREATE; ++i)
    {
        hProcess[i] = UM_INVALID_HANDLE_VALUE;
    }

    __try
    {
        for (DWORD i = 0; i < PROCESSES_TO_CREATE; ++i)
        {
            STATUS termStatus;

            status = SyscallProcessCreate("dummy.exe",
                                          sizeof("dummy.exe"),
                                          NULL,
                                          0,
                                          &hProcess[i]);
            if (!SUCCEEDED(status))
            {
                LOG_FUNC_ERROR("SyscallProcessCreate", status);
                __leave;
            }

            if (hProcess[i] == UM_INVALID_HANDLE_VALUE)
            {
                LOG_ERROR("0x%X is not a valid handle value for a process!\n", UM_INVALID_HANDLE_VALUE);
                __leave;
            }

            status = SyscallProcessWaitForTermination(hProcess[i], &termStatus);
            if (!SUCCEEDED(status))
            {
                LOG_FUNC_ERROR("SyscallProcessWaitForTermination", status);
                __leave;
            }
        }
    }
    __finally
    {
        for (DWORD i = 0; i < PROCESSES_TO_CREATE; ++i)
        {
            if (hProcess[i] != UM_INVALID_HANDLE_VALUE)
            {
                status = SyscallProcessCloseHandle(hProcess[i]);
                if (!SUCCEEDED(status))
                {
                    LOG_FUNC_ERROR("SyscallProcessCloseHandle", status);
                }
                hProcess[i] = UM_INVALID_HANDLE_VALUE;
            }
        }
    }

    return STATUS_SUCCESS;
}