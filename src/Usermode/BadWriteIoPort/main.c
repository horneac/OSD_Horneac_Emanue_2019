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

    __outbyte(0x60, 0x5);
    LOG_ERROR("Should have terminated the process!");

    return STATUS_SUCCESS;
}