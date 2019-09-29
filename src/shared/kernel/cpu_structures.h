#pragma once

typedef enum _IRQL
{
    IrqlApcLevel                = 0x2,
    IrqlDispatchLevel           = 0x3,

    // device interrupts
    IrqlUserInputLevel          = 0x4, // keyboard, mouse, touchpad, etc...
    IrqlStorageLevel            = 0x5,
    IrqlNetworkLevel            = 0x6,

    // device interrupts end
    IrqlErrorLevel              = 0xC, // used only by spurious interrupts
    IrqlClockLevel              = 0xD,
    IrqlIpiLevel                = 0xE,
    IrqlAssertLevel             = 0xF,
    IrqlMaxLevel                = IrqlAssertLevel
} IRQL;

#define IRQL_TO_VECTOR(irql)    ((BYTE)((irql)<<4))
#define VECTOR_TO_IRQL(vec)     ((IRQL)((vec)>>4))
#define VECTORS_PER_IRQL        0x10