#pragma once
//******************************************************************************
// Hash table
//
//
// The hash table implementation uses a user provided hash function and solves
// collisions by chaining the elements which hash to the same key. This
// implementation requires dynamically allocated memory only when initializing
// the hash table structure, this is because the number of maximum keys in
// the hash table is determined only when initializing the hash table.
//
// However, after the hash table is initialized there is no more need for
// dynamically allocated memory, the scheme is the same as for LIST_ENTRY,
// each element to be inserted in the hash contains a HASH_ENTRY field which
// is used to link all the entries within a single key.
//
// The hash function is also provided by the user, also the user must specify
// the offset difference in the structure between the HASH_ENTRY field and
// the field which will be used as the key.
//
// Lets see a usage example: we have our own structure FOO which has the
// a WORD element named Id which will be used as the key and it has a
// HASH_ENTRY element named HashEntry which is used in the hash table.
//
// typedef struct _FOO
// {
//      DWORD           SomeData;
//      WORD            Id;
//      DWORD           MoreData;
//      HASH_ENTRY      HashEntry;
// } FOO, *PFOO;
//
// We first pre-initialize the hash table by specifying how many keys we
// want to have (the number of keys does not restrict the maximum number of
// elements):
//
// HASH_TABLE hashTable;
// PHASH_TABLE_DATA pHashData;
//
// DWORD dataSize = HashTablePreInit(&hashTable, 16, sizeof(WORD));
//
// Now we need to allocate dataSize bytes and pass the buffer to HashTableInit,
// we will also pass a hash function (we can select one of the already existent
// ones) and the offset in bytes between the Id field and the HashEntry field.
//
// pHashData = malloc(dataSize);
//
// HashTableInit(&hashTable,
//              pHashData,
//              HashFuncUniversal,
//              FIELD_OFFSET(FOO, Id) - FIELD_OFFSET(FOO, HashEntry));
//
// Now we can commence operations on the hash table:
//
// 1. Insertion
//
// PFOO pMyData;
//
// HashTableInsert(&hashTable, &pMyData->HashEntry);
//
// 2. Lookup
//
// PHASH_ENTRY pEntry;
// WORD idToSearchFor = 0x33;
//
// pEntry = HashTableLookup(&hashTable, &idToSearchFor);
// if (pEntry != NULL)
// {
//      PFOO pMyData = CONTAINING_RECORD(pEntry, FOO, HashEntry);
// }
//
// 3. Delete by entry
//
// HashTableRemoveEntry(&hashTable, &pMyData->HashEntry);
//
// 4. Delete by value
//
// WORD idToRemove = 0x33;
// PHASH_ENTRY pEntry;
//
// pEntry = HashTableRemove(&hashTable, &idToRemove);
//
// 5. Iteration
//
// HASH_TABLE_ITERATOR it;
// PHASH_ENTRY pEntry;
//
// HashTableIteratorInit(&hashTable, &it);
//
// while ((pEntry = HashTableIteratorNext(&it)) != NULL)
// {
//      // do whatever with the element
//      // we can safely remove it from the hash table
// }
//******************************************************************************

C_HEADER_START
#include "list.h"
#include "ref_cnt.h"

typedef struct _HASH_TABLE_DATA*    PHASH_TABLE_DATA;
typedef struct _HASH_ITERATOR*      PHASH_ITERATOR;
typedef struct _HASH_KEY*           PHASH_KEY;

typedef LIST_ENTRY                  HASH_ENTRY, *PHASH_ENTRY;

typedef
QWORD
(__cdecl FUNC_HashFunction) (
    IN_READS_BYTES(KeyLength)   PHASH_KEY   Key,
    IN                          DWORD       KeyLength,
    IN                          DWORD       MaxKeys
    );

typedef FUNC_HashFunction *PFUNC_HashFunction;

typedef struct _HASH_TABLE
{
    // The universe of keys U = {0, ... MaxKeys - 1}
    DWORD                       MaxKeys;

    // Size of the key in bytes, maximum is currently 8 bytes
    DWORD                       KeySize;

    // The number of elements currently found in the hash table
    DWORD                       NumberOfElements;

    // The offset difference in bytes between the HASH_ENTRY and the key
    INT32                       OffsetToKey;

    PFUNC_HashFunction          HashFunc;

    // Private data used by the hash table internal implementation
    PHASH_TABLE_DATA            TableData;
} HASH_TABLE, *PHASH_TABLE;

typedef struct _HASH_ITERATOR
{
    PHASH_TABLE                 HashTable;

    DWORD                       KeyIndex;
    LIST_ITERATOR               CurrentKeyIterator;
} HASH_ITERATOR, *PHASH_ITERATOR;

//******************************************************************************
// Function:     HashTablePreinit
// Description:  Pre-initializes a hash table with maximum MaxKeys, each key
//               occupying KeySize bytes.
// Returns:      DWORD - The number of bytes which need to be allocated for the
//                       internal TableData structure.
// Parameter:    OUT PHASH_TABLE HashTable
// Parameter:    IN DWORD MaxKeys - Maximum number of unique keys
// Parameter:    IN DWORD KeySize - The length in bytes of a key - required for
//               the hashing functions
//******************************************************************************
DWORD
HashTablePreinit(
    OUT     PHASH_TABLE         HashTable,
    IN      DWORD               MaxKeys,
    IN      DWORD               KeySize
    );

//******************************************************************************
// Function:     HashTableInit
// Description:  Initializes a pre-initialized hash table, using TableData for
//               its internal house keeping.
// Returns:      void
// Parameter:    INOUT PHASH_TABLE HashTable
// Parameter:    IN PHASH_TABLE_DATA TableData
// Parameter:    IN PFUNC_HashFunction HashFunction - Function which performs
//                                                    the hash on the key
// Parameter:    IN INT32 OffsetToKey - Difference in bytes between the offset
//               of the HASH_ENTRY element and the key. Generically this can be
//               calculated using the following formula:
//               FIELD_OFFSET(MY_STRUCT,Key) - FIELD_OFFSET(MY_STRUCT,HashEntry)
//******************************************************************************
void
HashTableInit(
    INOUT   PHASH_TABLE         HashTable,
    OUT_WRITES_BYTES(HashTable->MaxKeys * sizeof(HASH_ENTRY))
            PHASH_TABLE_DATA    TableData,
    IN      PFUNC_HashFunction  HashFunction,
    IN      INT32               OffsetToKey
    );

//******************************************************************************
// Function:     HashTableClear
// Description:  Removes all the elements from the hash table, optionally
//               calling a free function for each element: this function
//               receives a pointer to the HASH_ENTRY field of the element.
// Returns:      void
// Parameter:    INOUT PHASH_TABLE HashTable
// Parameter:    IN_OPT PFUNC_FreeFunction FreeFunction
// Parameter:    IN_OPT PVOID FreeContext
//******************************************************************************
void
HashTableClear(
    INOUT   PHASH_TABLE         HashTable,
    IN_OPT  PFUNC_FreeFunction  FreeFunction,
    IN_OPT  PVOID               FreeContext
    );

//******************************************************************************
// Function:     HashTableSize
// Description:
// Returns:      DWORD - Number of elements in the hash table
// Parameter:    IN HASH_TABLE* HashTable
//******************************************************************************
DWORD
HashTableSize(
    IN      HASH_TABLE*         HashTable
    );

//******************************************************************************
// Function:     HashTableInsert
// Description:  Inserts a new element into the hash table. If a collision
//               occurs the new element is chained with all the others which
//               have the same key.
// Returns:      PHASH_ENTRY - If the Element already exists it returns the
//                             previous entry, NULL otherwise
// Parameter:    INOUT PHASH_TABLE HashTable
// Parameter:    INOUT PHASH_ENTRY Element
//******************************************************************************
PHASH_ENTRY
HashTableInsert(
    INOUT   PHASH_TABLE         HashTable,
    INOUT   PHASH_ENTRY         Element
    );

//******************************************************************************
// Function:     HashTableRemove
// Description:  Removes from the hash table the element with key Key.
// Returns:      PHASH_ENTRY - Pointer to the HASH_ENTRY field in the found
//                             element
//                             NULL if no element with Key present
// Parameter:    INOUT PHASH_TABLE HashTable
// Parameter:    IN PHASH_KEY Key - Key to remove from the hash table.
//******************************************************************************
PTR_SUCCESS
PHASH_ENTRY
HashTableRemove(
    INOUT   PHASH_TABLE         HashTable,
    IN      PHASH_KEY           Key
    );

//******************************************************************************
// Function:     HashTableRemoveEntry
// Description:  Removes an element from the hash table.
// Returns:      void
// Parameter:    INOUT PHASH_TABLE HashTable
// Parameter:    IN PHASH_ENTRY Element
//******************************************************************************
void
HashTableRemoveEntry(
    INOUT   PHASH_TABLE         HashTable,
    IN      PHASH_ENTRY         Element
    );

//******************************************************************************
// Function:     HashTableLookup
// Description:  Searches for the element with key Key in the hash table.
// Returns:      PHASH_ENTRY - A pointer to the HASH_ENTRY field in the found
//                             element
//                             NULL if no element with Key present
// Parameter:    INOUT PHASH_TABLE HashTable
// Parameter:    IN PHASH_KEY Key
//******************************************************************************
PTR_SUCCESS
PHASH_ENTRY
HashTableLookup(
    INOUT   PHASH_TABLE         HashTable,
    IN      PHASH_KEY           Key
    );

//******************************************************************************
// Function:     HashTableIteratorInit
// Description:  Initializes an iterator over the hash table.
// Returns:      void
// Parameter:    INOUT PHASH_TABLE HashTable
// Parameter:    OUT PHASH_ITERATOR HashIterator
// NOTE:         There is no guarantee regarding the order in which the hash
//               table is traversed.
//******************************************************************************
void
HashTableIteratorInit(
    IN      PHASH_TABLE         HashTable,
    OUT     PHASH_ITERATOR      HashIterator
    );

//******************************************************************************
// Function:     HashTableIteratorNext
// Description:  Returns the next element in the hash table.
// Returns:      PHASH_ENTRY - A pointer to the HASH_ENTRY field in the found
//                             element
//                             NULL if there are no more elements
// Parameter:    INOUT PHASH_ITERATOR HashIterator
//******************************************************************************
PHASH_ENTRY
HashTableIteratorNext(
    INOUT   PHASH_ITERATOR      HashIterator
    );

// Generic Hash functions which you can use for keys of size up to 8 bytes

// 1. HashFuncGenericIncremental: Key % MaxKeys
// Good for hashes where the keys simply increment by 1: 1, 2, 3, 4, 5 =>
// these elements will be distributed evenly in each chain
FUNC_HashFunction               HashFuncGenericIncremental;

// 2. HashFuncUniversal: (((a * Key) + b) % p) % MaxKeys
// Good for all the other cases.
FUNC_HashFunction               HashFuncUniversal;
C_HEADER_END
