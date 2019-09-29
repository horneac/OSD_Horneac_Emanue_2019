#include "common_lib.h"
#include "bitmap.h"

// we defined BITMAP_ENTRY_BITS directly to 32 so as not to call
// BITS_FOR_STRUCTURE for each index calculation
#define BITMAP_ENTRY_BITS           8

static
void
_BitmapChangeBit(
    INOUT       PBYTE       BitmapBuffer,
    IN          DWORD       Index,
    IN          BOOLEAN     Set
    );

static
void
_BitmapChangeBits(
    INOUT       PBYTE       BitmapBuffer,
    IN          DWORD       Index,
    IN          BOOLEAN     Set,
    IN          DWORD       Count
    );

static
BOOLEAN
_BitmapGetBit(
    IN          PBYTE       BitmapBuffer,
    IN          DWORD       Index
    );

static
SIZE_SUCCESS
DWORD
_BitmapScanInternal(
    IN          PBITMAP     Bitmap,
    IN          DWORD       StartIndex,
    IN          DWORD       FirstInvalidBitIndex,
    IN          DWORD       ConsecutiveBits,
    IN          BOOLEAN     Set
    );

DWORD
BitmapPreinit(
    OUT         PBITMAP     Bitmap,
    IN          DWORD       NumberOfElements
    )
{
    ASSERT( NULL != Bitmap );
    ASSERT( 0 != NumberOfElements );

    memzero(Bitmap, sizeof(BITMAP));

    Bitmap->BitCount = NumberOfElements;
    Bitmap->BufferSize = ( AlignAddressUpper(Bitmap->BitCount, BITMAP_ENTRY_BITS) / BITMAP_ENTRY_BITS ) * sizeof(BYTE);

    return Bitmap->BufferSize;
}

void
BitmapInitEx(
    INOUT       PBITMAP     Bitmap,
    IN          PBYTE       BitmapBuffer,
    IN          BOOLEAN     Set
    )
{
    ASSERT(NULL != Bitmap);
    ASSERT(NULL != BitmapBuffer);

    memset(BitmapBuffer,
           Set ? MAX_BYTE : 0,
           Bitmap->BufferSize);

    Bitmap->BitmapBuffer = BitmapBuffer;
}

void
BitmapUninit(
    INOUT       PBITMAP     Bitmap
    )
{
    ASSERT( NULL != Bitmap );

    memzero(Bitmap, sizeof(BITMAP));
}

DWORD
BitmapGetMaxElementCount(
    IN          PBITMAP     Bitmap
    )
{
    ASSERT(NULL != Bitmap);

    return Bitmap->BitCount;
}

void
BitmapSetBitValue(
    INOUT		PBITMAP		Bitmap,
    IN			DWORD       Index,
    IN          BOOLEAN     Set
    )
{
    ASSERT(NULL != Bitmap);
    ASSERT(Index < Bitmap->BitCount);

    _BitmapChangeBit(Bitmap->BitmapBuffer, Index, Set);
}

BOOLEAN
BitmapGetBitValue(
    IN          PBITMAP     Bitmap,
    IN          DWORD       Index
    )
{
    ASSERT(NULL != Bitmap);
    ASSERT(Index < Bitmap->BitCount);

    return _BitmapGetBit(Bitmap->BitmapBuffer, Index );
}

void
BitmapSetBitsValue(
    INOUT		PBITMAP		Bitmap,
    IN			DWORD       Index,
    IN          DWORD       Count,
    IN          BOOLEAN     Set
    )
{
    ASSERT(NULL != Bitmap);
    ASSERT(Index < Bitmap->BitCount);
    ASSERT(Bitmap->BitCount - Count >= Index);

    _BitmapChangeBits(Bitmap->BitmapBuffer, Index, Set, Count);
}

SIZE_SUCCESS
DWORD
BitmapScan(
    IN          PBITMAP     Bitmap,
    IN          DWORD       ConsecutiveBits,
    IN          BOOLEAN     Set
    )
{
    return BitmapScanFromTo(Bitmap,0,  Bitmap->BitCount, ConsecutiveBits,Set);
}

SIZE_SUCCESS
DWORD
BitmapScanFrom(
    IN          PBITMAP     Bitmap,
    IN          DWORD       Index,
    IN          DWORD       ConsecutiveBits,
    IN          BOOLEAN     Set
    )
{
    return BitmapScanFromTo(Bitmap,Index, Bitmap->BitCount, ConsecutiveBits,Set);
}

SIZE_SUCCESS
DWORD
BitmapScanFromTo(
    IN          PBITMAP     Bitmap,
    IN          DWORD       Index,
    IN          DWORD       FirstInvalidBitIndex,
    IN          DWORD       ConsecutiveBits,
    IN          BOOLEAN     Set
)
{
    if (NULL == Bitmap)
    {
        return MAX_DWORD;
    }

    if (0 == ConsecutiveBits)
    {
        return MAX_DWORD;
    }

    if( Index > FirstInvalidBitIndex || FirstInvalidBitIndex > Bitmap->BitCount)
    {
        return MAX_DWORD;
    }

    return _BitmapScanInternal(Bitmap, Index, FirstInvalidBitIndex, ConsecutiveBits, Set);
}

SIZE_SUCCESS
DWORD
BitmapScanAndFlip(
    INOUT       PBITMAP     Bitmap,
    IN          DWORD       ConsecutiveBits,
    IN          BOOLEAN     Set
    )
{
    return BitmapScanFromToAndFlip(Bitmap, 0, Bitmap->BitCount, ConsecutiveBits, Set);
}

SIZE_SUCCESS
DWORD
BitmapScanFromAndFlip(
    INOUT       PBITMAP     Bitmap,
    IN          DWORD       Index,
    IN          DWORD       ConsecutiveBits,
    IN          BOOLEAN     Set
    )
{
    return BitmapScanFromToAndFlip(Bitmap, Index, Bitmap->BitCount, ConsecutiveBits, Set );
}

SIZE_SUCCESS
DWORD
BitmapScanFromToAndFlip(
    INOUT       PBITMAP     Bitmap,
    IN          DWORD       Index,
    IN          DWORD       FirstInvalidBitIndex,
    IN          DWORD       ConsecutiveBits,
    IN          BOOLEAN     Set
)
{
    DWORD bitmapIndex;

    if (NULL == Bitmap)
    {
        return MAX_DWORD;
    }

    if (0 == ConsecutiveBits)
    {
        return MAX_DWORD;
    }

    if( Index > FirstInvalidBitIndex || FirstInvalidBitIndex > Bitmap->BitCount)
    {
        return MAX_DWORD;
    }

    bitmapIndex = _BitmapScanInternal(Bitmap, Index, FirstInvalidBitIndex, ConsecutiveBits, Set);
    if (MAX_DWORD == bitmapIndex)
    {
        return MAX_DWORD;
    }

    // let's flip these bits
    _BitmapChangeBits(Bitmap->BitmapBuffer, bitmapIndex, !Set, ConsecutiveBits);

    return bitmapIndex;
}

static
void
_BitmapChangeBit(
    INOUT       PBYTE       BitmapBuffer,
    IN          DWORD       Index,
    IN          BOOLEAN     Set
    )
{
    DWORD byteIndex;
    DWORD bitIndex;

    ASSERT( NULL != BitmapBuffer );

    byteIndex = Index / BITMAP_ENTRY_BITS;
    bitIndex = Index % BITMAP_ENTRY_BITS;

    if (Set)
    {
        BitmapBuffer[byteIndex] |= ( 1 << bitIndex );
    }
    else
    {
        BitmapBuffer[byteIndex] &= ~( 1 << bitIndex );
    }
}

static
void
_BitmapChangeBits(
    INOUT       PBYTE       BitmapBuffer,
    IN          DWORD       Index,
    IN          BOOLEAN     Set,
    IN          DWORD       Count
    )
{
    DWORD i;

    for (i = 0; i < Count; ++i)
    {
        _BitmapChangeBit(BitmapBuffer, Index + i, Set);
    }
}

static
BOOLEAN
_BitmapGetBit(
    IN          PBYTE       BitmapBuffer,
    IN          DWORD       Index
    )
{
    DWORD byteIndex;
    DWORD bitIndex;

    ASSERT( NULL != BitmapBuffer);

    byteIndex = Index / BITMAP_ENTRY_BITS;
    bitIndex = Index % BITMAP_ENTRY_BITS;

    return IsBooleanFlagOn( BitmapBuffer[byteIndex], ( (BYTE) 1 << bitIndex ) );
}

static
SIZE_SUCCESS
DWORD
_BitmapScanInternal(
    IN          PBITMAP     Bitmap,
    IN          DWORD       StartIndex,
    IN          DWORD       FirstInvalidBitIndex,
    IN          DWORD       ConsecutiveBits,
    IN          BOOLEAN     Set
    )
{
    DWORD i, j;
    DWORD lastIndex;
    DWORD firstIndex;
    BOOLEAN found;

    ASSERT( NULL != Bitmap );
    ASSERT( 0 != ConsecutiveBits );
    ASSERT( StartIndex <= FirstInvalidBitIndex );
    ASSERT( FirstInvalidBitIndex <= Bitmap->BitCount );

    if (FirstInvalidBitIndex - StartIndex < ConsecutiveBits)
    {
        // we don't have that many bits
        return MAX_DWORD;
    }

    lastIndex = FirstInvalidBitIndex - ConsecutiveBits;
    firstIndex = StartIndex;
    found = FALSE;

    for (i = firstIndex; i <= lastIndex; ++i)
    {
        found = TRUE;
        for (j = 0; j < ConsecutiveBits; ++j)
        {
            if (Set != _BitmapGetBit(Bitmap->BitmapBuffer, i + j))
            {
                found = FALSE;
                break;
            }
        }

        if (found)
        {
            return i;
        }
    }

    return MAX_DWORD;
}