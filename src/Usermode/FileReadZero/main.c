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
    char buf;

    UNREFERENCED_PARAMETER(argc);
    UNREFERENCED_PARAMETER(argv);

    handle = UM_INVALID_HANDLE_VALUE;

    __try
    {
        status = SyscallFileCreate("HAL9000.ini",
                                   sizeof("HAL9000.ini"),
                                   FALSE,
                                   FALSE,
                                   &handle);
        if (!SUCCEEDED(status))
        {
            LOG_FUNC_ERROR("SyscallFileCreate", status);
            __leave;
        }

        status = SyscallFileRead(handle,
                                 &buf,
                                 0,
                                 &bytesRead);
        if (!SUCCEEDED(status))
        {
            LOG_FUNC_ERROR("SyscallFileRead", status);
            __leave;
        }

        if (bytesRead != 0)
        {
            LOG_ERROR("We expected to read zero bytes, while we actually read %U!\n", bytesRead);
            __leave;
        }

    }
    __finally
    {
        if (handle != UM_INVALID_HANDLE_VALUE)
        {
            status = SyscallFileClose(handle);
            handle = UM_INVALID_HANDLE_VALUE;
        }
    }

    return STATUS_SUCCESS;
}