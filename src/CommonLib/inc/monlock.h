#pragma once

C_HEADER_START
#define MONITOR_FILTER_SIZE     64

#pragma pack(push,16)
#pragma warning(push)

// warning C4201: nonstandard extension used: nameless struct/union
#pragma warning(disable:4201)
typedef struct _MONITOR_LOCK
{
    union
    {
        SPINLOCK                Lock;
        volatile BYTE           Reserved[MONITOR_FILTER_SIZE];
    };
} MONITOR_LOCK, *PMONITOR_LOCK;

#pragma warning(pop)
#pragma pack(pop)

void
MonitorLockInit(
    OUT         PMONITOR_LOCK       Lock
    );

void
MonitorLockAcquire(
    INOUT       PMONITOR_LOCK       Lock,
    OUT         INTR_STATE*         IntrState
    );

BOOL_SUCCESS
BOOLEAN
MonitorLockTryAcquire(
    INOUT       PMONITOR_LOCK       Lock,
    OUT         INTR_STATE*         IntrState
    );

BOOLEAN
MonitorLockIsOwner(
    IN          PMONITOR_LOCK       Lock
    );

void
MonitorLockRelease(
    INOUT       PMONITOR_LOCK       Lock,
    IN          INTR_STATE          OldIntrState
    );
C_HEADER_END
