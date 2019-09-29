#pragma once

typedef enum _IRQ
{
    IrqPitTimer,
    IrqKeyboard,
    IrqCascade,
    IrqCOM2,
    IrqCom1,
    IrqReserved0,
    IrqDiskette,
    IrqLPT1,
    IrqSpurious = IrqLPT1,
    IrqRtc,
    IrqCGA,
    IrqReserved1,
    IrqReserved2,
    IrqReserved3,
    IrqFPU,
    IrqHdController
} IRQ;

#define     IRQS_PER_PIC    0x8
#define     NO_OF_IRQS      0x10

void
PicInitialize(
    IN BYTE MasterBase,
    IN BYTE SlaveBase
    );

void
PicSendEOI(
    IN      BYTE        Irq
    );

void
PicChangeIrqMask(
    IN      BYTE        Irq,
    IN      BOOLEAN     MaskIrq
    );

WORD
PicGetIrr(
    void
    );

WORD
PicGetIsr(
    void
    );