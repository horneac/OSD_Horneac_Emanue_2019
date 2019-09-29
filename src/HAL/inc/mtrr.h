#pragma once

typedef enum _MEMORY_CACHING
{
    MemoryCachingStrongUncacheable,
    MemoryCachingWriteCombine       = 1,
    MemoryCachingWriteThrough       = 4,
    MemoryCachingWriteProtect,
    MemoryCachingWriteBack,
    MemoryCachingUncacheable,

    MemoryCachingReserved           = MemoryCachingUncacheable + 1
} MEMORY_CACHING, *PMEMORY_CACHING;