#pragma once

C_HEADER_START
void
GSNotifyStackChange(
    IN  PVOID       OldStackBase,
    IN  PVOID       NewStackBase,
    IN  DWORD       StackSize
    );
C_HEADER_END
