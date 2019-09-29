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
    UM_HANDLE hProcessHandle;

    UNREFERENCED_PARAMETER(argc);
    UNREFERENCED_PARAMETER(argv);

    status = SyscallProcessCreate(NULL,
                                  0,
                                  NULL,
                                  0,
                                  &hProcessHandle);
    if (SUCCEEDED(status))
    {
        LOG_ERROR("Process creation with invalid pointer should have failed!\n");
    }

    return STATUS_SUCCESS;
}