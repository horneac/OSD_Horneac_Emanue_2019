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
    UM_HANDLE hFile1, hFile2;

    UNREFERENCED_PARAMETER(argc);
    UNREFERENCED_PARAMETER(argv);

    hFile1 = UM_INVALID_HANDLE_VALUE;
    hFile2 = UM_INVALID_HANDLE_VALUE;

    __try
    {
        status = SyscallFileCreate("HAL9000.ini",
                                   sizeof("HAL9000.ini"),
                                   FALSE,
                                   FALSE,
                                   &hFile1);
        if (!SUCCEEDED(status))
        {
            LOG_FUNC_ERROR("SyscallFileCreate", status);
            __leave;
        }

        status = SyscallFileCreate("HAL9000.ini",
                                   sizeof("HAL9000.ini"),
                                   FALSE,
                                   FALSE,
                                   &hFile2);
        if (!SUCCEEDED(status))
        {
            LOG_FUNC_ERROR("SyscallFileCreate", status);
            __leave;
        }

        if (hFile1 == hFile2)
        {
            LOG_ERROR("File handles should have different values: hFile1 is 0x%X, hFile2 is 0x%X!\n",
                      hFile1, hFile2);
        }
    }
    __finally
    {
        if (hFile1 != UM_INVALID_HANDLE_VALUE)
        {
            SyscallFileClose(hFile1);
            hFile1 = UM_INVALID_HANDLE_VALUE;
        }

        if (hFile2!= UM_INVALID_HANDLE_VALUE)
        {
            SyscallFileClose(hFile2);
            hFile2 = UM_INVALID_HANDLE_VALUE;
        }
    }

    return STATUS_SUCCESS;
}