#include "common_lib.h"
#include "hash_table.h"

#pragma warning(push)

// warning C4200: nonstandard extension used: zero-sized array in struct/union
#pragma warning(disable:4200)
typedef struct _HASH_TABLE_DATA
{
    HASH_ENTRY          Entries[0];
} HASH_TABLE_DATA, *PHASH_TABLE_DATA;
#pragma warning(pop)

static FUNC_CompareFunction _HashTableSearchElem;

typedef struct _HASH_ELEM_SEARCH_CTX
{
    PHASH_TABLE             HashTable;

    PHASH_ENTRY             PlaceholderEntry;

    INT32                   Result;
} HASH_ELEM_SEARCH_CTX, *PHASH_ELEM_SEARCH_CTX;

static
__forceinline
PHASH_KEY
_HashTableObtainKeyAddress(
    IN      PHASH_TABLE         HashTable,
    IN      PHASH_ENTRY         Element
    )
{
    ASSERT(HashTable != NULL);
    ASSERT(Element != NULL);

    return (PHASH_KEY) ((PBYTE)Element + HashTable->OffsetToKey);
}

static
__forceinline
BOOLEAN
_HashTableSearchKeyInBucket(
    IN      PHASH_TABLE         HashTable,
    IN      PHASH_ENTRY         Bucket,
    IN      PHASH_KEY           Key,
    OUT_PTR PHASH_ENTRY*        Placeholder
    )
{
    PHASH_ENTRY pResult;
    HASH_ELEM_SEARCH_CTX searchCtx = { 0 };

    ASSERT(HashTable != NULL);
    ASSERT(Bucket != NULL);
    ASSERT(Key != NULL);
    ASSERT(Placeholder != NULL);

    searchCtx.HashTable = HashTable;
    searchCtx.PlaceholderEntry = Bucket;
    searchCtx.Result = 0;

    // ListSearchForElement takes as the second argument (entry to compare with) a HASH_ENTRY, however because the only
    // place that parameter is used is in our own _HashTableSearchElem function we do a 'hackish' optimization so as
    // not to perform two extra operations
    pResult = ListSearchForElement(Bucket, (PHASH_ENTRY) Key, TRUE, _HashTableSearchElem, &searchCtx);
    if (pResult != NULL)
    {
        ASSERT(pResult == searchCtx.PlaceholderEntry);
        ASSERT(searchCtx.Result == 0);
    }

    // If the result > 0 => the element we reached has a higher value than the one which we may want to insert, as a
    // result the insertion will be done after the Blink pointer
    *Placeholder = (searchCtx.Result > 0) ? searchCtx.PlaceholderEntry->Blink : searchCtx.PlaceholderEntry;

    return (pResult != NULL);
}

DWORD
HashTablePreinit(
    OUT     PHASH_TABLE         HashTable,
    IN      DWORD               MaxKeys,
    IN      DWORD               KeySize
    )
{
    ASSERT(HashTable != NULL);
    ASSERT(0 < MaxKeys && MaxKeys < MAX_DWORD / sizeof(HASH_ENTRY));

    memzero(HashTable, sizeof(HASH_TABLE));

    HashTable->MaxKeys = MaxKeys;
    HashTable->KeySize = KeySize;

    return HashTable->MaxKeys * sizeof(HASH_ENTRY);
}

void
HashTableInit(
    INOUT   PHASH_TABLE         HashTable,
    OUT_WRITES_BYTES(HashTable->MaxKeys * sizeof(HASH_ENTRY))
            PHASH_TABLE_DATA    TableData,
    IN      PFUNC_HashFunction  HashFunction,
    IN      INT32               OffsetToKey
    )
{
    ASSERT(HashTable != NULL);
    ASSERT(TableData != NULL);
    ASSERT(HashFunction != NULL);

    HashTable->TableData = TableData;
    HashTable->HashFunc = HashFunction;
    HashTable->OffsetToKey = OffsetToKey;

    for (DWORD i = 0; i < HashTable->MaxKeys; ++i)
    {
        InitializeListHead(&HashTable->TableData->Entries[i]);
    }
}

void
HashTableClear(
    INOUT   PHASH_TABLE         HashTable,
    IN_OPT  PFUNC_FreeFunction  FreeFunction,
    IN_OPT  PVOID               FreeContext
    )
{
    HASH_ITERATOR it;
    PHASH_ENTRY pEntry;

    ASSERT(HashTable != NULL);

    HashTableIteratorInit(HashTable, &it);

    while ((pEntry = HashTableIteratorNext(&it)) != NULL)
    {
        // remove from hash table
        HashTableRemoveEntry(HashTable, pEntry);

        // call free function if one was provided
        if (FreeFunction != NULL)
        {
            FreeFunction(pEntry, FreeContext);
        }
    }
}

DWORD
HashTableSize(
    IN      HASH_TABLE*         HashTable
    )
{
    ASSERT(HashTable != NULL);

    return HashTable->NumberOfElements;
}

PHASH_ENTRY
HashTableInsert(
    INOUT   PHASH_TABLE         HashTable,
    INOUT   PHASH_ENTRY         Element
    )
{
    PHASH_ENTRY pPlaceToInsertInBucket;
    PHASH_KEY pKey;
    QWORD hIndex;

    ASSERT(HashTable != NULL);
    ASSERT(Element != NULL);

    pKey = _HashTableObtainKeyAddress(HashTable, Element);

    hIndex = HashTable->HashFunc(pKey,
                                 HashTable->KeySize,
                                 HashTable->MaxKeys);
    ASSERT(hIndex < HashTable->MaxKeys);

    BOOLEAN bElementAlreadyInHash = _HashTableSearchKeyInBucket(
        HashTable,
        &HashTable->TableData->Entries[hIndex],
        pKey,
        &pPlaceToInsertInBucket);

    InsertHeadList(pPlaceToInsertInBucket, Element);

    if (bElementAlreadyInHash)
    {
        RemoveEntryList(pPlaceToInsertInBucket);
    }
    else
    {
        HashTable->NumberOfElements++;
    }

    return bElementAlreadyInHash ? pPlaceToInsertInBucket : NULL;
}

PTR_SUCCESS
PHASH_ENTRY
HashTableRemove(
    INOUT   PHASH_TABLE         HashTable,
    IN      PHASH_KEY           Key
    )
{
    PHASH_ENTRY pHashEntry;

    ASSERT(HashTable != NULL);
    ASSERT(Key != NULL);

    pHashEntry = HashTableLookup(HashTable, Key);
    if (pHashEntry != NULL)
    {
        HashTableRemoveEntry(HashTable, pHashEntry);
    }

    return pHashEntry;
}

void
HashTableRemoveEntry(
    INOUT   PHASH_TABLE         HashTable,
    IN      PHASH_ENTRY         Element
    )
{
    ASSERT(HashTable != NULL);
    ASSERT(Element != NULL);

    RemoveEntryList(Element);

    HashTable->NumberOfElements--;
}

PTR_SUCCESS
PHASH_ENTRY
HashTableLookup(
    INOUT   PHASH_TABLE         HashTable,
    IN      PHASH_KEY           Key
    )
{
    QWORD hIndex;
    PHASH_ENTRY pResult;

    ASSERT(HashTable != NULL);
    ASSERT(Key != NULL);

    hIndex = HashTable->HashFunc(Key,
                                 HashTable->KeySize,
                                 HashTable->MaxKeys);

    BOOLEAN bElementInHash = _HashTableSearchKeyInBucket(
        HashTable,
        &HashTable->TableData->Entries[hIndex],
        Key,
        &pResult);

    return bElementInHash ? pResult : NULL;
}

void
HashTableIteratorInit(
    IN      PHASH_TABLE         HashTable,
    OUT     PHASH_ITERATOR      HashIterator
    )
{
    ASSERT(HashTable != NULL);
    ASSERT(HashIterator != NULL);

    HashIterator->HashTable = HashTable;
    HashIterator->KeyIndex = 0;

    ListIteratorInit(&HashTable->TableData->Entries[0], &HashIterator->CurrentKeyIterator);
}

PHASH_ENTRY
HashTableIteratorNext(
    INOUT   PHASH_ITERATOR      HashIterator
    )
{
    PHASH_ENTRY pResult;

    ASSERT(HashIterator != NULL);

    pResult = NULL;

    while (HashIterator->KeyIndex < HashIterator->HashTable->MaxKeys)
    {
        pResult = ListIteratorNext(&HashIterator->CurrentKeyIterator);

        if (pResult) break;

        HashIterator->KeyIndex++;
        ListIteratorInit(&HashIterator->HashTable->TableData->Entries[HashIterator->KeyIndex], &HashIterator->CurrentKeyIterator);
    }

    return pResult;
}

QWORD
(__cdecl HashFuncGenericIncremental) (
    IN_READS_BYTES(KeyLength)   PHASH_KEY   Key,
    IN                          DWORD       KeyLength,
    IN                          DWORD       MaxKeys
    )
{
    QWORD keyValue;

    ASSERT(Key != NULL);
    ASSERT(KeyLength <= MAX_QWORD);

    keyValue = 0;
    cl_memcpy(&keyValue, Key, KeyLength);

    return keyValue % MaxKeys;
}

#define HASH_UNIVERSAL_A        1701
#define HASH_UNIVERSAL_B        59
#define HASH_UNIVERSAL_P        10007

STATIC_ASSERT_INFO(HASH_UNIVERSAL_P <= (MAX_QWORD / HASH_UNIVERSAL_A) - HASH_UNIVERSAL_B,
    "If hash HASH_UNIVERSAL_P is too big we may have an OF, see assert in HashFuncUniversal");

QWORD
(__cdecl HashFuncUniversal) (
    IN_READS_BYTES(KeyLength)   PHASH_KEY   Key,
    IN                          DWORD       KeyLength,
    IN                          DWORD       MaxKeys
    )
{
    QWORD keyValue;
    QWORD result;

    ASSERT(Key != NULL);
    ASSERT(KeyLength <= MAX_QWORD);

    keyValue = 0;
    cl_memcpy(&keyValue, Key, KeyLength);

    // perform modulo operation first to make it less likely for
    // an overflow to happen
    result = keyValue % HASH_UNIVERSAL_P;

    ASSERT_INFO(result < (MAX_QWORD / HASH_UNIVERSAL_A) - HASH_UNIVERSAL_B,
        "Check for OF condition!\n");

    return ((HASH_UNIVERSAL_A * result
                + HASH_UNIVERSAL_B)
                    % HASH_UNIVERSAL_P)
                        % MaxKeys;
}

static
INT64
(__cdecl _HashTableSearchElem) (
    IN      PLIST_ENTRY     FirstElem,
    IN      PLIST_ENTRY     SecondElem,
    IN_OPT  PVOID           Context
    )
{
    PHASH_ELEM_SEARCH_CTX pCtx;

    ASSERT(FirstElem != NULL);
    ASSERT(SecondElem != NULL);
    ASSERT(Context != NULL);

    pCtx = (PHASH_ELEM_SEARCH_CTX)Context;

    pCtx->PlaceholderEntry = FirstElem;

    pCtx->Result = rmemcmp(
        _HashTableObtainKeyAddress(pCtx->HashTable, FirstElem),
        SecondElem, // This is actually a PHASH_KEY
        pCtx->HashTable->KeySize);

    return pCtx->Result;
}
