#include "hal_base.h"
#include "tss.h"

STATUS
TssInstall(
    OUT     PTSS        Tss,
    IN      WORD        GdtIndex,
    IN_RANGE(1,NO_OF_IST)
            BYTE        NumberOfStacks,
    IN_READS(NumberOfStacks)
            PVOID*      Stacks
    )
{
    STATUS status;
    SEGMENT_DESCRIPTOR_FLAGS flags;

    if (NULL == Tss)
    {
        return STATUS_INVALID_PARAMETER1;
    }

    if ((0 == NumberOfStacks) || ( NumberOfStacks > NO_OF_IST) )
    {
        return STATUS_INVALID_PARAMETER3;
    }

    if( NULL == Stacks )
    {
        return STATUS_INVALID_PARAMETER4;
    }

    status = STATUS_SUCCESS;

    // zero the whole TSS
    memzero(Tss, sizeof(TSS));

    for( DWORD i = 0; i < NumberOfStacks; ++i )
    {
        // setup ISTs
        Tss->IST[i] = (QWORD) Stacks[i];
    }

    // Set the IO Bitmap address at the end of the TSS, as a result all IO port accesses
    // made in user-mode will result in #GP exceptions
    Tss->IOMapBaseAddress = sizeof(TSS);

    // install descriptor in GDT
    flags = (1 << SegmentDescriptorFlagRing0) | (1 << SegmentDescriptorSystemDescriptor);

    status = GdtInstallDescriptor(GdtIndex, (QWORD) Tss, SystemSegment64BitTssAvailable, flags);
    if (!SUCCEEDED(status))
    {
        return status;
    }

    return status;
}