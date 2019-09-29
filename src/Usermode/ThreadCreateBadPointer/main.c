#include "common_lib.h"
#include "syscall_if.h"
#include "um_lib_helper.h"


STATUS
__main(
    DWORD       argc,
    char**      argv
)
{
    UM_HANDLE hThread;
    volatile QWORD test = 0;

    UNREFERENCED_PARAMETER(argc);
    UNREFERENCED_PARAMETER(argv);

    hThread = UM_INVALID_HANDLE_VALUE;

    __try
    {
        UmThreadCreate((PFUNC_ThreadStart) 0x7000'3203ULL, NULL, &hThread);

        // wait for the process to crash
        while(&hThread)
        {
            test += 1;
            _mm_pause();
        }
    }
    __finally
    {

    }

    return STATUS_SUCCESS;
}