#include "common_lib.h"
#include "syscall_if.h"
#include "um_lib_helper.h"

STATUS
__main(
    DWORD       argc,
    char**      argv
)
{
    LOG("Number of arguments 0x%x", argc);
    for (DWORD i = 0; i < argc; ++i)
    {
        LOG("Argument[%u] is [%s]", i, argv[i]);
    }

    return STATUS_SUCCESS;
}