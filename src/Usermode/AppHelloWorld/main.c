#include "common_lib.h"
#include "syscall_if.h"
#include "um_lib_helper.h"

FUNC_ThreadStart _HelloWorldFromThread;

STATUS
__main(
    DWORD       argc,
    char**      argv
    )
{
    STATUS status;
    TID tid;
    PID pid;
    UM_HANDLE umHandle;

    LOG("Hello from your usermode application!\n");

    LOG("Number of arguments 0x%x\n", argc);
    LOG("Arguments at 0x%X\n", argv);
    for (DWORD i = 0; i < argc; ++i)
    {
        LOG("Argument[%u] is at 0x%X\n", i, argv[i]);
        LOG("Argument[%u] is %s\n", i, argv[i]);
    }

    status = SyscallProcessGetPid(UM_INVALID_HANDLE_VALUE, &pid);
    if (!SUCCEEDED(status))
    {
        LOG_FUNC_ERROR("SyscallProcessGetPid", status);
        return status;
    }

    LOG("Hello from process with ID 0x%X\n", pid);


    status = SyscallThreadGetTid(UM_INVALID_HANDLE_VALUE, &tid);
    if (!SUCCEEDED(status))
    {
        LOG_FUNC_ERROR("SyscallThreadGetTid", status);
        return status;
    }

    LOG("Hello from thread with ID 0x%X\n", tid);

    status = UmThreadCreate(_HelloWorldFromThread, (PVOID)(QWORD) argc, &umHandle);
    if (!SUCCEEDED(status))
    {
        LOG_FUNC_ERROR("SyscallThreadCreate", status);
        return status;
    }

    //SyscallThreadCloseHandle()

    return STATUS_SUCCESS;
}

STATUS
(__cdecl _HelloWorldFromThread)(
    IN_OPT      PVOID       Context
    )
{
    STATUS status;
    TID tid;

    ASSERT(Context != NULL);

    status = SyscallThreadGetTid(UM_INVALID_HANDLE_VALUE, &tid);
    if (!SUCCEEDED(status))
    {
        LOG_FUNC_ERROR("SyscallThreadGetTid", status);
        return status;
    }

    LOG("Hello from thread with ID 0x%X\n", tid);
    LOG("Context is 0x%X\n", Context);

    return status;
}
