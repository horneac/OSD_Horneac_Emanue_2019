#include "hal_base.h"
#include "cpu.h"
#include "idt.h"
#include "tss.h"

#define PREDEFINED_IDT_SIZE                     10
#define PREDEFINED_IDT_ENTRY_SIZE               16

#pragma pack(push,1)

#pragma warning(push)
//warning C4214: nonstandard extension used : bit field types other than int
#pragma warning(disable:4214)
// 6.14.1, Vol 3, No. 56
typedef struct _IDT_ENTRY
{
    // 15:0
    WORD            LowWordOffset;              // Bits 15:0 of address

                                                // 31:16
    WORD            SegmentSelector;

    // 34:32
    // IST = Interrupt Stack Table
    // if set to 0 will use legacy stack switching
    // else will use IST entry from TSS
    WORD            IST                 : 3;
    WORD            Reserved0           : 5;    // these must be 0 on x64
    WORD            Type                : 4;
    WORD            Reserved1           : 1;    // 0
    WORD            DPL                 : 2;
    WORD            Present             : 1;
    WORD            HighWordOffset;             // Bits 31:16 of address
    DWORD           HighestDwordOffset;         // Bits 63:32 of address
    DWORD           Reserved;
} IDT_ENTRY, *PIDT_ENTRY;
STATIC_ASSERT( sizeof( IDT_ENTRY ) == PREDEFINED_IDT_ENTRY_SIZE );
#pragma warning(pop)

typedef struct _IDT
{
    // IDTR(Limit) <-- SRC[0:15];
    WORD            Limit;

    // IDTR(Base)  <-- SRC[16:79];
    IDT_ENTRY*      Base;
} IDT, *PIDT;
STATIC_ASSERT( sizeof( IDT ) == PREDEFINED_IDT_SIZE );
#pragma pack(pop)

__declspec(align(NATURAL_ALIGNMENT))
static IDT_ENTRY    m_idtDescriptors[NO_OF_TOTAL_INTERRUPTS];
static IDT          m_idt;

void
IdtInitialize(
    void
    )
{
    m_idt.Limit = sizeof(IDT_ENTRY) * NO_OF_TOTAL_INTERRUPTS - 1;
    m_idt.Base = m_idtDescriptors;

    // zero everything
    memzero(m_idt.Base, m_idt.Limit + 1);

    // install IDT
    IdtReload();
}

STATUS
IdtInstallDescriptor(
    IN                  BYTE            InterruptIndex,
    IN                  WORD            CodeSelector,
    IN                  BYTE            GateType,
    IN                  BYTE            InterruptStackIndex,
    IN                  BOOLEAN         Present,
    _When_(Present, IN)
    _When_(!Present, IN_OPT)
    PVOID           HandlerAddress
    )
{
    DWORD lowAddress;
    IDT_ENTRY* pDescriptor;

    if (Present)
    {
        // we're talking about a real descriptor, we need to check the other fields
        if ((SystemSegment64BitTaskGate > GateType) || (SystemSegment64BitTrapGate < GateType))
        {
            // invalid descriptor
            return STATUS_INVALID_PARAMETER3;
        }

        if (NO_OF_IST < InterruptStackIndex)
        {
            return STATUS_INVALID_PARAMETER4;
        }

        if (NULL == HandlerAddress)
        {
            return STATUS_INVALID_PARAMETER6;
        }
    }

    pDescriptor = &m_idt.Base[InterruptIndex];

    lowAddress = QWORD_LOW(HandlerAddress);

    // set all fields to 0
    memzero(pDescriptor, sizeof(IDT_ENTRY));

    pDescriptor->HighestDwordOffset = QWORD_HIGH(HandlerAddress);
    pDescriptor->HighWordOffset = DWORD_HIGH(lowAddress);
    pDescriptor->Present = (WORD)Present;
    pDescriptor->DPL = RING_ZERO_PL;
    pDescriptor->Type = GateType;
    pDescriptor->IST = InterruptStackIndex;
    pDescriptor->SegmentSelector = CodeSelector;
    pDescriptor->LowWordOffset = DWORD_LOW(lowAddress);

    return STATUS_SUCCESS;
}

void
IdtReload(
    void
    )
{
    __lidt(&m_idt);
}