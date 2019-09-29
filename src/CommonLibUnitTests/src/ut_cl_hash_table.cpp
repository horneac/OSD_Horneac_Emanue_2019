#include "ut_base.h"
#include "ut_cl_hash_table.h"
#include "hash_table.h"
#include <unordered_map>
#include "ut_cl_rng.h"

typedef struct _UT_HASH_ELEM
{
    HASH_ENTRY                  HashEntry;

    QWORD                       Value;
} UT_HASH_ELEM, *PUT_HASH_ELEM;

typedef struct _UT_PHASE
{
    DWORD                       ElemsToInsert;

    DWORD                       ElemsToRemoveByKeyLinearly;
    DWORD                       ElemsToRemoveByKeyLinearlyThroughSkipping;
    DWORD                       ElemsToRemoveByEntryLinearly;
    DWORD                       ElemsToRemoveByEntryThroughSkipping;
} UT_PHASE, *PUT_PHASE;

static constexpr auto NO_OF_PHASES = 2;

typedef struct _STACK_UT_PARAMS
{
    const std::string           TestName;

    DWORD                       MaxKeys;

    UT_PHASE                    Phases[NO_OF_PHASES];
} STACK_UT_PARAMS, *PSTACK_UT_PARAMS;

static const STACK_UT_PARAMS UT_PARAMS[] =
{
    {"Empty remove", 16, 0, 25, 25, 25, 25, 0, 25, 25, 25, 25},
    {"No remove", 16, 10000, 0, 0, 0, 0, 10000, 0, 0, 0, 0},
    {"Basic test", 16, 1000, 250, 250, 250, 250, 1000, 250, 250, 250, 250},
    {"Lots of collisions", 4, 10000, 2000, 2000, 2000, 2000, 10000, 2000, 2000, 2000, 2000},
    {"A lot of buckets", 1024, 10000, 2000, 2000, 2000, 2000, 10000, 2000, 2000, 2000, 2000},
    {"Single bucket", 1, 1000, 500, 500, 500, 500, 2000, 500, 500, 500, 500},
    {"Many entries", 4096, 100'000, 50'000, 50'000, 50'000, 50'000, 100'000, 50'000, 50'000, 50'000, 50'000},
    {"Huge entries", 100'000, 1'000'000, 0, 0, 0, 1'000'000, 1'000'000, 1'000'000, 1'000'000, 1'000'000},
};

static const PFUNC_HashFunction HASH_FUNCTIONS[] =
{
    HashFuncGenericIncremental, HashFuncUniversal
};

static const DWORD KEY_SIZES[] =
{
    1, 2, 3, 4, 5, 6, 7, 8
};

static
STATUS
_HashTableCreate(
    _In_        DWORD               MaxKeys,
    _In_        DWORD               KeySize,
    _In_        PFUNC_HashFunction  HashFunc,
    _Out_       HASH_TABLE*         HashTable
    )
{
    ASSERT(HashFunc != nullptr);
    ASSERT(HashTable != nullptr);

    DWORD requiredDataSize = HashTablePreinit(
        HashTable,
        MaxKeys,
        KeySize);

    if (requiredDataSize == 0) return CL_STATUS_SIZE_INVALID;

    PHASH_TABLE_DATA pData = (PHASH_TABLE_DATA) new BYTE[requiredDataSize];

    HashTableInit(
        HashTable,
        pData,
        HashFunc,
        FIELD_OFFSET(UT_HASH_ELEM, Value) - FIELD_OFFSET(UT_HASH_ELEM, HashEntry));

    return CL_STATUS_SUCCESS;
}

static
STATUS
_HashInsertElements(
    _Inout_     HASH_TABLE*                         HashTable,
    _In_        DWORD                               ElementsToInsert,
    _Outref_    PUT_HASH_ELEM&                      HashElems,
    _Inout_     std::unordered_map<QWORD,DWORD>&    ShadowHash
    )
{
    STATUS status;
    UtCl::RNG rngInstance = UtCl::RNG::GetInstance();

    ASSERT(HashTable != nullptr);

    status = CL_STATUS_SUCCESS;
    PUT_HASH_ELEM pStackElems = new UT_HASH_ELEM[ElementsToInsert];

    for (DWORD i = 0; i < ElementsToInsert; ++i)
    {
        pStackElems[i].Value = (rngInstance.GetNextRandom() & CREATE_BIT_MASK_FOR_N_BITS(BITS_PER_BYTE * HashTable->KeySize));

        PHASH_ENTRY pEntry = HashTableLookup(HashTable, (PHASH_KEY) &pStackElems[i].Value);
        BOOLEAN bFoundInShadowHash = (ShadowHash.find(pStackElems[i].Value) != ShadowHash.end());

        if (pEntry == nullptr)
        {
            if (bFoundInShadowHash)
            {
                LOG_ERROR("We couldn't find key 0x%I64X in our hash table, however it is present in shadow hash!\n",
                    pStackElems[i].Value);
                status = CL_STATUS_ELEMENT_NOT_FOUND;
            }
        }
        else
        {
            if (!bFoundInShadowHash)
            {
                LOG_ERROR("We couldn't find key 0x%I64X in our shadow hash table, however it is present in our hash!\n",
                    pStackElems[i].Value);
                status = CL_STATUS_ELEMENT_FOUND;
            }
        }

        if (!SUCCEEDED(status)) break;

        ASSERT(pStackElems[i].Value <= MAX_DWORD);
        ShadowHash[pStackElems[i].Value] = (DWORD) pStackElems[i].Value;
        PHASH_ENTRY pPreviousElem = HashTableInsert(HashTable, &pStackElems[i].HashEntry);

        if (pPreviousElem != pEntry)
        {
            QWORD entryValue = MAX_QWORD;
            QWORD prevValue = MAX_QWORD;

            if (pEntry != nullptr)
            {
                entryValue = (CONTAINING_RECORD(pEntry, UT_HASH_ELEM, HashEntry))->Value;
            }

            if (pPreviousElem != nullptr)
            {
                prevValue = (CONTAINING_RECORD(pPreviousElem, UT_HASH_ELEM, HashEntry))->Value;
            }

            LOG_ERROR("When we did a lookup we got the previous element at 0x%p with value 0x%I64X, however now we received element at 0x%p with value 0x%I64X\n",
                pEntry, entryValue, pPreviousElem, prevValue);
            status = CL_STATUS_INTERNAL_ERROR;
            break;
        }
    }

    HashElems = pStackElems;

    return status;
}

static
STATUS
_HashRemoveElements(
    _Inout_     HASH_TABLE*                         HashTable,
    _In_        DWORD                               ElementsToRemove,
    _Inout_     std::unordered_map<QWORD, DWORD>&   ShadowHash,
    _In_        bool                                SkipOne,
    _In_        bool                                RemoveByEntry
    )
{
    STATUS status;
    HASH_ITERATOR it;

    ASSERT(HashTable != nullptr);

    status = CL_STATUS_SUCCESS;
    HashTableIteratorInit(HashTable, &it);

    for (DWORD i = 0; i < ElementsToRemove; ++i)
    {
        if (SkipOne && ((i % 2) == 0)) continue;

        PHASH_ENTRY pEntry = HashTableIteratorNext(&it);
        if (pEntry == nullptr)
        {
            if (!ShadowHash.empty())
            {
                LOG_ERROR("Our hash table is empty, however shadow hash is not, it has %zu elements!\n",
                    ShadowHash.size());
                status = CL_STATUS_ELEMENT_NOT_FOUND;
            }
        }
        else
        {
            QWORD keyToDelete = (CONTAINING_RECORD(pEntry, UT_HASH_ELEM, HashEntry))->Value;

            if (ShadowHash.empty())
            {
                LOG_ERROR("Shadow hash is empty, however our hash table is not, it has %u elements!\n",
                    HashTableSize(HashTable));
                status = CL_STATUS_ELEMENT_FOUND;
            }
            else
            {
                if (RemoveByEntry)
                {
                    HashTableRemoveEntry(HashTable, pEntry);
                }
                else
                {
                    PHASH_ENTRY pDeletedEntry = HashTableRemove(HashTable, (PHASH_KEY)&keyToDelete);
                    if (pDeletedEntry != pEntry)
                    {
                        LOG_ERROR("Entry received at iterator next differs from that of remove. Entry at iterator is at 0x%p with value 0x%I64X, at delete it is at 0x%p with value 0x%I64X!\n",
                            pEntry, keyToDelete, pDeletedEntry, pDeletedEntry != nullptr ? (CONTAINING_RECORD(pDeletedEntry, UT_HASH_ELEM, HashEntry))->Value : MAX_QWORD);
                        status = CL_STATUS_INTERNAL_ERROR;
                    }
                }

                ShadowHash.erase(keyToDelete);
            }
        }

        if (!SUCCEEDED(status)) break;
    }

    return status;
}

static
STATUS
_HashCompareSizes(
    _In_ const HASH_TABLE*                          HashTable,
    _In_ const std::unordered_map<QWORD, DWORD>&    ShadowHash
    )
{
    DWORD stackSize;
    size_t shadowStackSize;

    ASSERT(HashTable != nullptr);

    stackSize = HashTableSize(HashTable);
    shadowStackSize = ShadowHash.size();
    ASSERT(shadowStackSize <= MAX_DWORD);

    if (stackSize == shadowStackSize) return CL_STATUS_SUCCESS;

    LOG_ERROR("Our reported hash size is %u, while the shadow hash size is %zu\n",
        stackSize, shadowStackSize);

    return CL_STATUS_SIZE_INVALID;
}

static
void
(_cdecl _HashFreeItem)(
    IN      PVOID       Object,
    IN_OPT  PVOID       Context
    )
{
    std::vector<DWORD> *vect = (std::vector<DWORD>*) Context;

    ASSERT(Object != nullptr);
    ASSERT(Context != nullptr);

    PUT_HASH_ELEM pStackElem = CONTAINING_RECORD(Object, UT_HASH_ELEM , HashEntry);

    ASSERT(pStackElem->Value <= MAX_DWORD);
    vect->push_back((DWORD)pStackElem->Value);
}

static
STATUS
_HashClear(
    _Inout_ HASH_TABLE*                          HashTable,
    _Inout_ std::unordered_map<QWORD, DWORD>&    ShadowHash
    )
{
    STATUS status;
    std::vector<DWORD> removedElems;

    ASSERT(HashTable != nullptr);

    status = CL_STATUS_SUCCESS;

    HashTableClear(
        HashTable,
        _HashFreeItem,
        &removedElems
    );

    for (const auto& value : removedElems)
    {
        bool bShadowHashEmpty = ShadowHash.empty();

        if (bShadowHashEmpty)
        {
            LOG_ERROR("The shadow stack is empty, however we were able to remove element with value %u from stack!\n",
                value);
            status = CL_STATUS_ELEMENT_FOUND;
        }
        else
        {
            bool bFoundElemInShadowHash = (ShadowHash.find(value) != ShadowHash.end());

            if (!bFoundElemInShadowHash)
            {

                status = CL_STATUS_INTERNAL_ERROR;
            }
            else
            {
                DWORD shadowHash = ShadowHash[value];

                if (value != shadowHash)
                {
                    LOG_ERROR("Values in hashes differ: value is %u while on shadow hash value is %u\n",
                        value, shadowHash);
                    status = CL_STATUS_VALUE_MISMATCH;
                }
            }
        }

        if (!SUCCEEDED(status)) break;

        ShadowHash.erase(value);
    }

    if (SUCCEEDED(status))
    {
        if (!ShadowHash.empty())
        {
            LOG_ERROR("We have cleared our whole stack, however the shadow stack still has %zu elements!\n",
                ShadowHash.size());
            return CL_STATUS_SIZE_INVALID;
        }
    }

    return status;
}

STATUS
_UtClRunTestcase(
    _In_ const STACK_UT_PARAMS&             Params,
    _In_ const PFUNC_HashFunction&          HashFunc,
    _In_ const DWORD&                       KeySize
    )
{
    STATUS status;
    HASH_TABLE hashTable;
    std::unordered_map<QWORD, DWORD> shadowHash;
    PUT_HASH_ELEM elems[NO_OF_PHASES] = {};

    status = _HashTableCreate(
        Params.MaxKeys,
        KeySize,
        HashFunc,
        &hashTable);
    if (!SUCCEEDED(status))
    {
        LOG_FUNC_ERROR("_HashTableCreate", status);
        goto cleanup;
    }

    status = _HashCompareSizes(
        &hashTable,
        shadowHash);
    if (!SUCCEEDED(status))
    {
        LOG_FUNC_ERROR("_HashCompareSizes", status);
        goto cleanup;
    }

    for (auto i = 0; i < NO_OF_PHASES; ++i)
    {
        status = _HashInsertElements(
            &hashTable,
            Params.Phases[i].ElemsToInsert,
            elems[i],
            shadowHash
        );
        if (!SUCCEEDED(status))
        {
            LOG_FUNC_ERROR("_HashInsertElements", status);
            goto cleanup;
        }

        status = _HashCompareSizes(
            &hashTable,
            shadowHash);
        if (!SUCCEEDED(status))
        {
            LOG_FUNC_ERROR("_HashCompareSizes", status);
            goto cleanup;
        }

        status = _HashRemoveElements(
            &hashTable,
            Params.Phases[i].ElemsToRemoveByKeyLinearly,
            shadowHash,
            false,
            false);
        if (!SUCCEEDED(status))
        {
            LOG_FUNC_ERROR("_HashRemoveElements", status);
            goto cleanup;
        }

        status = _HashCompareSizes(
            &hashTable,
            shadowHash);
        if (!SUCCEEDED(status))
        {
            LOG_FUNC_ERROR("_HashCompareSizes", status);
            goto cleanup;
        }

        status = _HashRemoveElements(
            &hashTable,
            Params.Phases[i].ElemsToRemoveByKeyLinearlyThroughSkipping,
            shadowHash,
            true,
            false);
        if (!SUCCEEDED(status))
        {
            LOG_FUNC_ERROR("_HashRemoveElements", status);
            goto cleanup;
        }

        status = _HashCompareSizes(
            &hashTable,
            shadowHash);
        if (!SUCCEEDED(status))
        {
            LOG_FUNC_ERROR("_HashCompareSizes", status);
            goto cleanup;
        }

        status = _HashRemoveElements(
            &hashTable,
            Params.Phases[i].ElemsToRemoveByEntryLinearly,
            shadowHash,
            false,
            true);
        if (!SUCCEEDED(status))
        {
            LOG_FUNC_ERROR("_HashRemoveElements", status);
            goto cleanup;
        }

        status = _HashCompareSizes(
            &hashTable,
            shadowHash);
        if (!SUCCEEDED(status))
        {
            LOG_FUNC_ERROR("_HashCompareSizes", status);
            goto cleanup;
        }

        status = _HashRemoveElements(
            &hashTable,
            Params.Phases[i].ElemsToRemoveByEntryThroughSkipping,
            shadowHash,
            true,
            true);
        if (!SUCCEEDED(status))
        {
            LOG_FUNC_ERROR("_HashRemoveElements", status);
            goto cleanup;
        }

        status = _HashCompareSizes(
            &hashTable,
            shadowHash);
        if (!SUCCEEDED(status))
        {
            LOG_FUNC_ERROR("_HashCompareSizes", status);
            goto cleanup;
        }
    }

    status = _HashClear(
        &hashTable,
        shadowHash
    );
    if (!SUCCEEDED(status))
    {
        LOG_FUNC_ERROR("_HashClear", status);
        goto cleanup;
    }

cleanup:
    for (auto i = 0; i < NO_OF_PHASES; ++i)
    {
        if (elems[i] != nullptr)
        {
            delete[] elems[i];
            elems[i] = nullptr;
        }
    }

    return status;
}

STATUS
UtClHashTable()
{
    STATUS status = CL_STATUS_SUCCESS;

    for (const auto& keySize : KEY_SIZES)
    {
        for (const auto& hashFunc : HASH_FUNCTIONS)
        {
            for (const auto& ut : UT_PARAMS)
            {
                status = _UtClRunTestcase(
                    ut,
                    hashFunc,
                    keySize);
                if (!SUCCEEDED(status))
                {
                    LOG_ERROR("Failed test [%s] with key size %u and hash func 0x%p with status 0x%X\n",
                        ut.TestName.c_str(), keySize, hashFunc, status);
                    break;
                }
            }
        }
    }

    return status;
}
