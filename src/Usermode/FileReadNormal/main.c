#include "common_lib.h"
#include "syscall_if.h"
#include "um_lib_helper.h"

#define EXPECTED_BUFFER "UserModeApplications:Applications"

#define MIN_BUFFER_SIZE     0x200

STATUS
__main(
    DWORD       argc,
    char**      argv
)
{
    STATUS status;
    UM_HANDLE handle;
    QWORD bytesRead;
    BYTE bufferRead[MIN_BUFFER_SIZE];

    UNREFERENCED_PARAMETER(argc);
    UNREFERENCED_PARAMETER(argv);

    handle = UM_INVALID_HANDLE_VALUE;
    bufferRead[MIN_BUFFER_SIZE - 1] = '\0';

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
                                 bufferRead,
                                 MIN_BUFFER_SIZE,
                                 &bytesRead);
        if (!SUCCEEDED(status))
        {
            LOG_FUNC_ERROR("SyscallFileRead", status);
            __leave;
        }

        if (bytesRead != MIN_BUFFER_SIZE)
        {
            LOG_ERROR("We expected to read %U bytes, while we actually read %U!\n",
                      MIN_BUFFER_SIZE, bytesRead);
            __leave;
        }

        if (memcmp(bufferRead, EXPECTED_BUFFER, (DWORD) sizeof(EXPECTED_BUFFER) - 1) != 0)
        {
            LOG_ERROR("Expected buffer is [%s], buffer read is [%s]\n",
                      EXPECTED_BUFFER, bufferRead);
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