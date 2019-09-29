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
                                 PAGE_RIGHTS_READ,
                                 UM_INVALID_HANDLE_VALUE,
                                 0,
                                 (PVOID*) &pAllocatedAddress);
    if (!SUCCEEDED(status))
    {
        LOG_FUNC_ERROR("SyscallVirtualAlloc", status);
        return status;
    }

    *pAllocatedAddress = VALUE_TO_WRITE;
    LOG_ERROR("We were able to write in a read-only page!\n");

    return STATUS_SUCCESS;
}