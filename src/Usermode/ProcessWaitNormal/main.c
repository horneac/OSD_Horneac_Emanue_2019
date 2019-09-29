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
    UM_HANDLE hProcess1;
    UM_HANDLE hProcess2;

    UNREFERENCED_PARAMETER(argc);
    UNREFERENCED_PARAMETER(argv);

    hProcess1 = UM_INVALID_HANDLE_VALUE;
    hProcess2 = UM_INVALID_HANDLE_VALUE;

    __try
    {
        status = SyscallProcessCreate("dummy.exe",
                                      sizeof("dummy.exe"),
                                      NULL,
                                      0,
                                      &hProcess1);
        if (!SUCCEEDED(status))
        {
            LOG_FUNC_ERROR("SyscallProcessCreate", status);
            __leave;
        }

        status = SyscallProcessWaitForTermination(hProcess1, &terminationStatus);
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

        status = SyscallProcessCreate("dummyf~1.exe",
                                      sizeof("dummyf~1.exe"),
                                      NULL,
                                      0,
                                      &hProcess2);
        if (!SUCCEEDED(status))
        {
            LOG_FUNC_ERROR("SyscallProcessCreate", status);
            __leave;
        }

        status = SyscallProcessWaitForTermination(hProcess2, &terminationStatus);
        if (!SUCCEEDED(status))
        {
            LOG_FUNC_ERROR("SyscallProcessWaitForTermination", status);
            __leave;
        }

        if (terminationStatus != STATUS_UNSUCCESSFUL)
        {
            LOG_ERROR("Process should have terminated with status 0x%x, but terminated with 0x%x\n",
                      STATUS_UNSUCCESSFUL, terminationStatus);
            __leave;
        }
    }
    __finally
    {
        if (hProcess1 != UM_INVALID_HANDLE_VALUE)
        {
            status = SyscallProcessCloseHandle(hProcess1);
            if (!SUCCEEDED(status))
            {
                LOG_FUNC_ERROR("SyscallProcessCloseHandle", status);
            }
            hProcess1 = UM_INVALID_HANDLE_VALUE;
        }

        if (hProcess2 != UM_INVALID_HANDLE_VALUE)
        {
            status = SyscallProcessCloseHandle(hProcess2);
            if (!SUCCEEDED(status))
            {
                LOG_FUNC_ERROR("SyscallProcessCloseHandle", status);
            }
            hProcess2 = UM_INVALID_HANDLE_VALUE;
        }
    }

    return STATUS_SUCCESS;
}