#pragma once

C_HEADER_START
#pragma pack(push,8)
typedef struct _DATE
{
    BYTE			Day;
    BYTE			Month;
    WORD			Year;
} DATE, *PDATE;

typedef struct _TIME
{
    BYTE			Second;
    BYTE			Minute;
    BYTE			Hour;
} TIME, *PTIME;

typedef struct _DATETIME
{
    DATE			Date;
    TIME			Time;
} DATETIME, *PDATETIME;
STATIC_ASSERT(sizeof(DATETIME) == sizeof(QWORD));
#pragma pack(pop)

STATUS
TimeGetStringFormattedBuffer(
    IN                          DATETIME        DateTime,
    OUT_WRITES_Z(BufferSize)    char*           Buffer,
    IN                          DWORD           BufferSize
    );
C_HEADER_END
