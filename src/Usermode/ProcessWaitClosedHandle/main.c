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

        status = SyscallProcessCloseHandle(hProcess);
        if (!SUCCEEDED(status))
        {
            LOG_FUNC_ERROR("SyscallProcessCloseHandle", status);
            __leave;
        }

        status = SyscallProcessWaitForTermination(hProcess, &terminationStatus);
        if (SUCCEEDED(status))
        {
            LOG_ERROR("SyscallProcessWaitForTermination should have failed for closed handle 0x%X", hProcess);
            __leave;
        }

        LOG_TEST_PASS;
    }
    __finally
    {

    }

    return STATUS_SUCCESS;
}