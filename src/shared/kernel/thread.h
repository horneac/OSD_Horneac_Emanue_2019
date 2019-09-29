#pragma once

#include "thread_defs.h"

//******************************************************************************
// Function:     ThreadCreate
// Description:  Spawns a new thread named Name with priority Function which
//               will execute the function Function which will receive as its
//               single parameter Context. The function returns a pointer
//               (handle) to the thread structure.
// Returns:      STATUS
// Parameter:    IN_Z char * Name
// Parameter:    IN THREAD_PRIORITY Priority
// Parameter:    IN PFUNC_ThreadStart Function
// Parameter:    IN_OPT PVOID Context
// Parameter:    OUT_PTR PTHREAD * Thread
// NOTE:         The thread may terminate at any time, but its data structure
//               will not be un-allocated until the handle receive in Thread is
//               closed with ThreadCloseHandle.
//******************************************************************************
STATUS
ThreadCreate(
    IN_Z        char*               Name,
    IN          THREAD_PRIORITY     Priority,
    IN          PFUNC_ThreadStart   Function,
    IN_OPT      PVOID               Context,
    OUT_PTR     PTHREAD*            Thread
    );

//******************************************************************************
// Function:     ThreadYield
// Description:  Yields the CPU to the scheduler, which picks a new thread to
//               run. The new thread might be the current thread, so you can't
//               depend on this function to keep this thread from running for
//               any particular length of time.
// Returns:      void
// Parameter:    void
//******************************************************************************
void
ThreadYield(
    void
    );

//******************************************************************************
// Function:     ThreadExit
// Description:  Causes the current thread to exit. Never returns.
// Returns:      void
// Parameter:    IN STATUS ExitStatus
//******************************************************************************
void
ThreadExit(
    IN      STATUS              ExitStatus
    );

//******************************************************************************
// Function:     ThreadWaitForTermination
// Description:  Waits for a thread to terminate. The exit status of the thread
//               will be placed in ExitStatus.
// Returns:      void
// Parameter:    IN PTHREAD Thread
// Parameter:    OUT STATUS * ExitStatus
//******************************************************************************
void
ThreadWaitForTermination(
    IN      PTHREAD             Thread,
    OUT     STATUS*             ExitStatus
    );

//******************************************************************************
// Function:     ThreadCloseHandle
// Description:  Closes a thread handle received from ThreadCreate. This is
//               necessary for the structure to be destroyed when it is no
//               longer needed.
// Returns:      void
// Parameter:    INOUT PTHREAD Thread
// NOTE:         If you need to wait for a thread to terminate or find out its
//               termination status call this function only after you called
//               ThreadWaitForTermination.
//******************************************************************************
void
ThreadCloseHandle(
    INOUT   PTHREAD             Thread
    );

//******************************************************************************
// Function:     ThreadGetName
// Description:  Returns the thread's name.
// Returns:      const char*
// Parameter:    IN_OPT PTHREAD Thread If NULL returns the name of the
//               current thread.
//******************************************************************************
const
char*
ThreadGetName(
    IN_OPT  PTHREAD             Thread
    );

//******************************************************************************
// Function:     ThreadGetId
// Description:  Returns the thread's ID.
// Returns:      TID
// Parameter:    IN_OPT PTHREAD Thread - If NULL returns the ID of the
//               current thread.
//******************************************************************************
TID
ThreadGetId(
    IN_OPT  PTHREAD             Thread
    );

//******************************************************************************
// Function:     ThreadGetPriority
// Description:  Returns the thread's priority. In the presence of
//               priority donation, returns the higher(donated) priority.
// Returns:      THREAD_PRIORITY
// Parameter:    IN_OPT PTHREAD Thread - If NULL returns the priority of the
//               current thread.
//******************************************************************************
THREAD_PRIORITY
ThreadGetPriority(
    IN_OPT  PTHREAD             Thread
    );
