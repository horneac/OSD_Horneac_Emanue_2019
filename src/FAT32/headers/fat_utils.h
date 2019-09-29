#pragma once

STATUS
NextSectorInClusterChain(
    IN      PFAT_DATA       FatData,
    IN      QWORD           CurrentSector,
    OUT     QWORD*          NextSector,
    IN      BOOLEAN         ExtendChain
);

STATUS
NextClusterInChain(
    IN      PFAT_DATA       FatData,
    IN      QWORD           CurrentCluster,
    OUT     QWORD*          Result
);

STATUS
FirstSectorOfCluster(
    IN      PFAT_DATA   FatData,
    IN      QWORD       Cluster,
    OUT     QWORD*      Result
);

STATUS
ClusterOfSector(
    IN      PFAT_DATA   FatData,
    IN      QWORD       Sector,
    OUT     QWORD*      Result
);

STATUS
GetDirEntryFromSector(
    IN      PFAT_DATA   FatData,
    IN      QWORD       EntrySector,
    IN      QWORD       DirFirstSector, // search criterion
    OUT     QWORD*      EntryIndex,
    OUT     DIR_ENTRY*  DirEntry
);

STATUS
WriteDirEntryToSector(
    IN      PFAT_DATA   FatData,
    IN      QWORD       EntrySector,
    IN      QWORD       EntryIndex,
    IN      DIR_ENTRY*  DirEntry
);

STATUS
ConvertFatDateTimeToDateTime(
    IN      FATDATE*    FatDate,
    IN      FATTIME*    FatTime,
    OUT     DATETIME*   DateTime
);

void
ConvertDateTimeToFatDateTime(
    IN      PDATETIME   DateTime,
    OUT     FATDATE*    FatDate,
    OUT     FATTIME*    FatTime
);

STATUS
ConvertFatNameToName(
    IN_READS(SHORT_NAME_CHARS)      char*       FatName,
    IN                              DWORD       BufferSize,
    OUT_WRITES(BufferSize)          char*       Buffer,
    OUT                             DWORD*      ActualNameLength
);

STATUS
WritePartialLongFatNameToName(
    IN                          LONG_DIR_ENTRY* LongDirEntry,
    IN                          DWORD           BufferSize,
    INOUT_UPDATES(BufferSize)   char*           Buffer
);
