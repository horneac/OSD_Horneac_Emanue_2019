#pragma once

#define IO_APIC_IDENTIFICATION_REGISTER_OFFSET          0x0
#define IO_APIC_VERSION_REGISTER_OFFSET                 0x1
#define IO_APIC_REDIRECTION_TABLE_BASE_OFFSET           0x10
#define IO_APIC_REDIRECTION_TABLE_ENTRY_SIZE            0x2

#define PREDEFINED_IO_APIC_REGISTER_SIZE                0x10
#define PREDEFINED_IO_APIC_USABLE_REGISTER_SIZE         0x4

#define PREDEFINED_IO_APIC_TABLE_REDIR_ENTRY_SIZE       0x8

#pragma pack(push,1)

#pragma warning(push)

// warning C4214: nonstandard extension used: bit field types other than int
#pragma warning(disable:4214)

// warning C4201: nonstandard extension used: nameless struct/union
#pragma warning(disable:4201)
typedef struct _IO_APIC_REGISTER
{
    volatile DWORD          Value;
    DWORD                   __Reserved[3];
} IO_APIC_REGISTER, *PIO_APIC_REGISTER;
STATIC_ASSERT(sizeof(IO_APIC_REGISTER) == PREDEFINED_IO_APIC_REGISTER_SIZE);

typedef union _IO_APIC_REG_SEL
{
    struct
    {
        BYTE                    ApicAddress;
        BYTE                    __Reserved[3];
    };
    DWORD                   Value;
} IO_APIC_REG_SEL, *PIO_APIC_REG_SEL;
STATIC_ASSERT(sizeof(IO_APIC_REG_SEL) == PREDEFINED_IO_APIC_USABLE_REGISTER_SIZE);

typedef union _IO_APIC_ID_REGISTER
{
    struct
    {
        BYTE                __Reserved0[3];

        BYTE                ApicId                      :  4;
        BYTE                __Reserved1                 :  4;
    };
    DWORD                   Value;
} IO_APIC_ID_REGISTER, *PIO_APIC_ID_REGISTER;
STATIC_ASSERT(sizeof(IO_APIC_ID_REGISTER) == PREDEFINED_IO_APIC_USABLE_REGISTER_SIZE);

typedef union _IO_APIC_VERSION_REGISTER
{
    struct
    {
        BYTE                ApicVersion;

        BYTE                __Reserved0;

        BYTE                MaximumRedirectionEntry;
        BYTE                __Reserved1;
    };
    DWORD                   Value;
} IO_APIC_VERSION_REGISTER, *PIO_APIC_VERSION_REGISTER;
STATIC_ASSERT(sizeof(IO_APIC_VERSION_REGISTER) == PREDEFINED_IO_APIC_USABLE_REGISTER_SIZE);

typedef union _IO_APIC_REDIR_TABLE_ENTRY
{
    struct
    {
        // Interrupt Vector(INTVEC)—R / W: The vector field is an 8 bit field containing the interrupt
        // vector for this interrupt.Vector values range from 10h to FEh.
        QWORD                   Vector                  :  8;

        // Delivery Mode(DELMOD)—R / W.The Delivery Mode is a 3 bit field that specifies how the
        // APICs listed in the destination field should act upon reception of this signal.Note that certain
        // Delivery Modes only operate as intended when used in conjunction with a specific trigger Mode.
        // These restrictions are indicated in the following table for each Delivery Mode.

        // Bits
        // [10:8] Mode Description
        // 000 Fixed    Deliver the signal on the INTR signal of all processor cores listed in the
        //              destination.Trigger Mode for "fixed" Delivery Mode can be edge or level.
        // 001 Lowest
        // Priority     Deliver the signal on the INTR signal of the processor core that is
        //              executing at the lowest priority among all the processors listed in the
        //              specified destination.Trigger Mode for "lowest priority".Delivery Mode
        //              can be edge or level.
        // 010 SMI      System Management Interrupt.A delivery mode equal to SMI requires an
        //              edge trigger mode.The vector information is ignored but must be
        //              programmed to all zeroes for future compatibility.
        // 011 Reserved
        // 100          NMI Deliver the signal on the NMI signal of all processor cores listed in the
        //              destination.Vector information is ignored.NMI is treated as an edge
        //              triggered interrupt, even if it is programmed as a level triggered interrupt.
        //              For proper operation, this redirection table entry must be programmed to
        //              “edge” triggered interrupt.
        // 101 INIT     Deliver the signal to all processor cores listed in the destination by
        //              asserting the INIT signal.All addressed local APICs will assume their
        //              INIT state.INIT is always treated as an edge triggered interrupt, even if
        //              programmed otherwise.For proper operation, this redirection table entry
        //              must be programmed to “edge” triggered interrupt.
        // 110 Reserved
        // 111 ExtINT   Deliver the signal to the INTR signal of all processor cores listed in the
        //              destination as an interrupt that originated in an externally connected
        //              (8259A - compatible) interrupt controller.The INTA cycle that corresponds
        //              to this ExtINT delivery is routed to the external controller that is expected
        //              to supply the vector.A Delivery Mode of "ExtINT" requires an edge
        //              trigger mode.
        QWORD                   DeliveryMode            :  3;

        // Destination Mode(DESTMOD)—R / W.This field determines the interpretation of the
        // Destination field.When DESTMOD = 0 (physical mode), a destination APIC is identified by its ID.
        // Bits 56 through 59 of the Destination field specify the 4 bit APIC ID.When DESTMOD = 1 (logical
        // mode), destinations are identified by matching on the logical destination under the control of the
        // Destination Format Register and Logical Destination Register in each Local APIC.

        // Destination Mode IOREDTBLx[11] Logical Destination Address
        // 0, Physical Mode IOREDTBLx[59:56] = APIC ID
        // 1, Logical Mode IOREDTBLx[63:56] = Set of processors
        QWORD                   DestinationMode         :  1;

        // Delivery Status(DELIVS)—RO.The Delivery Status bit contains the current status of the
        // delivery of this interrupt.Delivery Status is read - only and writes to this bit(as part of a 32 bit
        // word) do not effect this bit. 0 = IDLE(there is currently no activity for this interrupt). 1 = Send
        // Pending(the interrupt has been injected but its delivery is temporarily held up due to the APIC
        // bus being busy or the inability of the receiving APIC unit to accept that interrupt at that time).
        QWORD                   DeliveryStatus          :  1;

        // Interrupt Input Pin Polarity(INTPOL)—R / W.This bit specifies the polarity of the interrupt
        // signal. 0 = High active, 1 = Low active.
        QWORD                   PinPolarity             :  1;

        // Remote IRR—RO. This bit is used for level triggered interrupts. Its meaning is undefined for
        // edge triggered interrupts. For level triggered interrupts, this bit is set to 1 when local APIC(s)
        // accept the level interrupt sent by the IOAPIC. The Remote IRR bit is set to 0 when an EOI
        // message with a matching interrupt vector is received from a local APIC.
        QWORD                   RemoteIRR               :  1;

        // Trigger Mode—R / W.The trigger mode field indicates the type of signal on the interrupt pin that
        // triggers an interrupt. 1 = Level sensitive, 0 = Edge sensitive.
        QWORD                   TriggerMode             :  1;

        // Interrupt Mask—R / W.When this bit is 1, the interrupt signal is masked.Edge - sensitive
        // interrupts signaled on a masked interrupt pin are ignored(i.e., not delivered or held pending).
        // Level - asserts or negates occurring on a masked level - sensitive pin are also ignored and have no
        // side effects.Changing the mask bit from unmasked to masked after the interrupt is accepted by
        // a local APIC has no effect on that interrupt.This behavior is identical to the case where the
        // device withdraws the interrupt before that interrupt is posted to the processor.It is software's
        // responsibility to handle the case where the mask bit is set after the interrupt message has been
        // accepted by a local APIC unit but before the interrupt is dispensed to the processor.When this
        // bit is 0, the interrupt is not masked.An edge or level on an interrupt pin that is not masked
        // results in the delivery of the interrupt to the destination.
        QWORD                   Masked                  :  1;

        QWORD                   __Reserved0             : 15;

        QWORD                   __Reserved1             : 24;


        // Destination Field—R/W. If the Destination Mode of this entry is Physical Mode(bit 11 = 0), bits
        // [59:56] contain an APIC ID.If Logical Mode is selected(bit 11 = 1), the Destination Field
        // potentially defines a set of processors.Bits[63:56] of the Destination Field specify the logical
        // destination address.

        // Destination Mode IOREDTBLx[11] Logical Destination Address
        // 0, Physical Mode IOREDTBLx[59:56] = APIC ID
        // 1, Logical Mode IOREDTBLx[63:56] = Set of processors
        QWORD                   Destination             :  8;
    };
    struct
    {
        DWORD                   LowDword;
        DWORD                   HighDword;
    };
} IO_APIC_REDIR_TABLE_ENTRY, *PIO_APIC_REDIR_TABLE_ENTRY;
STATIC_ASSERT(sizeof(IO_APIC_REDIR_TABLE_ENTRY) == PREDEFINED_IO_APIC_TABLE_REDIR_ENTRY_SIZE );

typedef struct _IO_APIC
{
    IO_APIC_REGISTER        IoRegSel;

    IO_APIC_REGISTER        IoRegData;

    IO_APIC_REGISTER        __Reserved0[2];

    // valid only if version >= 0x20
    // see: http://lxr.free-electrons.com/source/arch/x86/kernel/apic/io_apic.c
    IO_APIC_REGISTER        IoRegEOI;
} IO_APIC, *PIO_APIC;

#pragma warning(pop)
#pragma pack(pop)