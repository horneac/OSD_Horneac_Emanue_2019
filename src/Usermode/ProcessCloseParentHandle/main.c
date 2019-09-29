#include "common_lib.h"
#include "syscall_if.h"
#include "um_lib_helper.h"
#include "strutils.h"

STATUS
__main(
    DWORD       argc,
    char**      argv
)
{
    if (argc == 1)
    {
        // Initial process, will spawn this process again with an additional argument specifying the handle to close
        STATUS status;
        STATUS terminationStatus;
        UM_HANDLE hFile;
        UM_HANDLE hProcess;
        char argBuffer[MAX_PATH];

        __try
        {
            status = SyscallFileCreate("HAL9000.ini",
                                       sizeof("HAL9000.ini"),
                                       FALSE,
                                       FALSE,
                                       &hFile);
            if (!SUCCEEDED(status))
            {
                LOG_FUNC_ERROR("SyscallFileCreate", status);
                __leave;
            }

            status = snprintf(argBuffer, MAX_PATH, "%U", hFile);
            if (!SUCCEEDED(status))
            {
                LOG_FUNC_ERROR("snprintf", status);
                __leave;
            }

            status = SyscallProcessCreate("Proces~3.exe",
                                          sizeof("Proces~3.exe"),
                                          argBuffer,
                                          MAX_PATH,
                                          &hProcess);
            if (!SUCCEEDED(status))
            {
                LOG_FUNC_ERROR("SyscallProcessCreate", status);
                __leave;
            }

            status = SyscallProcessWaitForTermination(hProcess,
                                                      &terminationStatus);
            if (!SUCCEEDED(status))
            {
                LOG_FUNC_ERROR("SyscallProcessWaitForTermination", status);
                __leave;
            }

            status = SyscallProcessCloseHandle(hProcess);
            if (!SUCCEEDED(status))
            {
                LOG_FUNC_ERROR("SyscallProcessCloseHandle", status);
                __leave;
            }

            status = SyscallFileClose(hFile);
            if (!SUCCEEDED(status))
            {
                LOG_FUNC_ERROR("SyscallFileClose", status);
                __leave;
            }
        }
        __finally
        {

        }
    }
    else
    {
        STATUS status;
        UM_HANDLE hFileHandle;

        atoi64(&hFileHandle, argv[1],BASE_TEN);

        status = SyscallFileClose(hFileHandle);
        if (SUCCEEDED(status))
        {
            LOG_ERROR("SyscallFileClose should have failed for parent file handle!\n");
        }
    }

    return STATUS_SUCCESS;
}