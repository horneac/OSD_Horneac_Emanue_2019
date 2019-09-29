#include "common_lib.h"
#include "syscall_if.h"
#include "um_lib_helper.h"

static
STATUS
(__cdecl _ThreadFunc)(
    IN_OPT      PVOID       Context
    )
{
    if (Context != _ThreadFunc)
    {
        LOG_ERROR("Context value should be 0x%X, but it is 0x%X\n",
                  _ThreadFunc, Context);
    }

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
    STATUS terminationStatus;

    UNREFERENCED_PARAMETER(argc);
    UNREFERENCED_PARAMETER(argv);

    hThread = UM_INVALID_HANDLE_VALUE;

    __try
    {
        //  warning C4054: 'type cast': from function pointer 'STATUS (__cdecl *)(const PVOID)' to data pointer 'PVOID'
#pragma warning(suppress:4054)
        status = UmThreadCreate(_ThreadFunc, (PVOID)_ThreadFunc, &hThread);
        if (!SUCCEEDED(status))
        {
            LOG_FUNC_ERROR("UmThreadCreate", status);
            __leave;
        }

        status = SyscallThreadWaitForTermination(hThread, &terminationStatus);
        if (!SUCCEEDED(status))
        {
            LOG_FUNC_ERROR("SyscallThreadWaitForTermination", status);
            __leave;
        }
    }
    __finally
    {
        if (hThread != UM_INVALID_HANDLE_VALUE)
        {
            SyscallThreadCloseHandle(hThread);
            hThread = UM_INVALID_HANDLE_VALUE;
        }
    }

    return STATUS_SUCCESS;
}