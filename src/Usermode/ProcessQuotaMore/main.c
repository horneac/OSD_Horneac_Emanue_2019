#include "common_lib.h"
#include "syscall_if.h"
#include "um_lib_helper.h"

#define HANDLES_TO_OPEN         32
#define NO_OF_VALID_HANDLES     16

STATUS
__main(
    DWORD       argc,
    char**      argv
)
{
    STATUS status;
    UM_HANDLE handles[HANDLES_TO_OPEN];

    UNREFERENCED_PARAMETER(argc);
    UNREFERENCED_PARAMETER(argv);

    for (DWORD i = 0; i < HANDLES_TO_OPEN; ++i)
    {
        status = SyscallFileCreate("HAL9000.ini",
                                   sizeof("HAL9000.ini"),
                                   FALSE,
                                   FALSE,
                                   &handles[i]);
        if ((i < NO_OF_VALID_HANDLES) && !SUCCEEDED(status))
        {
            LOG_FUNC_ERROR("SyscallFileCreate", status);
            return status;
        }

        if ((i >= NO_OF_VALID_HANDLES) && SUCCEEDED(status))
        {
            LOG_ERROR("SyscallFileCreate succeeded even if we exceeded or %u open files quota!\n",
                      NO_OF_VALID_HANDLES);
            return status;
        }
    }

    for (DWORD i = 0; i < NO_OF_VALID_HANDLES; ++i)
    {
        status = SyscallFileClose(handles[i]);
        if (!SUCCEEDED(status))
        {
            LOG_FUNC_ERROR("SyscallFileClose", status);
            return status;
        }

        handles[i] = UM_INVALID_HANDLE_VALUE;
    }

    return STATUS_SUCCESS;
}