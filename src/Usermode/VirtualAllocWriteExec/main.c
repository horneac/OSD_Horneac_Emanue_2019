#include "common_lib.h"
#include "syscall_if.h"
#include "um_lib_helper.h"

#define VALUE_TO_WRITE              0x37U

STATUS
__main(
    DWORD       argc,
    char**      argv
)
{
    STATUS status;
    volatile BYTE* pAllocatedAddress;

    UNREFERENCED_PARAMETER(argc);
    UNREFERENCED_PARAMETER(argv);

    pAllocatedAddress = NULL;

    status = SyscallVirtualAlloc(NULL,
                                 PAGE_SIZE,
                                 VMM_ALLOC_TYPE_RESERVE | VMM_ALLOC_TYPE_COMMIT,
                                 PAGE_RIGHTS_ALL,
                                 UM_INVALID_HANDLE_VALUE,
                                 0,
                                 (PVOID*) &pAllocatedAddress);
    if (SUCCEEDED(status))
    {
        LOG_ERROR("We shouldn't be able to allocate write + execute memory!\n");
        return status;
    }

    LOG_TEST_PASS;

    return STATUS_SUCCESS;
}