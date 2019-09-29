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
    QWORD bytesRead;

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

    status = SyscallFileRead(handle, (PVOID) 0xFFFF800000000000ULL, PAGE_SIZE, &bytesRead);
    if (SUCCEEDED(status))
    {
        LOG_ERROR("SyscallFileRead should have failed!\n");
    }

    return STATUS_SUCCESS;
}