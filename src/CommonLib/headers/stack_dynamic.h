#pragma once

#include "stack_interface.h"
#include "stack_internal.h"

// public
FUNC_StackPush              StackDynamicPush;
FUNC_StackPop               StackDynamicPop;
FUNC_StackPeek              StackDynamicPeek;
FUNC_StackClear             StackDynamicClear;
FUNC_StackIsEmpty           StackDynamicIsEmpty;
FUNC_StackSize              StackDynamicSize;

// private
FUNC_StackGetRequiredSize   StackDynamicGetRequiredSize;
FUNC_StackInit              StackDynamicInit;
