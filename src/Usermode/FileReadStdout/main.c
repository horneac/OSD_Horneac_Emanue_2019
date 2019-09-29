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
    QWORD bytesRead;
    char buff;

    UNREFERENCED_PARAMETER(argc);
    UNREFERENCED_PARAMETER(argv);

    status = SyscallFileRead(UM_FILE_HANDLE_STDOUT,
                             &buff,
                             1,
                             &bytesRead);
    if (SUCCEEDED(status))
    {
        LOG_ERROR("We should not be able to read from STDOUT!\n");
    }

    return STATUS_SUCCESS;
}