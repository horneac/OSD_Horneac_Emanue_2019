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
    STATUS terminationStatus;

    UNREFERENCED_PARAMETER(argc);
    UNREFERENCED_PARAMETER(argv);

    status = SyscallProcessWaitForTermination(0x700, &terminationStatus);
    if (SUCCEEDED(status))
    {
        LOG_ERROR("SyscallProcessWaitForTermination should have failed for invalid handle!\n");
    }

    return STATUS_SUCCESS;
}