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

    UNREFERENCED_PARAMETER(argc);
    UNREFERENCED_PARAMETER(argv);

    status = SyscallFileClose(UM_FILE_HANDLE_STDOUT);
    if (!SUCCEEDED(status))
    {
        LOG_FUNC_ERROR("SyscallFileClose", status);
    }

    return STATUS_SUCCESS;
}