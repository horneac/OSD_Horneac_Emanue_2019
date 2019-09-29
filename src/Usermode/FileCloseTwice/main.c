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
    UM_HANDLE handle;

    UNREFERENCED_PARAMETER(argc);
    UNREFERENCED_PARAMETER(argv);

    __try
    {
        status = SyscallFileCreate("HAL9000.ini",
                                   sizeof("HAL9000.ini"),
                                   FALSE,
                                   FALSE,
                                   &handle);
        if (!SUCCEEDED(status))
        {
            LOG_FUNC_ERROR("SyscallFileCreate", status);
            __leave;
        }

        status = SyscallFileClose(handle);
        if (!SUCCEEDED(status))
        {
            LOG_FUNC_ERROR("SyscallFileClose", status);
        }

        status = SyscallFileClose(handle);
        if (SUCCEEDED(status))
        {
            LOG_ERROR("SyscallFileClose should have failed!!!\n");
        }

        status = SyscallFileClose(UM_FILE_HANDLE_STDOUT);
        if (!SUCCEEDED(status))
        {
            LOG_FUNC_ERROR("SyscallFileClose", status);
        }

        status = SyscallFileClose(UM_FILE_HANDLE_STDOUT);
        if (SUCCEEDED(status))
        {
            LOG_ERROR("SyscallFileClose should have failed!!!\n");
        }
    }
    __finally
    {

    }

    return STATUS_SUCCESS;
}