#pragma once

// SyscallIdIdentifyVersion
//******************************************************************************
// Function:     SyscallValidateInterface
// Description:  Validates that the kernel is at the same system call interface
//               version as the user land counterpart.
// Returns:      STATUS - STATUS_SUCCESS if InterfaceVersion is equal to
//               SYSCALL_IF_VERSION_KM.
// Parameter:    IN SYSCALL_IF_VERSION InterfaceVersion
//******************************************************************************
STATUS
SyscallValidateInterface(
    IN  SYSCALL_IF_VERSION          InterfaceVersion
    );

// SyscallIdThreadExit
//******************************************************************************
// Function:     SyscallThreadExit
// Description:  Causes the executing thread to exit with ExitStatus.
// Returns:      STATUS
// Parameter:    IN STATUS ExitStatus
//******************************************************************************
STATUS
SyscallThreadExit(
    IN      STATUS                  ExitStatus
    );

// SyscallIdThreadCreate
//******************************************************************************
// Function:     SyscallThreadCreate
// Description:  Creates a new thread in the context of the current process to
//               execute StartFunction with context Context.
// Returns:      STATUS
// Parameter:    IN PFUNC_ThreadStart StartFunction
// Parameter:    IN_OPT PVOID Context
// Parameter:    OUT UM_HANDLE* ThreadHandle - user handle to the newly created
//               thread.
//******************************************************************************
STATUS
SyscallThreadCreate(
    IN      PFUNC_ThreadStart       StartFunction,
    IN_OPT  PVOID                   Context,
    OUT     UM_HANDLE*              ThreadHandle
    );

// SyscallIdThreadGetTid
//******************************************************************************
// Function:     SyscallThreadGetTid
// Description:  Returns the thread ID for ThreadHandle. If ThreadHandle is
//               UM_INVALID_HANDLE_VALUE the TID of the current thread is
//               retrieved.
// Returns:      STATUS
// Parameter:    IN_OPT UM_HANDLE ThreadHandle
// Parameter:    OUT TID * ThreadId
//******************************************************************************
STATUS
SyscallThreadGetTid(
    IN_OPT  UM_HANDLE               ThreadHandle,
    OUT     TID*                    ThreadId
    );

// SyscallIdThreadWaitForTermination
//******************************************************************************
// Function:     SyscallThreadWaitForTermination
// Description:  Waits for process ThreadHandle to terminate and returns the
//               termination status in TerminationStatus.
// Returns:      STATUS
// Parameter:    IN UM_HANDLE ThreadHandle
// Parameter:    OUT STATUS * TerminationStatus
//******************************************************************************
STATUS
SyscallThreadWaitForTermination(
    IN      UM_HANDLE               ThreadHandle,
    OUT     STATUS*                 TerminationStatus
    );

// SyscallIdThreadCloseHandle
//******************************************************************************
// Function:     SyscallThreadCloseHandle
// Description:  Closes the handle to ThreadHandle.
// Returns:      STATUS
// Parameter:    IN UM_HANDLE ThreadHandle
//******************************************************************************
STATUS
SyscallThreadCloseHandle(
    IN      UM_HANDLE               ThreadHandle
    );

// SyscallIdProcessExit
//******************************************************************************
// Function:     SyscallProcessExit
// Description:  Terminates the currently executing process with ExitStatus
//               status.
// Returns:      STATUS
// Parameter:    IN STATUS ExitStatus
//******************************************************************************
STATUS
SyscallProcessExit(
    IN      STATUS                  ExitStatus
    );

// SyscallIdProcessCreate
//******************************************************************************
// Function:     SyscallProcessCreate
// Description:  Creates a new process to execute the code from ProcessPath with
//               arguments Arguments.
// Returns:      STATUS
// Parameter:    IN_READS_Z(PathLength) char* ProcessPath - the process path may
//               be a absolute path (starting with X:\) or a relative path to
//               %SYSTEMDRIVE%\APPLIC~1\.
// Parameter:    IN DWORD PathLength
// Parameter:    IN_READS_OPT_Z(ArgLength) char* Arguments - may be NULL
// Parameter:    IN DWORD ArgLength
// Parameter:    OUT UM_HANDLE* ProcessHandle - user handle to the newly created
//               process.
//
// The following usage examples are valid
// Spawn a process to execute the file found at %SYSTEMDRIVE%\APPLIC~1\App.exe
// SyscallProcessCreate("App.exe", sizeof("App.exe"), NULL, 0, &handle);
//
// Spawn a process to execute the file found at D:\Sample.exe
// SyscallProcessCreate("D:\Sample.exe", sizeof("D:\Sample.exe"), NULL, 0, &handle);
//******************************************************************************
STATUS
SyscallProcessCreate(
    IN_READS_Z(PathLength)
                char*               ProcessPath,
    IN          QWORD               PathLength,
    IN_READS_OPT_Z(ArgLength)
                char*               Arguments,
    IN          QWORD               ArgLength,
    OUT         UM_HANDLE*          ProcessHandle
    );

// SyscallIdProcessGetPid
//******************************************************************************
// Function:     SyscallProcessGetPid
// Description:  Retrieves the process identifier for ProcessHandle. If the
//               handle value is UM_INVALID_HANDLE_VALUE the PID for the
//               executing process will be returned.
// Returns:      STATUS
// Parameter:    IN_OPT UM_HANDLE ProcessHandle
// Parameter:    OUT PID * ProcessId
//******************************************************************************
STATUS
SyscallProcessGetPid(
    IN_OPT  UM_HANDLE               ProcessHandle,
    OUT     PID*                    ProcessId
    );

// SyscallIdProcessWaitForTermination
//******************************************************************************
// Function:     SyscallProcessWaitForTermination
// Description:  Waits for process ProcessHandle to terminate and returns the
//               termination status in TerminationStatus.
// Returns:      STATUS
// Parameter:    IN UM_HANDLE ProcessHandle
// Parameter:    OUT STATUS * TerminationStatus
//******************************************************************************
STATUS
SyscallProcessWaitForTermination(
    IN      UM_HANDLE               ProcessHandle,
    OUT     STATUS*                 TerminationStatus
    );

// SyscallIdProcessCloseHandle
//******************************************************************************
// Function:     SyscallProcessCloseHandle
// Description:  Closes the handle to ProcessHandle.
// Returns:      STATUS
// Parameter:    IN UM_HANDLE ProcessHandle
//******************************************************************************
STATUS
SyscallProcessCloseHandle(
    IN      UM_HANDLE               ProcessHandle
    );

// SyscallIdVirtualAlloc
//******************************************************************************
// Function:     SyscallVirtualAlloc
// Description:  Allocates a virtual address range, this can be used for mapping
//               a file in memory by specifying a valid FileHandle. When Key is
//               non-zero this creates or accesses an already existing shared
//               memory range.
// Returns:      STATUS
// Parameter:    IN_OPT PVOID BaseAddress - Provides a hint on where to place
//                                          the virtual allocation.
// Parameter:    IN QWORD Size
// Parameter:    IN VMM_ALLOC_TYPE AllocType
// Parameter:    IN PAGE_RIGHTS PageRights
// Parameter:    IN_OPT UM_HANDLE FileHandle - When non-NULL the range will be
//                                             backed up by a file.
// Parameter:    IN_OPT QWORD Key - When non-zero creates or accesses a system
//                                  wide shared memory region.
// Parameter:    OUT PVOID* AllocatedAddress - The virtual address allocated
//******************************************************************************
STATUS
SyscallVirtualAlloc(
    IN_OPT      PVOID                   BaseAddress,
    IN          QWORD                   Size,
    IN          VMM_ALLOC_TYPE          AllocType,
    IN          PAGE_RIGHTS             PageRights,
    IN_OPT      UM_HANDLE               FileHandle,
    IN_OPT      QWORD                   Key,
    OUT         PVOID*                  AllocatedAddress
    );

// SyscallIdVirtualFree
//******************************************************************************
// Function:     SyscallVirtualFree
// Description:  Frees a previously allocated virtual address range.
// Returns:      STATUS
// Parameter:    IN PVOID Address
// Parameter:    QWORD Size
// Parameter:    IN VMM_FREE_TYPE FreeType
//******************************************************************************
STATUS
SyscallVirtualFree(
    IN          PVOID                   Address,
    _When_(VMM_FREE_TYPE_RELEASE == FreeType, _Reserved_)
    _When_(VMM_FREE_TYPE_RELEASE != FreeType, IN)
                QWORD                   Size,
    IN          VMM_FREE_TYPE           FreeType
    );

// SyscallIdFileCreate
//******************************************************************************
// Function:     SyscallFileCreate
// Description:  Depending on the Create parameter creates a new file or opens
//               an existing one.
// Returns:      STATUS
// Parameter:    IN_READS_Z(PathLength) char* path  the file path may
//               be a absolute path (starting with X:\) or a relative path to
//               %SYSTEMDRIVE%\.
// Parameter:    IN DWORD PathLength
// Parameter:    IN BOOLEAN Directory
// Parameter:    IN BOOLEAN Create
// Parameter:    OUT UM_HANDLE* FileHandle
//
// The following usage examples are valid
// Open the file found at %SYSTEMDRIVE%\myfile
// SyscallFileCreate("myfile", sizeof("myfile"), FALSE, FALSE, &handle);
//
// Create a new directory on partition E:\ named new_dir
// SyscallFileCreate("E:\new_dir", sizeof("E:\new_dir"), TRUE, TRUE, &handle);
//******************************************************************************
STATUS
SyscallFileCreate(
    IN_READS_Z(PathLength)
                char*                   Path,
    IN          QWORD                   PathLength,
    IN          BOOLEAN                 Directory,
    IN          BOOLEAN                 Create,
    OUT         UM_HANDLE*              FileHandle
    );

// SyscallIdFileClose
//******************************************************************************
// Function:     SyscallFileClose
// Description:  Closes a previously opened file.
// Returns:      STATUS
// Parameter:    IN UM_HANDLE FileHandle
//******************************************************************************
STATUS
SyscallFileClose(
    IN          UM_HANDLE               FileHandle
    );

// SyscallIdFileRead
//******************************************************************************
// Function:     SyscallFileRead
// Description:  Reads the content of file FileHandle into Buffer.
// Returns:      STATUS
// Parameter:    IN UM_HANDLE FileHandle
// Parameter:    OUT_WRITES_BYTES(BytesToRead) PVOID Buffer
// Parameter:    IN QWORD BytesToRead - Bytes to read.
// Parameter:    OUT QWORD * BytesRead - Actualy number of bytes read from the
//               file. This value may be lower than BytesToRead if the EOF was
//               detected.
//******************************************************************************
STATUS
SyscallFileRead(
    IN  UM_HANDLE                   FileHandle,
    OUT_WRITES_BYTES(BytesToRead)
        PVOID                       Buffer,
    IN  QWORD                       BytesToRead,
    OUT QWORD*                      BytesRead
    );

// SyscallIdFileWrite
//******************************************************************************
// Function:     SyscallFileWrite
// Description:  Writes the content of Buffer into the FileHandle file.
// Returns:      STATUS
// Parameter:    IN UM_HANDLE FileHandle
// Parameter:    IN_READS_BYTES(BytesToWrite) PVOID Buffer
// Parameter:    IN QWORD BytesToWrite
// Parameter:    OUT QWORD* BytesWritten
//******************************************************************************
STATUS
SyscallFileWrite(
    IN  UM_HANDLE                   FileHandle,
    IN_READS_BYTES(BytesToWrite)
        PVOID                       Buffer,
    IN  QWORD                       BytesToWrite,
    OUT QWORD*                      BytesWritten
    );
