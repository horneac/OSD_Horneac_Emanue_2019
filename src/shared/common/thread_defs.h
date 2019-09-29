#pragma once

typedef QWORD       TID, *PTID;

typedef enum _THREAD_PRIORITY
{
    ThreadPriorityLowest            = 0,
    ThreadPriorityDefault           = 16,
    ThreadPriorityMaximum           = 31,
    ThreadPriorityReserved          = ThreadPriorityMaximum + 1
} THREAD_PRIORITY;

typedef struct _THREAD* PTHREAD;

typedef
STATUS
(__cdecl FUNC_ThreadStart)(
    IN_OPT      PVOID       Context
    );

typedef FUNC_ThreadStart*   PFUNC_ThreadStart;