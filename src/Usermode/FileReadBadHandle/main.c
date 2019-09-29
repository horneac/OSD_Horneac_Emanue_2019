#include "common_lib.h"
#include "syscall_if.h"
#include "um_lib_helper.h"

STATUS
__main(
    DWORD       argc,
    char**      argv
)
{
    const UM_HANDLE BAD_HANDLES[] = { UM_INVALID_HANDLE_VALUE, 5, 1000, MAX_QWORD };

    UNREFERENCED_PARAMETER(argc);
    UNREFERENCED_PARAMETER(argv);

    for (DWORD i = 0; i < ARRAYSIZE(BAD_HANDLES); ++i)
    {
        STATUS status;
        char buf;
        QWORD bytesRead;

        status = SyscallFileRead(BAD_HANDLES[i],
                                 &buf,
                                 1,
                                 &bytesRead);
        if (SUCCEEDED(status))
        {
            LOG_ERROR("SyscallFileRead should have failed because of invalid handle 0x%X\n", BAD_HANDLES[i]);
        }
    }

    return STATUS_SUCCESS;
}