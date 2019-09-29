#include "common_lib.h"
#include "syscall_if.h"
#include "um_lib_helper.h"

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
    UM_HANDLE hThread;
    STATUS status;

    UNREFERENCED_PARAMETER(argc);
    UNREFERENCED_PARAMETER(argv);

    hThread = UM_INVALID_HANDLE_VALUE;

    __try
    {
        status = UmThreadCreate(_ThreadFunc, NULL, &hThread);
        if (!SUCCEEDED(status))
        {
            LOG_FUNC_ERROR("UmThreadCreate", status);
            __leave;
        }

        status = SyscallThreadCloseHandle(hThread);
        if (!SUCCEEDED(status))
        {
            LOG_FUNC_ERROR("SyscallThreadCloseHandle", status);
            __leave;
        }

        status = SyscallThreadCloseHandle(hThread);
        if (SUCCEEDED(status))
        {
            LOG_ERROR("SyscallThreadCloseHandle succeeded even if the 0x%X handle was already closed!\n", hThread);
            __leave;
        }
    }
    __finally
    {

    }

    return STATUS_SUCCESS;
}