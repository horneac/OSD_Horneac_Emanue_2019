#pragma once

C_HEADER_START
typedef enum _EVENT_TYPE
{
    EventTypeNotification,
    EventTypeSynchronization,

    EventTypeReserved
} EVENT_TYPE, *PEVENT_TYPE;

#pragma warning(push)

// nonstandard extension used : nameless struct/union
#pragma warning(disable:4201)
typedef struct _EVENT
{
    volatile BYTE       State;
    EVENT_TYPE          EventType;
} EVENT, *PEVENT;
#pragma warning(pop)

//******************************************************************************
// Function:     EvtInitialize
// Description:  Creates an EVENT object. The behavior differs depending on the
//               type of event:
//               -> EventTypeNotification: Once an event is signaled it remains
//               signaled until it is manually cleared.
//               -> EventTypeSynchronization: Once an event is signaled, the
//               first CPU which will wait detect the signal in EvtWaitForSignal
//               will also clear it, i.e. a single CPU acknowledges the event,
//               whereas the notification case all the CPUs acknowledge it.
// Returns:      STATUS
// Parameter:    OUT EVENT * Event
// Parameter:    IN EVENT_TYPE EventType
// Parameter:    IN BOOLEAN Signaled
//******************************************************************************
STATUS
EvtInitialize(
    OUT     EVENT*          Event,
    IN      EVENT_TYPE      EventType,
    IN      BOOLEAN         Signaled
    );

//******************************************************************************
// Function:     EvtSignal
// Description:  Signals an event.
// Returns:      void
// Parameter:    INOUT EVENT * Event
//******************************************************************************
void
EvtSignal(
    INOUT   EVENT*          Event
    );

//******************************************************************************
// Function:     EvtClearSignal
// Description:  Clears an event signal.
// Returns:      void
// Parameter:    INOUT EVENT * Event
//******************************************************************************
void
EvtClearSignal(
    INOUT   EVENT*          Event
    );

//******************************************************************************
// Function:     EvtWaitForSignal
// Description:  Busy waits until an event is signaled.
// Returns:      void
// Parameter:    INOUT EVENT * Event
//******************************************************************************
void
EvtWaitForSignal(
    INOUT   EVENT*          Event
    );

//******************************************************************************
// Function:     EvtIsSignaled
// Description:  Checks if an event is signaled and returns instantly.
// Returns:      BOOLEAN - TRUE if the event was signaled, FALSE otherwise.
// Parameter:    INOUT EVENT * Event
//******************************************************************************
BOOLEAN
EvtIsSignaled(
    INOUT   EVENT*          Event
    );
C_HEADER_END
