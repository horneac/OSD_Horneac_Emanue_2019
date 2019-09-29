#include "um_lib_base.h"

#define RFLAGS_DIRECTION_BIT            ((QWORD)1<<10)

extern
STATUS
__main(
    DWORD       argc,
    char**      argv
    );

static FUNC_AssertFunction _UmLibAssert;
static BOOLEAN m_syscallIfVerified;

extern
void
__start(
    DWORD       argc,
    char**      argv
    )
{
    STATUS status;
    COMMON_LIB_INIT libInit;

    status = STATUS_SUCCESS;
    memzero(&libInit, sizeof(COMMON_LIB_INIT));

    __try
    {

        libInit.Size = sizeof(COMMON_LIB_INIT);
        libInit.MonitorSupport = FALSE; // we do not support MONITOR in UM
        libInit.AssertFunction = _UmLibAssert;

        // Setup common lib
        status = CommonLibInit(&libInit);
        if (!SUCCEEDED(status))
        {
            __leave;
        }

        CHECK_STACK_ALIGNMENT;

        // Validate syscall interface
        status = SyscallValidateInterface(SYSCALL_IMPLEMENTED_IF_VERSION);
        if (!SUCCEEDED(status))
        {
            __leave;
        }
        // We can only start logging from here

        m_syscallIfVerified = TRUE;
        LOG("Successfully validated the interface!");

        // Setup Heap

        // Call actual function
        status = __main(argc, argv);

        LOG("Returned from main function, will call ThreadExit with status 0x%x!",
             status);

        SyscallThreadExit(status);
    }
    __finally
    {
    }

    NOT_REACHED;
}

void
(__cdecl _UmLibAssert)(
    IN_Z            char*           Message
    )
{
    UNREFERENCED_PARAMETER(Message);

    if (!m_syscallIfVerified)
    {
        __writemsr(0xDEADBEEF, 0x0);

    }
    else
    {
        LOG_ERROR("%s\n", Message);

        SyscallProcessExit(STATUS_ASSERTION_FAILURE);
    }

    NOT_REACHED;
}

__forceinline
extern
void
CpuClearDirectionFlag(
    void
    )
{
    __writeeflags(__readeflags() & (~RFLAGS_DIRECTION_BIT));
}