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

    LOG("Congratulations - you have successfully read an IO port: 0x%x",
        __indword(0x60));
    LOG_ERROR("Should have terminated the process!");

    return STATUS_SUCCESS;
}