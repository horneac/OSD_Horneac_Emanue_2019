#include "common_lib.h"
#include "syscall_if.h"
#include "um_lib_helper.h"
#include "strutils.h"

#define SHARED_KEY_VALUE            0x7391'3921'0231'3922ULL

typedef enum _SCENARIO_IDX
{
    ScenarioIdxNormal       = 0,
    ScenarioIdxLessAccess,
    ScenarioIdxMoreAccess,
    ScenarioIdxDifferentSize,
    ScenarioIdxEager,
    ScenarioIdxLazy
} SCENARIO_IDX;

STATUS
__main(
    DWORD       argc,
    char**      argv
)
{
    STATUS status;
    volatile QWORD* pAllocatedAddress;
    BOOLEAN bPassed;
    BOOLEAN bFirstProcess;
    UM_HANDLE hProcess;
    DWORD processIdx;
    PAGE_RIGHTS pageRights;
    QWORD sizeToAllocate;
    DWORD sizeInPages;
    SCENARIO_IDX scenario;

    if (argc != 5)
    {
        LOG_ERROR("Program usage is %s $PROCESS_IDX $ACCESS_RIGHTS $NO_OF_PAGES $SCENARIO_IDX\n", argv[0]);
        return STATUS_INVALID_PARAMETER1;
    }

    pAllocatedAddress = NULL;
    bPassed = FALSE;
    hProcess = UM_INVALID_HANDLE_VALUE;

    atoi32(&processIdx, argv[1], BASE_TEN);
    bFirstProcess = (processIdx == 0);

    atoi32(&pageRights, argv[2], BASE_HEXA);
    atoi32(&sizeInPages, argv[3], BASE_HEXA);

    sizeToAllocate = sizeInPages * PAGE_SIZE;

    atoi32(&scenario, argv[4], BASE_TEN);

    LOG("First process is %u\n", bFirstProcess);
    LOG("Page rights is 0x%x\n", pageRights);
    LOG("Size to allocate is 0x%x\n", sizeToAllocate);
    LOG("Scenario is %u\n", scenario);


    __try
    {
        status = SyscallVirtualAlloc(NULL,
                                     sizeToAllocate,
                                     VMM_ALLOC_TYPE_RESERVE |
                                     VMM_ALLOC_TYPE_COMMIT  |
                                     ((scenario == ScenarioIdxEager) ? VMM_ALLOC_TYPE_NOT_LAZY : 0),
                                     pageRights,
                                     UM_INVALID_HANDLE_VALUE,
                                     SHARED_KEY_VALUE,
                                     (PVOID*) &pAllocatedAddress);
        if (bFirstProcess)
        {
            STATUS termStatus;
            char buffer[MAX_PATH];
            PAGE_RIGHTS childRights;

            if (!SUCCEEDED(status))
            {
                LOG_FUNC_ERROR("SyscallVirtualAlloc", status);
                __leave;
            }

            if (IsBooleanFlagOn(pageRights, PAGE_RIGHTS_WRITE) && scenario != ScenarioIdxLazy)
            {
                for (DWORD i = 0; i < sizeToAllocate / sizeof(*pAllocatedAddress); ++i)
                {
                    pAllocatedAddress[i] = i;
                }
            }

            if (scenario == ScenarioIdxLessAccess)
            {
                childRights = pageRights & (~PAGE_RIGHTS_WRITE);
            }
            else if (scenario == ScenarioIdxMoreAccess)
            {
                childRights = pageRights | PAGE_RIGHTS_WRITE;
            }
            else
            {
                childRights = pageRights;
            }

            status = snprintf(buffer, MAX_PATH, "%u %u %u %u",
                              1,
                              childRights,
                              (scenario == ScenarioIdxDifferentSize) ? sizeInPages * 2 : sizeInPages,
                              scenario);
            ASSERT(SUCCEEDED(status));

            // create a new reincarcanation of itself
            status = SyscallProcessCreate(argv[0],
                                          strlen(argv[0]) + 1,
                                          buffer,
                                          strlen(buffer) + 1,
                                          &hProcess);
            if (!SUCCEEDED(status))
            {
                LOG_FUNC_ERROR("SyscallProcessCreate", status);
                __leave;
            }


            status = SyscallProcessWaitForTermination(hProcess, &termStatus);
            if (!SUCCEEDED(status))
            {
                LOG_FUNC_ERROR("SyscallProcessWaitForTermination", status);
                __leave;
            }

            if (   scenario == ScenarioIdxLessAccess
                || scenario == ScenarioIdxDifferentSize)
            {
                // we expect to find the values with which we initialized the memory
                for (DWORD i = 0; i < sizeToAllocate / sizeof(*pAllocatedAddress); ++i)
                {
                    if (pAllocatedAddress[i] != i)
                    {
                        // fail here
                        LOG_ERROR("We have preivously written 0x%X at offset 0x%X, but now we have 0x%X",
                                  i, PtrDiff(pAllocatedAddress + i, pAllocatedAddress), pAllocatedAddress[i]);
                        __leave;
                    }
                }
            }
            else if (scenario != ScenarioIdxMoreAccess)
            {
                // we expect to see the double value of what we wrote
                // in the case of the more scenario the mapping failed and we only had read access to it => noone wrote
                // anything => garbage data
                for (DWORD i = 0; i < sizeToAllocate / sizeof(*pAllocatedAddress); ++i)
                {
                    if (pAllocatedAddress[i] != i * 2)
                    {
                        // fail here
                        LOG_ERROR("We have preivously written 0x%X at offset 0x%X, but now we have 0x%X",
                                  i * 2, PtrDiff(pAllocatedAddress + i, pAllocatedAddress), pAllocatedAddress[i]);
                        __leave;
                    }
                }
            }

            bPassed = TRUE;
        }
        else
        {
            if (scenario == ScenarioIdxDifferentSize || scenario == ScenarioIdxMoreAccess)
            {
                // must have failed
                if (SUCCEEDED(status))
                {
                    LOG_ERROR("We shouldn't have been able to open the shared memory with a different size of 0x%X or"
                              "With more access rights 0x%x!",
                              sizeToAllocate, pageRights);
                }

                __leave;
            }

            // if we're here we succeeded in obtaining the shared memory address

            if (scenario == ScenarioIdxLazy)
            {
                // If the scenario is the lazy one => the parent process didn't cause the physical frames to be
                // reserved yet, it's the child's job to do so
                for (DWORD i = 0; i < sizeToAllocate / sizeof(*pAllocatedAddress); ++i)
                {
                    pAllocatedAddress[i] = i * 2;
                }
                __leave;
            }

            // We check if the parent wrote i's to the buffer
            for (DWORD i = 0; i < sizeToAllocate / sizeof(*pAllocatedAddress); ++i)
            {
                if (pAllocatedAddress[i] != i)
                {
                    // fail here
                    LOG_ERROR("We have preivously written 0x%X at offset 0x%X, but now we have 0x%X",
                              i, PtrDiff(pAllocatedAddress + i, pAllocatedAddress), pAllocatedAddress[i]);
                    __leave;
                }
            }

            for (DWORD i = 0; i < sizeToAllocate / sizeof(*pAllocatedAddress); ++i)
            {
                pAllocatedAddress[i] *= 2;
            }

            if (scenario == ScenarioIdxLessAccess)
            {
                // fail
                LOG_ERROR("We shouldn't have been able to get here, we only requested read rights: 0x%x", pageRights);
                __leave;
            }
        }

    }
    __finally
    {
        if (pAllocatedAddress != NULL)
        {
            status = SyscallVirtualFree((PVOID)pAllocatedAddress, 0,  VMM_FREE_TYPE_RELEASE);
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