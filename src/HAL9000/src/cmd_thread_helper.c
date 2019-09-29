#include "HAL9000.h"
#include "cmd_thread_helper.h"
#include "thread_internal.h"
#include "process_internal.h"
#include "print.h"
#include "smp.h"
#include "list.h"
#include "cpumu.h"
#include "display.h"
#include "test_thread.h"
#include "isr.h"
#include "iomu.h"
#include "dmp_cpu.h"
#include "strutils.h"
#include "smp.h"
#include "ex_timer.h"
#include "vmm.h"

#pragma warning(push)

// warning C4212: nonstandard extension used: function declaration used ellipsis
#pragma warning(disable:4212)

// warning C4029: declared formal parameter list different from definition
#pragma warning(disable:4029)

static FUNC_IpcProcessEvent _CmdIpiCmd;

static
void
_CmdHelperPrintThreadFunctions(
    void
    );

__forceinline
static
const char*
_CmdThreadStateToName(
    IN      THREAD_STATE        State
    )
{
    static const char __stateNames[ThreadStateReserved][10] = { "Running", "Ready", "Blocked", "Dying" };

    ASSERT( State < ThreadStateReserved);

    return __stateNames[State];
}

static
void
_CmdReadAndDumpCpuid(
    IN      DWORD               Index,
    IN      DWORD               SubIndex
    );

static FUNC_ListFunction _CmdThreadPrint;

void
(__cdecl CmdListCpus)(
    IN          QWORD       NumberOfParameters
    )
{
    PLIST_ENTRY pCpuListHead;
    PLIST_ENTRY pCurEntry;

    ASSERT(NumberOfParameters == 0);

    pCpuListHead = NULL;

    SmpGetCpuList(&pCpuListHead);

    printf("\n");

    printColor(MAGENTA_COLOR, "%8s", "Apic ID|");
    printColor(MAGENTA_COLOR, "%4s", "BSP|");
    printColor(MAGENTA_COLOR, "%13s", "Idle|");
    printColor(MAGENTA_COLOR, "%13s", "Kernel|");
    printColor(MAGENTA_COLOR, "%13s", "Total|");
    printColor(MAGENTA_COLOR, "%7s", "%|");
    printColor(MAGENTA_COLOR, "%7s", "#PF|");
    printColor(MAGENTA_COLOR, "%15s", "Current Thread|");

    for(pCurEntry = pCpuListHead->Flink;
        pCurEntry != pCpuListHead;
        pCurEntry = pCurEntry->Flink)
    {
        PPCPU pCpu = CONTAINING_RECORD( pCurEntry, PCPU, ListEntry);
        QWORD totalTicks = pCpu->ThreadData.IdleTicks + pCpu->ThreadData.KernelTicks;

        // we can't do division by 0 => we only divide by totalTicks if the tick count is different
        // from 0
        QWORD percentage = 0 != totalTicks ? ( pCpu->ThreadData.IdleTicks * 10000 ) / totalTicks : 0;

        printf("%7x%c", pCpu->ApicId, '|' );
        printf("%3s%c", pCpu->BspProcessor ? "YES" : "NO", '|');
        printf("%12U%c", pCpu->ThreadData.IdleTicks, '|');
        printf("%12U%c", pCpu->ThreadData.KernelTicks, '|');
        printf("%12U%c", totalTicks, '|');
        printf("%3d.%02d%c", percentage / 100, percentage % 100, '|');
        printf("%6U%c", pCpu->PageFaults, '|' );
        printf("%14s%c", pCpu->ThreadData.CurrentThread->Name, '|');
    }
}

void
(__cdecl CmdListThreads)(
    IN          QWORD       NumberOfParameters
    )
{
    STATUS status;

    ASSERT(NumberOfParameters == 0);

    printColor(MAGENTA_COLOR, "%7s", "TID|");
    printColor(MAGENTA_COLOR, "%20s", "Name|");
    printColor(MAGENTA_COLOR, "%5s", "Prio|");
    printColor(MAGENTA_COLOR, "%8s", "State|");
    printColor(MAGENTA_COLOR, "%10s", "Cmp ticks|");
    printColor(MAGENTA_COLOR, "%10s", "Prt ticks|");
    printColor(MAGENTA_COLOR, "%10s", "Ttl ticks|");
    printColor(MAGENTA_COLOR, "%10s", "Process|");

    status = ThreadExecuteForEachThreadEntry(_CmdThreadPrint, NULL );
    ASSERT( SUCCEEDED(status));
}

void
(__cdecl CmdYield)(
    IN          QWORD       NumberOfParameters
    )
{
    ASSERT(NumberOfParameters == 0);

    printf("Will yield CPU\n");

    ThreadYield();
}

void
(__cdecl CmdRunTest)(
    IN          QWORD       NumberOfParameters,
    IN_Z        char*       TestName,
    IN_Z        char*       NumberOfThreadsString
    )
{
    BOOLEAN bFoundTest;
    DWORD noOfThreads;

    ASSERT(1 <= NumberOfParameters && NumberOfParameters <= 2);
    bFoundTest = FALSE;

    if (NumberOfParameters >= 2)
    {
        atoi32(&noOfThreads, NumberOfThreadsString, BASE_TEN);
    }
    else
    {
        noOfThreads = SmpGetNumberOfActiveCpus() * 2;
    }

    for (DWORD i = 0; i < THREADS_TOTAL_NO_OF_TESTS; ++i)
    {
        if (0 == stricmp(THREADS_TEST[i].TestName, TestName))
        {
            printf("Will run test [%s] on %d threads\n", TestName, noOfThreads);
            TestThreadFunctionality(&THREADS_TEST[i], NULL, noOfThreads);
            bFoundTest = TRUE;
            break;
        }
    }

    if (!bFoundTest)
    {
        pwarn("Test [%s] does not exist. Try one of the following tests:\n", TestName);
        _CmdHelperPrintThreadFunctions();
    }
    else
    {
        printf("Finished running test [%s]\n", TestName);
    }
}

void
(__cdecl CmdSendIpi)(
    IN          QWORD               NumberOfParameters,
    IN_Z        char*               SendModeString,
    IN_Z        char*               DestinationString,
    IN_Z        char*               WaitForTerminationString
    )
{
    STATUS status;
    SMP_IPI_SEND_MODE sendMode;
    SMP_DESTINATION destination;
    BOOLEAN bWaitForAll;

    ASSERT(1 <= NumberOfParameters && NumberOfParameters <= 3);

    UNREFERENCED_PARAMETER(WaitForTerminationString);

    atoi32(&sendMode, SendModeString, BASE_TEN);
    memzero(&destination, sizeof(SMP_DESTINATION));
    bWaitForAll = FALSE;

    if (NumberOfParameters >= 2)
    {
        DWORD temp;

        atoi32(&temp, DestinationString, BASE_HEXA);
        if (temp > MAX_BYTE)
        {
            perror("Destination must be between 0 and FF\n");
            return;
        }

        destination.Group.Affinity = (CPU_AFFINITY)temp;
        bWaitForAll = (NumberOfParameters == 3);
    }

    status = SmpSendGenericIpiEx(_CmdIpiCmd, NULL, NULL, NULL, bWaitForAll, sendMode, destination);
    if (!SUCCEEDED(status))
    {
        perror("SmpSendGenericIpiEx failed with status 0x%x\n", status);
    }
    else
    {
        printf("SmpSendGenericIpiEx finished successfully!\n");
    }
}

void
(__cdecl CmdListCpuInterrupts)(
    IN          QWORD       NumberOfParameters
    )
{
    PLIST_ENTRY pCpuListHead;
    PLIST_ENTRY pCurEntry;

    /// 2K on stack
    QWORD grandTotal[NO_OF_TOTAL_INTERRUPTS] = { 0 };
    QWORD total = 0;

    ASSERT(NumberOfParameters == 0);

    pCpuListHead = NULL;

    SmpGetCpuList(&pCpuListHead);

    for (pCurEntry = pCpuListHead->Flink;
         pCurEntry != pCpuListHead;
         pCurEntry = pCurEntry->Flink)
    {
        PPCPU pCpu = CONTAINING_RECORD(pCurEntry, PCPU, ListEntry);

        LOG("Interrupts on CPU 0x%02x\n", pCpu->ApicId );
        for (DWORD i = 0; i < NO_OF_TOTAL_INTERRUPTS; ++i)
        {
            if (0 != pCpu->InterruptsTriggered[i])
            {
                LOG("%12u [0x%02x]\n", pCpu->InterruptsTriggered[i], i, pCpu->ApicId );
                grandTotal[i] = grandTotal[i] + pCpu->InterruptsTriggered[i];
                total = total + pCpu->InterruptsTriggered[i];
            }
        }

        LOG("\n");
    }

    LOG("Total interrupts:\n");
    for (DWORD i = 0; i < NO_OF_TOTAL_INTERRUPTS; ++i)
    {
        if (0 != grandTotal[i])
        {
            LOG("%12u [0x%02x]\n", grandTotal[i], i);
        }
    }
    LOG("%12u [TOTAL]\n", total );
}

void
(__cdecl CmdTestTimer)(
    IN          QWORD               NumberOfParameters,
    IN_Z        char*               TimerTypeString,
    IN_Z        char*               RelativeTimeString,
    IN_Z        char*               NumberOfTimesString
    )
{
    STATUS status;
    EX_TIMER timer;
    QWORD timeToWaitUs;
    DWORD i;
    DWORD noOfTimesToWait;
    DATETIME systemTimeBeginning;
    DATETIME systemTimeEnd;
    char tmpBuffer[MAX_PATH];
    EX_TIMER_TYPE timerMode;
    QWORD relativeTimeUs;
    DWORD noOfTimesToExecute;

    ASSERT(1 <= NumberOfParameters && NumberOfParameters <= 3);

    atoi32(&timerMode, TimerTypeString, BASE_TEN);
    relativeTimeUs = 10 * SEC_IN_US;
    noOfTimesToExecute = 1;

    if (NumberOfParameters >= 2)
    {
        atoi64(&relativeTimeUs, RelativeTimeString, BASE_TEN);

        if (NumberOfParameters >= 3)
        {
            atoi32(&noOfTimesToExecute, NumberOfTimesString, BASE_TEN);
        }
    }

    status = STATUS_SUCCESS;

    printf("Will start a timer of type %u, relative time %U for %u times\n",
           timerMode, relativeTimeUs, noOfTimesToExecute);

    timeToWaitUs = (timerMode == ExTimerTypeAbsolute) ? IomuGetSystemTimeUs() + relativeTimeUs : relativeTimeUs;
    noOfTimesToWait = (timerMode == ExTimerTypeRelativePeriodic) ? noOfTimesToExecute : 1;

    status = ExTimerInit(&timer,
                         timerMode,
                         timeToWaitUs
                         );
    if (!SUCCEEDED(status))
    {
        perror("ExTimerInit failed with status 0x%x\n", status);
        return;
    }

    systemTimeBeginning = IoGetCurrentDateTime();

    ExTimerStart(&timer);

    for (i = 0; i < noOfTimesToExecute; ++i)
    {
        ExTimerWait(&timer);
        printf("Tick at %U us\n", IomuGetSystemTimeUs());
    }

    systemTimeEnd = IoGetCurrentDateTime();

    ExTimerUninit(&timer);
    printf("Timer finished\n");

    TimeGetStringFormattedBuffer(systemTimeBeginning, tmpBuffer, MAX_PATH);
    printf("Timer started at [%s]\n", tmpBuffer);
    TimeGetStringFormattedBuffer(systemTimeEnd, tmpBuffer, MAX_PATH);
    printf("Timer ended at [%s]\n", tmpBuffer);
}

void
(__cdecl CmdCpuid)(
    IN          QWORD               NumberOfParameters,
    IN_Z        char*               IndexString,
    IN_Z        char*               SubIndexString
    )
{
    ASSERT(0 <= NumberOfParameters && NumberOfParameters <= 2);

    if ( NumberOfParameters >= 1 )
    {
        DWORD index;
        DWORD subIdx;

        atoi32(&index, IndexString, BASE_HEXA);
        if (NumberOfParameters >= 2)
        {
            atoi32(&subIdx, SubIndexString, BASE_HEXA);
        }
        else
        {
            subIdx = 0;
        }

        _CmdReadAndDumpCpuid(index, subIdx);
    }
    else
    {
        CPUID_INFO cpuId;

        __cpuid(cpuId.values, CpuidIdxBasicInformation);

        for( DWORD i = 0; i <= cpuId.BasicInformation.MaxValueForBasicInfo; ++i )
        {
            _CmdReadAndDumpCpuid(i, 0);
        }

        __cpuid( cpuId.values, CpuidIdxFeatureInformation );

        if (cpuId.FeatureInformation.ecx.HV)
        {
            for (DWORD i = 0x4000'0000; i <= 0x4000'0020; ++i)
            {
                _CmdReadAndDumpCpuid(i, 0);
            }
        }

        __cpuid(cpuId.values, CpuidIdxExtendedMaxFunction);

        for( DWORD i = 0x8000'0000; i <= cpuId.ExtendedInformation.MaxValueForExtendedInfo; ++i )
        {
            _CmdReadAndDumpCpuid(i, 0);
        }
    }
}

void
(__cdecl CmdRdmsr)(
    IN      QWORD       NumberOfParameters,
    IN_Z    char*       IndexString
    )
{
    DWORD index;

    ASSERT(NumberOfParameters == 1);

    atoi32(&index, IndexString, BASE_HEXA);

    QWORD value = __readmsr(index);

    LOG("RDMSR[%x] = 0x%X\n", index, value );
}

void
(__cdecl CmdWrmsr)(
    IN      QWORD       NumberOfParameters,
    IN_Z    char*       IndexString,
    IN_Z    char*       ValueString
    )
{
    DWORD index;
    QWORD value;

    ASSERT(NumberOfParameters == 2);

    atoi32(&index, IndexString, BASE_HEXA);
    atoi64(&value, ValueString, BASE_HEXA);

    LOG("WRMSR[%x] = 0x%X\n", index, value);

    __writemsr(index, value );
}

void
(__cdecl CmdCheckAd)(
    IN      QWORD       NumberOfParameters
    )
{
    DWORD dirtyCount;
    DWORD accessedCount;
    volatile BYTE* pData;
    PML4 cr3;

    ASSERT(NumberOfParameters == 0);

    dirtyCount = 0;
    accessedCount = 0;
    pData = NULL;
    cr3.Raw = (QWORD) __readcr3();

    pData = ExAllocatePoolWithTag(PoolAllocateZeroMemory, PAGE_SIZE, HEAP_TEMP_TAG, PAGE_SIZE);
    if (pData == NULL)
    {
        return;
    }

    __try
    {
        for (DWORD i = 0; i < PAGE_SIZE; ++i)
        {
            BOOLEAN bAccessed = FALSE;
            BOOLEAN bDirty = FALSE;

            PHYSICAL_ADDRESS pAddr = VmmGetPhysicalAddressEx(cr3,
                                                             (PVOID) pData,
                                                             &bAccessed,
                                                             &bDirty);
            ASSERT(pAddr != NULL);

            accessedCount += (bAccessed == TRUE);
            dirtyCount += (bDirty == TRUE);

        }
    }
    __finally
    {
        ASSERT(pData != NULL);
            ExFreePoolWithTag((PVOID)pData, HEAP_TEMP_TAG);
            pData = NULL;
        }

    LOG("A/D: %u/%u\n", accessedCount, dirtyCount);
}

static
void
_CmdHelperPrintThreadFunctions(
    void
    )
{
    DWORD i;

    for (i = 0; i < THREADS_TOTAL_NO_OF_TESTS; ++i)
    {
        printf("%2u. %s\n", i, THREADS_TEST[i].TestName);
    }
}

static
STATUS
(__cdecl _CmdThreadPrint) (
    IN      PLIST_ENTRY     ListEntry,
    IN_OPT  PVOID           FunctionContext
    )
{
    PTHREAD pThread;

    ASSERT( NULL != ListEntry );
    ASSERT( NULL == FunctionContext );

    pThread = CONTAINING_RECORD(ListEntry, THREAD, AllList );

    printf("%6x%c", pThread->Id, '|');
    printf("%19s%c", pThread->Name, '|');
    printf("%4U%c", pThread->Priority, '|');
    printf("%7s%c", _CmdThreadStateToName(pThread->State), '|');
    printf("%9U%c", pThread->TickCountCompleted, '|');
    printf("%9U%c", pThread->TickCountEarly, '|');
    printf("%9U%c", pThread->TickCountCompleted + pThread->TickCountEarly, '|');
    printf("%9x%c", pThread->Process->Id, '|');

    return STATUS_SUCCESS;
}

static
void
_CmdReadAndDumpCpuid(
    IN      DWORD               Index,
    IN      DWORD               SubIndex
    )
{
    CPUID_INFO cpuidInfo;

    __cpuidex(cpuidInfo.values, Index, SubIndex );

    DumpCpuidValues(Index, SubIndex, cpuidInfo );
}

static
STATUS
(__cdecl _CmdIpiCmd)(
    IN_OPT  PVOID   Context
    )
{
    PCPU* pCpu;

    ASSERT(NULL == Context);

    pCpu = GetCurrentPcpu();
    ASSERT( NULL != pCpu );

    printf("Hello from CPU 0x%02x [0x%02x]\n", pCpu->ApicId, pCpu->LogicalApicId);

    return STATUS_SUCCESS;
}

#pragma warning(pop)
