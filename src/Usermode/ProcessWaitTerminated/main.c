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
    STATUS terminationStatus;
    UM_HANDLE hProcess;
    volatile QWORD test = 0;

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

        // wait for a period of time to make sure the newly spawned process has terminated
        for (DWORD i = 0; i < 0x10000; ++i)
        {
            test += i * i;
        }

        status = SyscallProcessWaitForTermination(hProcess, &terminationStatus);
        if (!SUCCEEDED(status))
        {
            LOG_FUNC_ERROR("SyscallProcessWaitForTermination", status);
            __leave;
        }

        if (terminationStatus != STATUS_SUCCESS)
        {
            LOG_ERROR("Process should have terminated with status 0x%x, but terminated with 0x%x\n",
                      STATUS_SUCCESS, terminationStatus);
            __leave;
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