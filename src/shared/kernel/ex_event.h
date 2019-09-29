#pragma once

#include "list.h"
#include "lock_common.h"

typedef enum _EX_EVT_TYPE
{
    ExEventTypeNotification,        // notifies all threads
    ExEventTypeSynchronization,     // notifies only one thread

    ExEventTypeReserved
} EX_EVT_TYPE;

typedef struct _EX_EVENT
{
    LOCK                EventLock;
    LIST_ENTRY          WaitingList;
    EX_EVT_TYPE         EventType;
    volatile BYTE       Signaled;
} EX_EVENT, *PEX_EVENT;

//******************************************************************************
// Function:     ExEventInit
// Description:  Initializes an executive event. As in the case of primitive
//               events, these may be notification or synchronization events.
// Returns:      STATUS
// Parameter:    OUT EX_EVENT * Event
// Parameter:    IN EX_EVT_TYPE EventType
// Parameter:    IN BOOLEAN Signaled
//******************************************************************************
STATUS
ExEventInit(
    OUT     EX_EVENT*     Event,
    IN      EX_EVT_TYPE   EventType,
    IN      BOOLEAN       Signaled
    );

//******************************************************************************
// Function:     ExEventSignal
// Description:  Signals an event. If the waiting list is not empty it will
//               wakeup one or multiple threads depending on the event type.
// Returns:      void
// Parameter:    INOUT EX_EVENT * Event
//******************************************************************************
void
ExEventSignal(
    INOUT   EX_EVENT*      Event
    );

//******************************************************************************
// Function:     ExEventClearSignal
// Description:  Clears an event signal.
// Returns:      void
// Parameter:    INOUT EX_EVENT * Event
//******************************************************************************
void
ExEventClearSignal(
    INOUT   EX_EVENT*      Event
    );

//******************************************************************************
// Function:     ExEventWaitForSignal
// Description:  Waits for an event to be signaled. If the event is not signaled
//               the calling thread will be placed in a waiting list and its
//               execution will be blocked.
// Returns:      void
// Parameter:    INOUT EX_EVENT * Event
//******************************************************************************
void
ExEventWaitForSignal(
    INOUT   EX_EVENT*      Event
    );
