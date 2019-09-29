#include "common_lib.h"
#include "cl_memory.h"

extern void CpuClearDirectionFlag();

_At_buffer_( address, i, size, _Post_satisfies_( ((PBYTE)address)[i] == value ))
void
cl_memset(
    OUT_WRITES_BYTES_ALL(size)  PVOID address,
    IN                          BYTE value,
    IN                          DWORD size
    )
{
    DWORD i;

    // validate parameters
    // size validation is done implicitly in the for loop
    if (NULL == address)
    {
        return;
    }

    for (i = 0; i < size; ++i)
    {
        ((BYTE*)address)[i] = value;
    }
}

_At_buffer_(Destination, i, Count,
            _Post_satisfies_(((PBYTE)Destination)[i] == ((PBYTE)Source)[i]))
void
cl_memcpy(
    OUT_WRITES_BYTES_ALL(Count) PVOID   Destination,
    IN_READS(Count)             void*   Source,
    IN                          QWORD   Count
    )
{
    PBYTE dst;
    PBYTE src;
    QWORD alignedCount;
    QWORD unalignedCount;

    if( (NULL == Destination) || (NULL == Source))
    {
        return;
    }

    unalignedCount = Count & 0x7;
    alignedCount = Count - unalignedCount;

    dst = (PBYTE)Destination;
    src = (PBYTE)Source;

    if (unalignedCount & 0x4)
    {
        *((PDWORD)dst) = *((PDWORD)src);
        dst = dst + sizeof(DWORD);
        src = src + sizeof(DWORD);
    }

    if (unalignedCount & 0x2)
    {
        *((PWORD)dst) = *((PWORD)src);
        dst = dst + sizeof(WORD);
        src = src + sizeof(WORD);
    }

    if (unalignedCount & 0x1)
    {
        *((PBYTE)dst) = *((PBYTE)src);
        dst = dst + sizeof(BYTE);
        src = src + sizeof(BYTE);
    }

    if (0 != alignedCount)
    {
        ASSERT(IsAddressAligned(alignedCount, sizeof(QWORD)));

        dst = (PBYTE)Destination + unalignedCount;
        src = (PBYTE)Source + unalignedCount;

        CpuClearDirectionFlag();

        __movsq(dst, src, alignedCount / sizeof(QWORD));
    }
}

_At_buffer_(Destination, i, Count,
            _Post_satisfies_(((PBYTE)Destination)[i] == ((PBYTE)Source)[i]))
void
cl_memmove(
    OUT_WRITES_BYTES_ALL(Count) PVOID   Destination,
    IN_READS(Count)             void*   Source,
    IN                          QWORD   Count
    )
{
    PBYTE dst;
    const BYTE* src;
    QWORD i;

    if ((NULL == Destination) || (NULL == Source))
    {
        return;
    }

    dst = Destination;
    src = Source;

    for (i = 0; i < Count; ++i)
    {
        dst[i] = src[i];
    }
}

int
cl_memcmp(
    IN_READS_BYTES(size)    void* ptr1,
    IN_READS_BYTES(size)    void* ptr2,
    IN                      DWORD size
    )
{
    INT64 i = size;
    const BYTE* p1;
    const BYTE* p2;

    if ((NULL == ptr1) || (NULL == ptr2))
    {
        // TODO: what is the best result to return?
        return size;
    }


    p1 = ptr1;
    p2 = ptr2;

    for (i = 0; i < size; ++i)
    {
        if (p1[i] != p2[i])
        {
            return p1[i] - p2[i];
        }
    }

    return 0;
}

int
cl_rmemcmp(
    IN_READS_BYTES(size)    void* ptr1,
    IN_READS_BYTES(size)    void* ptr2,
    IN                      DWORD size
    )
{
    INT64 i = size;
    const BYTE* p1;
    const BYTE* p2;

    if ((NULL == ptr1) || (NULL == ptr2))
    {
        // TODO: what is the best result to return?
        return size;
    }


    p1 = ptr1;
    p2 = ptr2;

    for (i = size - 1; i >= 0; --i)
    {
        if (p1[i] != p2[i])
        {
            return p1[i] - p2[i];
        }
    }

    return 0;
}

int
cl_memscan(
    IN_READS_BYTES(size)    void* buffer,
    IN                      DWORD size,
    IN                      BYTE  value
    )
{
    DWORD i;
    const BYTE* pData;

    if (NULL == buffer)
    {
        return 0;
    }

    pData = buffer;

    for (i = 0; i < size; ++i)
    {
        if (pData[i] != value)
        {
            // game over
            return i;
        }
    }

    return i;
}