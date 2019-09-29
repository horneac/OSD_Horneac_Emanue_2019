#pragma once

C_HEADER_START

#pragma pack(push,16)
typedef struct _BITMAP
{
    // contains the actual bitmap
    PBYTE                   BitmapBuffer;

    DWORD                   BufferSize;
    DWORD                   BitCount;
} BITMAP, *PBITMAP;
#pragma pack(pop)

//******************************************************************************
// Function:     BitmapPreinit
// Description:  
// Returns:      DWORD - Number of bytes required for the BitmapBuffer
// Parameter:    OUT PBITMAP Bitmap
// Parameter:    IN DWORD NumberOfElements
//******************************************************************************
DWORD
BitmapPreinit(
    OUT         PBITMAP     Bitmap,
    IN          DWORD       NumberOfElements
    );

#define BitmapInit(...)     BitmapInitEx(__VA_ARGS__,FALSE)

void
BitmapInitEx(
    INOUT       PBITMAP     Bitmap,
    IN          PBYTE       BitmapBuffer,
    IN          BOOLEAN     Set
    );

void
BitmapUninit(
    INOUT       PBITMAP     Bitmap
    );

// Returns the size in elements of the bitmap
DWORD
BitmapGetMaxElementCount(
    IN          PBITMAP     Bitmap
    );

#define BitmapSetBit(...)       BitmapSetBitValue(__VA_ARGS__,TRUE)
#define BitmapClearBit(...)     BitmapSetBitValue(__VA_ARGS__,FALSE)

void
BitmapSetBitValue(
    INOUT		PBITMAP		Bitmap,
    IN			DWORD       Index,
    IN          BOOLEAN     Set
    );

BOOLEAN
BitmapGetBitValue(
    IN          PBITMAP     Bitmap,
    IN          DWORD       Index
    );

#define BitmapSetBits(...)      BitmapSetBitsValue(__VA_ARGS__,TRUE)
#define BitmapClearBits(...)    BitmapSetBitsValue(__VA_ARGS__,FALSE)

void
BitmapSetBitsValue(
    INOUT		PBITMAP		Bitmap,
    IN			DWORD       Index,
    IN          DWORD       Count,
    IN          BOOLEAN     Set
    );

SIZE_SUCCESS
DWORD
BitmapScan(
    IN          PBITMAP     Bitmap,
    IN          DWORD       ConsecutiveBits,
    IN          BOOLEAN     Set
    );

SIZE_SUCCESS
DWORD
BitmapScanFrom(
    IN          PBITMAP     Bitmap,
    IN          DWORD       Index,
    IN          DWORD       ConsecutiveBits,
    IN          BOOLEAN     Set
    );

SIZE_SUCCESS
DWORD
BitmapScanFromTo(
    IN          PBITMAP     Bitmap,
    IN          DWORD       Index,
    IN          DWORD       FirstInvalidBitIndex,
    IN          DWORD       ConsecutiveBits,
    IN          BOOLEAN     Set
);

SIZE_SUCCESS
DWORD
BitmapScanAndFlip(
    INOUT       PBITMAP     Bitmap,
    IN          DWORD       ConsecutiveBits,
    IN          BOOLEAN     Set
    );

SIZE_SUCCESS
DWORD
BitmapScanFromAndFlip(
    INOUT       PBITMAP     Bitmap,
    IN          DWORD       Index,
    IN          DWORD       ConsecutiveBits,
    IN          BOOLEAN     Set
    );

SIZE_SUCCESS
DWORD
BitmapScanFromToAndFlip(
    INOUT       PBITMAP     Bitmap,
    IN          DWORD       Index,
    IN          DWORD       FirstInvalidBitIndex,
    IN          DWORD       ConsecutiveBits,
    IN          BOOLEAN     Set
);

C_HEADER_END