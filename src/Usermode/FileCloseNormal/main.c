#include "common_lib.h"
#include "syscall_if.h"
#include "um_lib_helper.h"

STATUS
__main(
    DWORD       argc,
    char**      argv
)
{
    UM_HANDLE handle;
    STATUS status;

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
            __leave;
        }
    }
    __finally
    {

    }

    return STATUS_SUCCESS;
}