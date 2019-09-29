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

    status = SyscallFileCreate("inexistent.ini",
                               sizeof("inexistent.ini"),
                               FALSE,
                               FALSE,
                               &handle);
    if (SUCCEEDED(status))
    {
        LOG_ERROR("Function should have failed because file doesn't exist!\n");
    }

    return STATUS_SUCCESS;
}