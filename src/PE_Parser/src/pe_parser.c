#include "pe_base.h"
#include "pe_parser.h"
#include "pe_structures.h"

STATUS
PeRetrieveNtHeader(
    IN_READS_BYTES(ImageSize)           PVOID                   ImageBase,
    IN                                  DWORD                   ImageSize,
    OUT                                 PPE_NT_HEADER_INFO      NtInfo
    )
{
    STATUS status;
    IMAGE_DOS_HEADER* pDos;
    IMAGE_NT_HEADERS64* nt;

    if (NULL == ImageBase)
    {
        return STATUS_INVALID_PARAMETER1;
    }

    if (0 == ImageSize)
    {
        return STATUS_INVALID_PARAMETER2;
    }

    if (NULL == NtInfo)
    {
        return STATUS_INVALID_PARAMETER3;
    }

    status = STATUS_SUCCESS;

    pDos = (IMAGE_DOS_HEADER*)ImageBase;

    if (MZ_SIGNATURE != pDos->e_magic)  // MZ
    {
        return STATUS_INVALID_MZ_IMAGE;
    }

    nt = (IMAGE_NT_HEADERS64*)((BYTE*)pDos + pDos->e_lfanew);
    if (!CHECK_BOUNDS(nt, sizeof(IMAGE_NT_HEADERS64), ImageBase, ImageSize))
    {
        return STATUS_INVALID_IMAGE_SIZE;
    }

    if (NT_SIGNATURE != nt->Signature)  // PE
    {
        return STATUS_INVALID_PE_IMAGE;
    } 
    
    NtInfo->ImageBase = ImageBase;
    NtInfo->NtBase = nt;
    NtInfo->Size = nt->OptionalHeader.SizeOfImage;
    NtInfo->Machine = nt->FileHeader.Machine;
    NtInfo->Subsystem = nt->OptionalHeader.Subsystem;
    NtInfo->AddressOfEntryPoint = PtrOffset(NtInfo->ImageBase, nt->OptionalHeader.AddressOfEntryPoint);
    NtInfo->SizeOfHeaders = nt->OptionalHeader.SizeOfHeaders;
    NtInfo->NumberOfSections = nt->FileHeader.NumberOfSections;
    NtInfo->ImageAlignment = nt->OptionalHeader.SectionAlignment;
    NtInfo->FileAlignment = nt->OptionalHeader.FileAlignment;

    NtInfo->Preferred.ImageBase = (PVOID) nt->OptionalHeader.ImageBase;
    NtInfo->Preferred.AddressOfEntryPoint = PtrOffset(NtInfo->Preferred.ImageBase, nt->OptionalHeader.AddressOfEntryPoint);

    return STATUS_SUCCESS;
}

STATUS
PeRetrieveSection(
    IN                          PPE_NT_HEADER_INFO      NtInfo,
    IN                          DWORD                   SectionIndex,
    OUT                         PPE_SECTION_INFO        SectionInfo
    )
{
    IMAGE_SECTION_HEADER* pSections;
    IMAGE_NT_HEADERS64* pNt;

    if (NULL == NtInfo)
    {
        return STATUS_INVALID_PARAMETER1;
    }

    if (SectionIndex >= NtInfo->NumberOfSections)
    {
        return STATUS_INVALID_PARAMETER2;
    }

    if (NULL == SectionInfo)
    {
        return STATUS_INVALID_PARAMETER3;
    }

    if (IMAGE_FILE_MACHINE_AMD64 != NtInfo->Machine)
    {
        // we currently only support 64-bit PEs
        return STATUS_UNSUPPORTED;
    }

    pSections = NULL;
    pNt = (IMAGE_NT_HEADERS64*)NtInfo->NtBase;

    pSections = (IMAGE_SECTION_HEADER*)((PBYTE)pNt + sizeof(DWORD) + sizeof(IMAGE_FILE_HEADER) + pNt->FileHeader.SizeOfOptionalHeader);
    if (!CHECK_BOUNDS(pSections, sizeof(IMAGE_SECTION_HEADER) * NtInfo->NumberOfSections, NtInfo->ImageBase, NtInfo->Size) )
    {
        return STATUS_INVALID_IMAGE_SIZE;
    }

    SectionInfo->BaseAddress = (PBYTE)NtInfo->ImageBase + pSections[SectionIndex].VirtualAddress;
    SectionInfo->Size = pSections[SectionIndex].Misc.VirtualSize;
    SectionInfo->Characteristics = pSections[SectionIndex].Characteristics;

    if (!CHECK_BOUNDS(SectionInfo->BaseAddress, SectionInfo->Size, NtInfo->ImageBase, NtInfo->Size))
    {
        return STATUS_INVALID_IMAGE_SIZE;
    }

    return STATUS_SUCCESS;
}

STATUS
PeRetrieveDataDirectory(
    IN                          PPE_NT_HEADER_INFO      NtInfo,
    IN                          BYTE                    DataDirectory,
    OUT                         PPE_DATA_DIRECTORY      DataDirectoryInfo
    )
{
    IMAGE_NT_HEADERS64* pNt;
    PIMAGE_DATA_DIRECTORY pDataDirectory;

    if (NULL == NtInfo)
    {
        return STATUS_INVALID_PARAMETER1;
    }

    if ( IMAGE_DIRECTORY_ENTRY_COM_DESCRIPTOR < DataDirectory )
    {
        return STATUS_INVALID_PARAMETER2;
    }

    if (NULL == DataDirectoryInfo)
    {
        return STATUS_INVALID_PARAMETER3;
    }

    if (IMAGE_FILE_MACHINE_AMD64 != NtInfo->Machine)
    {
        // we currently only support 64-bit PEs
        return STATUS_UNSUPPORTED;
    }

    pNt = (IMAGE_NT_HEADERS64*)NtInfo->NtBase;

    if (DataDirectory >= pNt->OptionalHeader.NumberOfRvaAndSizes)
    {
        return STATUS_INVALID_PARAMETER2;
    }

    pDataDirectory = &pNt->OptionalHeader.DataDirectory[DataDirectory];
    if (!CHECK_BOUNDS(pDataDirectory, sizeof(IMAGE_DATA_DIRECTORY), NtInfo->ImageBase, NtInfo->Size))
    {
        return STATUS_INVALID_IMAGE_SIZE;
    }

    DataDirectoryInfo->BaseAddress = (PBYTE)NtInfo->ImageBase + pDataDirectory->VirtualAddress;
    DataDirectoryInfo->Size = pDataDirectory->Size;

    if (!CHECK_BOUNDS(DataDirectoryInfo->BaseAddress, DataDirectoryInfo->Size, NtInfo->ImageBase, NtInfo->Size))
    {
        return STATUS_INVALID_IMAGE_SIZE;
    }

    return STATUS_SUCCESS;
}