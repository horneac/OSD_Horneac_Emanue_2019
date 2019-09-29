#include "common_lib.h"
#include "syscall_if.h"
#include "um_lib_helper.h"

STATUS
__main(
    DWORD       argc,
    char**      argv
)
{
    UNREFERENCED_PARAMETER(argc);
    UNREFERENCED_PARAMETER(argv);

    LOG("Congratulations - you have successfully read from a non-canonical address: 0x%X",
        *((QWORD*)0xFFFF000000000000ULL));
    LOG_ERROR("Should have terminated the process!");

    return STATUS_SUCCESS;
}