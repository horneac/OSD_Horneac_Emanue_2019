#include "common_lib.h"
#include "syscall_if.h"
#include "um_lib_helper.h"
#include "strutils.h"

#define VALUE_TO_WRITE              0x37U
#define SHARED_KEY_VALUE            0xC391'3921'0231'3922ULL

#define SIZE_TO_ALLOCATE            (16 * MB_SIZE)

#define PROCESSES_TO_SPAWN          4

STATUS
__main(
    DWORD       argc,
    char**      argv
)
{
    STATUS status;
    volatile QWORD* pAllocatedAddress;
    BOOLEAN bPassed;
    DWORD processIdx;
    BOOLEAN bFirstProcess;

    if (argc != 2)
    {
        LOG_ERROR("Program usage is %s $PROCESS_IDX\n", argv[0]);
        return STATUS_INVALID_PARAMETER1;
    }

    pAllocatedAddress = NULL;
    bPassed = FALSE;
    atoi32(&processIdx, argv[1], BASE_TEN);
    bFirstProcess = (processIdx == 0);


    __try
    {
        status = SyscallVirtualAlloc(NULL,
                                     SIZE_TO_ALLOCATE,
                                     VMM_ALLOC_TYPE_RESERVE | VMM_ALLOC_TYPE_COMMIT,
                                     PAGE_RIGHTS_READWRITE,
                                     UM_INVALID_HANDLE_VALUE,
                                     SHARED_KEY_VALUE,
                                     (PVOID*)&pAllocatedAddress);
        if (!SUCCEEDED(status))
        {
            LOG_FUNC_ERROR("SyscallVirtualAlloc", status);
            __leave;
        }

        if (bFirstProcess)
        {
            UM_HANDLE hProcesses[PROCESSES_TO_SPAWN];
            char nxtProcessIdx[3];

            for (DWORD i = 0; i < PROCESSES_TO_SPAWN; ++i)
            {
                hProcesses[i] = UM_INVALID_HANDLE_VALUE;
            }

            for (DWORD i = 0; i < PROCESSES_TO_SPAWN; ++i)
            {
                snprintf(nxtProcessIdx, sizeof(nxtProcessIdx), "%u", i + 1);

                // create a new reincarcanation of itself
                status = SyscallProcessCreate(argv[0],
                                              strlen(argv[0]) + 1,
                                              nxtProcessIdx,
                                              strlen(nxtProcessIdx) + 1,
                                              &hProcesses[i]);
                if (!SUCCEEDED(status))
                {
                    LOG_FUNC_ERROR("SyscallProcessCreate", status);
                    __leave;
                }
            }

            for (DWORD i = 0; i < PROCESSES_TO_SPAWN; ++i)
            {
                STATUS termStatus;

                status = SyscallProcessWaitForTermination(hProcesses[i], &termStatus);
                if (!SUCCEEDED(status))
                {
                    LOG_FUNC_ERROR("SyscallProcessWaitForTermination", status);
                    __leave;
                }

                if (!SUCCEEDED(termStatus))
                {
                    LOG_ERROR("Process terminated with status 0x%x\n", termStatus);
                }

                status = SyscallProcessCloseHandle(hProcesses[i]);
                if (!SUCCEEDED(status))
                {
                    LOG_FUNC_ERROR("SyscallProcessCloseHandle", status);
                    __leave;
                }

                hProcesses[i] = UM_INVALID_HANDLE_VALUE;
            }

            for (DWORD i = 0; i < SIZE_TO_ALLOCATE / sizeof(*pAllocatedAddress); ++i)
            {
                if (pAllocatedAddress[i] != i + VALUE_TO_WRITE)
                {
                    LOG_ERROR("We have preivously written 0x%X at offset 0x%X, but now we have 0x%X\n",
                              i + VALUE_TO_WRITE, PtrDiff(pAllocatedAddress + i, pAllocatedAddress), pAllocatedAddress[i]);
                    __leave;
                }
            }

            bPassed = TRUE;
        }
        else
        {
            DWORD idxInBuffer = processIdx - 1;

            for (DWORD i = idxInBuffer; i < SIZE_TO_ALLOCATE; i += PROCESSES_TO_SPAWN)
            {
                pAllocatedAddress[i] = i + VALUE_TO_WRITE;
            }
        }

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