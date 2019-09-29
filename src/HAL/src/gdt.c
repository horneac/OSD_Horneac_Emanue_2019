#include "hal_base.h"
#include "gdt.h"
#include "tss.h"

#define GDT_MAXIMUM_DESCRIPTORS                   512

typedef
void
(__cdecl FUNC_ReloadGDT) (
    IN      PGDT        NewGdt,
    IN      WORD        CsSelector,
    IN      WORD        DsSelector
    );

extern FUNC_ReloadGDT           __reloadGDT;


// the size of the GDT is 4K
__declspec(align(NATURAL_ALIGNMENT))
static SEGMENT_DESCRIPTOR       m_gdtDescriptors[GDT_MAXIMUM_DESCRIPTORS];
static GDT                      m_gdt;

STATUS
GdtInitialize(
    void
    )
{
    m_gdt.Limit = ( sizeof(SEGMENT_DESCRIPTOR) * GDT_MAXIMUM_DESCRIPTORS ) - 1;
    m_gdt.Base = m_gdtDescriptors;

    // set all descriptors to NULL
    memzero(m_gdt.Base, m_gdt.Limit + 1);

    return STATUS_SUCCESS;
}

STATUS
GdtInstallDescriptor(
    IN          WORD                        GdtIndex,
    IN          QWORD                       Base,
    IN          BYTE                        Type,
    IN          SEGMENT_DESCRIPTOR_FLAGS    Flags
    )
{
    BYTE descriptorSize;
    BOOLEAN tssDescriptor;
    PSEGMENT_DESCRIPTOR pSegDesc;
    DWORD segmentLimit;
    DWORD lowDword;
    WORD highWord;
    BOOLEAN sysDescriptor;

    // check if system descriptor
    sysDescriptor = IsBitSet(Flags, SegmentDescriptorSystemDescriptor);

    tssDescriptor = sysDescriptor && ((SystemSegment64BitTssAvailable == Type) || (SystemSegment64BitTssBusy == Type));

    if (sysDescriptor && !tssDescriptor)
    {
        // we can't install other system descriptors except TSS ones
        return STATUS_INVALID_PARAMETER3;
    }

    // normal descriptors occupy only 8 bytes, while a TSS occupies 16
    descriptorSize = tssDescriptor ? sizeof(TSS_DESCRIPTOR) : sizeof(SEGMENT_DESCRIPTOR);
    pSegDesc = NULL;

    // check if within GDT limits
    if (GdtIndex + descriptorSize > m_gdt.Limit)
    {
        return STATUS_INVALID_PARAMETER1;
    }

    // the limit of the TSS is given by its size, while for the rest of the descriptors
    // we use MAX_DWORD
    segmentLimit = tssDescriptor ? sizeof(TSS) : MAX_DWORD;

    if (!tssDescriptor)
    {
        if (Base >= MAX_DWORD)
        {
            return STATUS_INVALID_PARAMETER2;
        }
    }

    // get a pointer to the segment descriptor
    // NOTE: even if it's a TSS descriptor the lower 8 bytes have the common structure
    pSegDesc = (PSEGMENT_DESCRIPTOR)((PBYTE)m_gdt.Base + GdtIndex);
    memzero(pSegDesc, descriptorSize);

    lowDword = QWORD_LOW(Base);
    highWord = DWORD_HIGH(lowDword);

    pSegDesc->SegmentLimitLow = DWORD_LOW(segmentLimit);
    pSegDesc->BaseAddressLow = DWORD_LOW(lowDword);
    pSegDesc->BaseAddressMid = WORD_LOW(highWord);
    pSegDesc->Type = Type;
    pSegDesc->DescriptorType = !sysDescriptor;

    // we use only ring0 and ring3 for descriptors
    pSegDesc->DPL = IsBitSet(Flags, SegmentDescriptorFlagRing0) ? RING_ZERO_PL : RING_THREE_PL;
    pSegDesc->Present = 1;
    pSegDesc->SegmentLimitHigh = (BYTE)DWORD_HIGH(segmentLimit);
    pSegDesc->AVL = 0;
    pSegDesc->L = IsBitSet(Flags, SegmentDescriptorLongMode);
    pSegDesc->G = IsBitSet(Flags, SegmentDescriptor32bitOperation) || IsBitSet(Flags,SegmentDescriptorLongMode);
    pSegDesc->D_B = IsBitSet(Flags, SegmentDescriptor32bitOperation );
    pSegDesc->BaseAddressHigh = WORD_HIGH(highWord);

    if (tssDescriptor)
    {
        // if it is a TSS descriptor we also complete the upper 4 bytes of the address
        ((TSS_DESCRIPTOR*)pSegDesc)->BaseAddressHighQword = QWORD_HIGH(Base);
    }

    return STATUS_SUCCESS;
}

BOOLEAN
GdtIsSegmentPrivileged(
    IN          WORD            Selector
    )
{
    return !(IsFlagOn(Selector, 0b11));
}

void
GdtReload(
    IN          WORD            CodeSelector,
    IN          WORD            DataSelector
    )
{
    __reloadGDT(&m_gdt, CodeSelector, DataSelector);
}