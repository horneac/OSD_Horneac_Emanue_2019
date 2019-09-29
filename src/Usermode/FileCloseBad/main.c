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

    status = SyscallFileClose(0x73213213);
    if (SUCCEEDED(status))
    {
        LOG_ERROR("SyscallFileClose should have failed!!!\n");
    }

    return STATUS_SUCCESS;
}