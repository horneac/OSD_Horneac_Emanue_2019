#include "common_lib.h"
#include "syscall_if.h"
#include "um_lib_helper.h"

#define THREADS_TO_CREATE           10

static
STATUS
(__cdecl _ThreadFunc)(
    IN_OPT      PVOID       Context
    )
{
    UNREFERENCED_PARAMETER(Context);

    return STATUS_SUCCESS;
}

STATUS
__main(
    DWORD       argc,
    char**      argv
)
{
    UM_HANDLE hThreads[THREADS_TO_CREATE];
    STATUS status;

    UNREFERENCED_PARAMETER(argc);
    UNREFERENCED_PARAMETER(argv);

    for (DWORD i = 0; i < THREADS_TO_CREATE; ++i)
    {
        hThreads[i] = UM_INVALID_HANDLE_VALUE;
    }

    __try
    {
        for (DWORD i = 0; i < THREADS_TO_CREATE; ++i)
        {
            status = UmThreadCreate(_ThreadFunc, NULL, &hThreads[i]);
            if (!SUCCEEDED(status))
            {
                LOG_FUNC_ERROR("UmThreadCreate", status);
                __leave;
            }

            if (hThreads[i] == UM_INVALID_HANDLE_VALUE)
            {
                LOG_ERROR("UmThreadCreate succeeded but the returned thread handle is 0x%X\n", hThreads[i]);
                __leave;
            }
        }
    }
    __finally
    {
        for (DWORD i = 0; i < THREADS_TO_CREATE; ++i)
        {
            SyscallThreadCloseHandle(hThreads[i]);
            hThreads[i] = UM_INVALID_HANDLE_VALUE;
        }
    }

    return STATUS_SUCCESS;
}