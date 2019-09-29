#include "hal_base.h"
#include "hal.h"
#include "idt.h"
#include "gdt.h"

void
HalInitialize(
    void
    )
{
    GdtInitialize();

    IdtInitialize();
}