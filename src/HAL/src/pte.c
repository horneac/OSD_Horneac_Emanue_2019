#include "hal_base.h"
#include "pte.h"

void
PteMap(
    IN          PVOID               PageTable,
    IN_OPT      PHYSICAL_ADDRESS    PhysicalAddress,
    IN          PTE_MAP_FLAGS       Flags
    )
{
    PT_ENTRY* pTablePointer;

    ASSERT(NULL != PageTable);

    pTablePointer = PageTable;
    memzero(pTablePointer, sizeof(PVOID));

    pTablePointer->PhysicalAddress = (QWORD) PhysicalAddress >> SHIFT_FOR_PHYSICAL_ADDR;
    pTablePointer->Present = 1;

    pTablePointer->ReadWrite = Flags.Writable;
    pTablePointer->XD = !Flags.Executable;

    // 0 means user-mode accesses are forbidden
    pTablePointer->UserSupervisor = Flags.UserAccess;

    if (!Flags.PagingStructure)
    {
        // set caching
        pTablePointer->PAT = (Flags.PatIndex >> 2) & 1;
        pTablePointer->PCD = (Flags.PatIndex >> 1) & 1;
        pTablePointer->PWT = (Flags.PatIndex >> 0) & 1;

        pTablePointer->Global = Flags.GlobalPage;
    }
}

void
PteUnmap(
    IN          PVOID           PageTable
    )
{
    ASSERT( NULL != PageTable );
    
    memzero(PageTable, sizeof(PT_ENTRY));
}

PHYSICAL_ADDRESS
PteGetPhysicalAddress(
    IN          PVOID           PageTable
    )
{
    PPT_ENTRY pEntry;

    ASSERT( NULL != PageTable );

    pEntry = (PPT_ENTRY) PageTable;

    return (PHYSICAL_ADDRESS) ( pEntry->PhysicalAddress << SHIFT_FOR_PHYSICAL_ADDR );
}

PHYSICAL_ADDRESS
PteLargePageGetPhysicalAddress(
    IN          PVOID           PageTable
    )
{
    PPD_ENTRY_2MB pEntry;

    ASSERT(NULL != PageTable);

    pEntry = (PPD_ENTRY_2MB)PageTable;

    return (PHYSICAL_ADDRESS)(pEntry->PhysicalAddress << SHIFT_FOR_LARGE_PAGE);

}

BOOLEAN
PteIsPresent(
    IN          PVOID           PageTable
    )
{
    PML4_ENTRY* pTablePointer;

    ASSERT( NULL != PageTable );

    pTablePointer = PageTable;

    return ( 1== pTablePointer->Present );
}