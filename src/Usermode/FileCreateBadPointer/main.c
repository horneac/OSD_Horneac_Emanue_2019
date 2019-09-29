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

    status = SyscallFileCreate((char*)0x73213213,
                               0x100,
                               FALSE,
                               FALSE,
                               &handle
                               );
    if (SUCCEEDED(status))
    {
        LOG_ERROR("SyscallFileCreate should have failed!!!\n");
    }

    return STATUS_SUCCESS;
}