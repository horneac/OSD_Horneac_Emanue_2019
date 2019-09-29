#pragma once

_No_competing_thread_
void
NetworkStackPreinit(
    void
    );

_No_competing_thread_
STATUS
NetworkStackInit(
    IN  BOOLEAN     EnableNetworking
    );

_No_competing_thread_
STATUS
NetworkStackSetState(
    IN  BOOLEAN     EnableNetworking
    );