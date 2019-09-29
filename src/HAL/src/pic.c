#include "hal_base.h"
#include "pic.h"

// COMMAND on write | STATUS on read
// DATA on write | IMR on read

#define     PIC1_BASE           0x20
#define     PIC1_COMMAND        PIC1_BASE
#define     PIC1_DATA           (PIC1_BASE+1)

#define     PIC2_BASE           0xA0
#define     PIC2_COMMAND        PIC2_BASE
#define     PIC2_DATA           (PIC2_BASE+1)

#define     PIC_COMMAND_EOI     0x20    

//     If set(1), the PIC expects to receive IC4 during initialization.
#define     ICW1_ICW4           0x01

// If cleared, PIC is cascaded with slave PICs, and ICW3 must be sent to controller.
#define     ICW1_SINGLE         0x02

//  ignored by x86, and is default to 0
#define     ICW1_INTERVAL4      0x04

// If Not set (0), Operate in Edge Triggered Mode
#define     ICW1_LEVEL          0x08

// Initialization bit. Set 1 if PIC is to be initialized
#define     ICW1_INIT           0x10

//     If set (1), it is in 80x86 mode
#define     ICW4_8086           0x01
#define     ICW4_AUTO           0x02        /* Auto (normal) EOI */
#define     ICW4_BUF_SLAVE      0x08        /* Buffered mode/slave */
#define     ICW4_BUF_MASTER     0x0C        /* Buffered mode/master */
#define     ICW4_SFNM           0x10        /* Special fully nested (not) */

// OCW3
#define     PIC_OCW3_READ_IRR   0x0A        // OCW3 irq ready next CMD read
#define     PIC_OCW3_READ_ISR   0x0B        // OCW3 irq service next CMD read    

__forceinline
static
WORD
_PicGetIrqRegister(
    IN      BYTE        Ocw3
    )
{
    __outbyte( PIC1_COMMAND, Ocw3 );
    __outbyte( PIC2_COMMAND, Ocw3 );

    return BYTES_TO_WORD(__inbyte(PIC2_COMMAND), __inbyte(PIC1_COMMAND) );
}

void
PicInitialize(
    IN BYTE MasterBase,
    IN BYTE SlaveBase
    )
{
    // ICW1 - This is the primary control word used to initialize the PIC

    // starts the initialization sequence (in cascade mode)
    __outbyte(PIC1_COMMAND, ICW1_INIT | ICW1_ICW4);  

    // must be sent to both PICs
    __outbyte(PIC2_COMMAND, ICW1_INIT | ICW1_ICW4);


    // ICW2 - This control word is used to map the base address of the IDT of which the PIC are to use

    // Starting offset for PIC1
    __outbyte(PIC1_DATA, MasterBase);

    // Starting offset for PIC2
    __outbyte(PIC2_DATA, SlaveBase);

    
    // ICW3 - let the PICs know what IRQ lines to use when communicating with each other

    // ICW3: tell Master PIC that there is a slave PIC at IRQ2 (0000 0100)
    // We must send the bit which corresponds to IRQ2 (1<<2)
    __outbyte(PIC1_DATA, 1 << 2);                       
    
    // ICW3: tell Slave PIC its cascade identity (0000 0010)
    // For some reason on the secondary PIC we need to tell it the value instead of the bit :)
    __outbyte(PIC2_DATA, 2);                       


    // ICW4 - This controls how everything is to operate
    __outbyte(PIC1_DATA, ICW4_8086);
    
    __outbyte(PIC2_DATA, ICW4_8086);
    
    // mask all interrupts, we will be using the IOAPIC
    __outbyte(PIC1_DATA, MAX_BYTE);
    __outbyte(PIC2_DATA, MAX_BYTE);

    // Intel MP specification Section 3.6.2.1 PIC Mode
    // Before entering Symmetric I/O Mode, either the BIOS or the operating system must switch out of
    // PIC Mode by changing the IMCR.
    // To access the IMCR, write a value of 70h to I / O port 22h, which
    // selects the IMCR.Then write the data to I / O port 23h.The power - on default value is zero, which
    // connects the NMI and 8259 INTR lines directly to the BSP.Writing a value of 01h forces the
    // NMI and 8259 INTR signals to pass through the APIC.

    /// Intel MP specification Section B.3 Interrupt Mode Initialization and Handling
    /// The operating system should switch over to Symmetric I / O Mode to start multiprocessor operation.
    /// If the IMCRP bit of the MP feature information bytes is set, the operating system must set the
    /// IMCR to APIC mode.The operating system should not write to the IMCR unless the IMCRP bit
    /// is set7

    // Select IMCR
    __outbyte( 0x22, 0x70 );

    // Write 0x1
    __outbyte( 0x23, 0x1 );
}

void
PicSendEOI(
    IN      BYTE        Irq
    )
{
    WORD picCommand;

    ASSERT(Irq < 2 * IRQS_PER_PIC);

    picCommand = Irq >= IRQS_PER_PIC ? PIC2_COMMAND : PIC1_COMMAND;

    __outbyte(picCommand, PIC_COMMAND_EOI);
}

void
PicChangeIrqMask(
    IN      BYTE        Irq,
    IN      BOOLEAN     MaskIrq
    )
{
    WORD picData;
    BYTE value;
    BYTE irqLine;
    BYTE irqShift;

    ASSERT(Irq < 2 * IRQS_PER_PIC);

    irqLine = Irq;
    irqShift = 0;

    if (Irq < IRQS_PER_PIC)
    {
        picData = PIC1_DATA;
    }
    else
    {
        picData = PIC2_DATA;
        irqLine = irqLine - IRQS_PER_PIC;
    }

    irqShift = (1 << irqLine);

    value = __inbyte(picData);
    if (MaskIrq)
    {
        value = value | irqShift;
    }
    else
    {
        // we want to clear it => enable interrupt for it
        value = value & (~irqShift);
    }

    __outbyte(picData, value);
}

WORD
PicGetIrr(
    void
    )
{
    return _PicGetIrqRegister(PIC_OCW3_READ_IRR);
}

WORD
PicGetIsr(
    void
    )
{
    return _PicGetIrqRegister(PIC_OCW3_READ_ISR);
}