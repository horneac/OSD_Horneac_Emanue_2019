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

    status = SyscallFileCreate("HAL9000.ini",
                               sizeof("HAL9000.ini"),
                               FALSE,
                               FALSE,
                               &handle);
    if (!SUCCEEDED(status))
    {
        LOG_FUNC_ERROR("SyscallFileCreate", status);
    }

    status = SyscallProcessCloseHandle(handle);
    if (SUCCEEDED(status))
    {
        LOG_ERROR("SyscallProcessCloseHandle should have failed with file handle!\n");
    }

    return STATUS_SUCCESS;
}