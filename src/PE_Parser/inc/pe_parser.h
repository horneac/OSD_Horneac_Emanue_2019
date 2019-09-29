#pragma once

#include "pe_exports.h"

STATUS
PeRetrieveNtHeader(
    IN_READS_BYTES(ImageSize)   PVOID                   ImageBase,
    IN                          DWORD                   ImageSize,
    OUT                         PPE_NT_HEADER_INFO      NtInfo
    );

STATUS
PeRetrieveSection(
    IN                          PPE_NT_HEADER_INFO      NtInfo,
    IN                          DWORD                   SectionIndex,
    OUT                         PPE_SECTION_INFO        SectionInfo
    );

STATUS
PeRetrieveDataDirectory(
    IN                          PPE_NT_HEADER_INFO      NtInfo,
    IN                          BYTE                    DataDirectory,
    OUT                         PPE_DATA_DIRECTORY      DataDirectoryInfo
    );