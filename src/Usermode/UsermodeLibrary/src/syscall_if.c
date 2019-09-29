#include "um_lib_base.h"
#include "syscall_if.h"
#include "syscall_no.h"

extern
STATUS
SyscallEntry(
    IN      SYSCALL_ID              SyscallId,
    ...
    );

// SyscallIdIdentifyVersion
STATUS
SyscallValidateInterface(
    IN  SYSCALL_IF_VERSION          InterfaceVersion
    )
{
    return SyscallEntry(SyscallIdIdentifyVersion, InterfaceVersion);
}

// SyscallIdThreadExit
STATUS
SyscallThreadExit(
    IN  STATUS                      ExitStatus
    )
{
    return SyscallEntry(SyscallIdThreadExit, ExitStatus);
}

// SyscallIdThreadCreate
STATUS
SyscallThreadCreate(
    IN      PFUNC_ThreadStart       StartFunction,
    IN_OPT  PVOID                   Context,
    OUT     UM_HANDLE*              ThreadHandle
    )
{
    return SyscallEntry(SyscallIdThreadCreate, StartFunction, Context, ThreadHandle);
}

// SyscallIdThreadGetTid
STATUS
SyscallThreadGetTid(
    IN_OPT  UM_HANDLE               ThreadHandle,
    OUT     TID*                    ThreadId
    )
{
    return SyscallEntry(SyscallIdThreadGetTid, ThreadHandle, ThreadId);
}

// SyscallIdThreadWaitForTermination
STATUS
SyscallThreadWaitForTermination(
    IN      UM_HANDLE               ThreadHandle,
    OUT     STATUS*                 TerminationStatus
    )
{
    return SyscallEntry(SyscallIdThreadWaitForTermination, ThreadHandle, TerminationStatus);
}

// SyscallIdThreadCloseHandle
STATUS
SyscallThreadCloseHandle(
    IN      UM_HANDLE               ThreadHandle
    )
{
    return SyscallEntry(SyscallIdThreadCloseHandle, ThreadHandle);
}

// SyscallIdProcessExit
STATUS
SyscallProcessExit(
    IN      STATUS                  ExitStatus
    )
{
    return SyscallEntry(SyscallIdProcessExit, ExitStatus);
}

// SyscallIdProcessCreate
STATUS
SyscallProcessCreate(
    IN_READS_Z(PathLength)
                char*               ProcessPath,
    IN          QWORD               PathLength,
    IN_READS_OPT_Z(ArgLength)
                char*               Arguments,
    IN          QWORD               ArgLength,
    OUT         UM_HANDLE*          ProcessHandle
    )
{
    return SyscallEntry(SyscallIdProcessCreate, ProcessPath, PathLength, Arguments, ArgLength, ProcessHandle);
}

// SyscallIdProcessGetPid
STATUS
SyscallProcessGetPid(
    IN_OPT  UM_HANDLE               ProcessHandle,
    OUT     PID*                    ProcessId
    )
{
    return SyscallEntry(SyscallIdProcessGetPid, ProcessHandle, ProcessId);
}

// SyscallIdProcessWaitForTermination
STATUS
SyscallProcessWaitForTermination(
    IN      UM_HANDLE               ProcessHandle,
    OUT     STATUS*                 TerminationStatus
    )
{
    return SyscallEntry(SyscallIdProcessWaitForTermination, ProcessHandle, TerminationStatus);
}

// SyscallIdProcessCloseHandle
STATUS
SyscallProcessCloseHandle(
    IN      UM_HANDLE               ProcessHandle
    )
{
    return SyscallEntry(SyscallIdProcessCloseHandle, ProcessHandle);
}

// SyscallIdVirtualAlloc
STATUS
SyscallVirtualAlloc(
    IN_OPT      PVOID                   BaseAddress,
    IN          QWORD                   Size,
    IN          VMM_ALLOC_TYPE          AllocType,
    IN          PAGE_RIGHTS             PageRights,
    IN_OPT      UM_HANDLE               FileHandle,
    IN_OPT      QWORD                   Key,
    OUT         PVOID*                  AllocatedAddress
    )
{
    return SyscallEntry(SyscallIdVirtualAlloc, BaseAddress, Size, AllocType, PageRights, FileHandle, Key, AllocatedAddress);
}

// SyscallIdVirtualFree
STATUS
SyscallVirtualFree(
    IN          PVOID                   Address,
    _When_(VMM_FREE_TYPE_RELEASE == FreeType, _Reserved_)
    _When_(VMM_FREE_TYPE_RELEASE != FreeType, IN)
                QWORD                   Size,
    IN          VMM_FREE_TYPE           FreeType
    )
{
    return SyscallEntry(SyscallIdVirtualFree, Address, Size, FreeType);
}

// SyscallIdFileCreate
STATUS
SyscallFileCreate(
    IN_READS_Z(PathLength)
                char*                   Path,
    IN          QWORD                   PathLength,
    IN          BOOLEAN                 Directory,
    IN          BOOLEAN                 Create,
    OUT         UM_HANDLE*              FileHandle
    )
{
    return SyscallEntry(SyscallIdFileCreate, Path, PathLength, Directory, Create, FileHandle);
}

// SyscallIdFileClose
STATUS
SyscallFileClose(
    IN          UM_HANDLE               FileHandle
    )
{
    return SyscallEntry(SyscallIdFileClose, FileHandle);
}

// SyscallIdFileRead
STATUS
SyscallFileRead(
    IN  UM_HANDLE                   FileHandle,
    OUT_WRITES_BYTES(BytesToRead)
        PVOID                       Buffer,
    IN  QWORD                       BytesToRead,
    OUT QWORD*                      BytesRead
    )
{
    return SyscallEntry(SyscallIdFileRead, FileHandle, Buffer, BytesToRead, BytesRead);
}

// SyscallIdFileWrite
STATUS
SyscallFileWrite(
    IN  UM_HANDLE                   FileHandle,
    IN_READS_BYTES(BytesToWrite)
        PVOID                       Buffer,
    IN  QWORD                       BytesToWrite,
    OUT QWORD*                      BytesWritten
    )
{
    return SyscallEntry(SyscallIdFileWrite, FileHandle, Buffer, BytesToWrite, BytesWritten);
}
