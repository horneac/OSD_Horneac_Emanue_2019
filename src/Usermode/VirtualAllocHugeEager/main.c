#include "common_lib.h"
#include "syscall_if.h"
#include "um_lib_helper.h"

#define VALUE_TO_WRITE              0x37U
#define EAGER_ALLOC_SIZE            (16 * MB_SIZE)

STATUS
__main(
    DWORD       argc,
    char**      argv
)
{
    STATUS status;
    volatile BYTE* pAllocatedAddress;
    BOOLEAN bPassed;

    UNREFERENCED_PARAMETER(argc);
    UNREFERENCED_PARAMETER(argv);

    pAllocatedAddress = NULL;
    bPassed = FALSE;

    __try
    {
        status = SyscallVirtualAlloc(NULL,
                                     EAGER_ALLOC_SIZE,
                                     VMM_ALLOC_TYPE_RESERVE | VMM_ALLOC_TYPE_COMMIT | VMM_ALLOC_TYPE_NOT_LAZY,
                                     PAGE_RIGHTS_READWRITE,
                                     UM_INVALID_HANDLE_VALUE,
                                     0,
                                     (PVOID*) &pAllocatedAddress);
        if (!SUCCEEDED(status))
        {
            LOG_FUNC_ERROR("SyscallVirtualAlloc", status);
            __leave;
        }

        *pAllocatedAddress = VALUE_TO_WRITE;
        if (*pAllocatedAddress != VALUE_TO_WRITE)
        {
            LOG_ERROR("Unable to write value to virtual memory allocated\n");
            __leave;
        }

        bPassed = TRUE;
    }
    __finally
    {
        if (pAllocatedAddress != NULL)
        {
            status = SyscallVirtualFree((PVOID)pAllocatedAddress, 0, VMM_FREE_TYPE_RELEASE);
            if (!SUCCEEDED(status))
            {
                LOG_FUNC_ERROR("SyscallVirtualFree", status);
                bPassed = FALSE;
            }
            pAllocatedAddress = NULL;
        }

        if (bPassed)
        {
            LOG_TEST_PASS;
        }
    }

    return STATUS_SUCCESS;
}