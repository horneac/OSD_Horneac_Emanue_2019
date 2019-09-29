#include "common_lib.h"
#include "syscall_if.h"
#include "um_lib_helper.h"

#define VALUE_TO_WRITE              0x37U

#define SIZE_TO_ALLOCATE            (16 * MB_SIZE)

STATUS
__main(
    DWORD       argc,
    char**      argv
)
{
    STATUS status;
    volatile QWORD* pAllocatedAddress;
    BOOLEAN bPassed;

    UNREFERENCED_PARAMETER(argc);
    UNREFERENCED_PARAMETER(argv);

    pAllocatedAddress = NULL;
    bPassed = FALSE;

    __try
    {
        status = SyscallVirtualAlloc(NULL,
                                     SIZE_TO_ALLOCATE,
                                     VMM_ALLOC_TYPE_RESERVE | VMM_ALLOC_TYPE_COMMIT,
                                     PAGE_RIGHTS_READWRITE,
                                     UM_INVALID_HANDLE_VALUE,
                                     0,
                                     (PVOID*)&pAllocatedAddress);
        if (!SUCCEEDED(status))
        {
            LOG_FUNC_ERROR("SyscallVirtualAlloc", status);
            __leave;
        }
        memzero((PVOID)pAllocatedAddress, SIZE_TO_ALLOCATE);

        for (DWORD i = 0; i < SIZE_TO_ALLOCATE / sizeof(*pAllocatedAddress); i += PAGE_SIZE)
        {
            pAllocatedAddress[i] = i + VALUE_TO_WRITE;
        }

        for (DWORD i = 0; i < SIZE_TO_ALLOCATE / sizeof(*pAllocatedAddress); i += PAGE_SIZE)
        {
            if (pAllocatedAddress[i] != i + VALUE_TO_WRITE)
            {
                LOG_ERROR("We have preivously written 0x%X at offset 0x%X, but now we have 0x%X\n",
                          i + VALUE_TO_WRITE, PtrDiff(pAllocatedAddress + i, pAllocatedAddress), pAllocatedAddress[i]);
                __leave;
            }

            pAllocatedAddress[i] = 0;
        }

        for (DWORD i = 0; i < SIZE_TO_ALLOCATE / sizeof(*pAllocatedAddress); i += PAGE_SIZE)
        {
            if (pAllocatedAddress[i] != 0)
            {
                LOG_ERROR("We have preivously zeroed at offset 0x%X, but now we have 0x%X\n",
                          PtrDiff(pAllocatedAddress + i, pAllocatedAddress), pAllocatedAddress[i]);
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