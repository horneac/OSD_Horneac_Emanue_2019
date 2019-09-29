#include "common_lib.h"
#include "syscall_if.h"
#include "um_lib_helper.h"

#define VALUE_TO_WRITE              0x37U

#define MEMORY_SIZE_TO_WRITE        (16 * MB_SIZE)
#define ZERO_ALLOC_SIZE             (256 * MB_SIZE)

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
                                     ZERO_ALLOC_SIZE,
                                     VMM_ALLOC_TYPE_RESERVE | VMM_ALLOC_TYPE_COMMIT | VMM_ALLOC_TYPE_ZERO,
                                     PAGE_RIGHTS_READWRITE,
                                     UM_INVALID_HANDLE_VALUE,
                                     0,
                                     (PVOID*)&pAllocatedAddress);
        if (!SUCCEEDED(status))
        {
            LOG_FUNC_ERROR("SyscallVirtualAlloc", status);
            __leave;
        }

        for (DWORD i = 0; i < MEMORY_SIZE_TO_WRITE; i += PAGE_SIZE)
        {
            if (pAllocatedAddress[i] != 0)
            {
                LOG_ERROR("Allocated zero memory, but it isn't zero!\n");
                __leave;
            }

            pAllocatedAddress[i] = VALUE_TO_WRITE;
        }

        for (DWORD i = 0; i < MEMORY_SIZE_TO_WRITE; i += PAGE_SIZE)
        {
            if (pAllocatedAddress[i] != VALUE_TO_WRITE)
            {
                LOG_ERROR("Changes to zero memory not brought back in memory!\n");
                __leave;
            }
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