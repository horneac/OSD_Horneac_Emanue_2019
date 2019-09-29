#include "common_lib.h"
#include "ref_cnt.h"
#include "cl_memory.h"

void
RfcPreInit(
    OUT     REF_COUNT*              Object
    )
{
    ASSERT(NULL != Object);

    memzero(Object, sizeof(REF_COUNT));
}

STATUS
RfcInit(
    OUT     REF_COUNT*              Object,
    IN_OPT  PFUNC_FreeFunction      FreeFunction,
    IN_OPT  PVOID                   Context
    )
{
    STATUS status;

    if (NULL == Object)
    {
        return STATUS_INVALID_PARAMETER1;
    }

    status = STATUS_SUCCESS;

    Object->FreeFunction = FreeFunction;
    Object->Context = Context;
    Object->ReferenceCount = 1;

    return status;
}

SIZE_SUCCESS
DWORD
RfcReference(
    INOUT   REF_COUNT*              Object
    )
{
    DWORD newRefCount;

    ASSERT(NULL != Object);

    newRefCount = (DWORD)_InterlockedIncrement(&Object->ReferenceCount);
    ASSERT_INFO(MAX_DWORD > newRefCount, "Reached max reference count");
    ASSERT_INFO( 1 <= newRefCount,
                "Inexistent object with %u references referenced",
                newRefCount - 1
                );

    return newRefCount;
}

SIZE_SUCCESS
DWORD
RfcDereference(
    INOUT   REF_COUNT*              Object
    )
{
    DWORD newRefCount;

    ASSERT(NULL != Object);

    newRefCount = (DWORD)_InterlockedDecrement(&Object->ReferenceCount);
    ASSERT_INFO(MAX_DWORD != newRefCount, "Object reference count reached -1");

    if (0 == newRefCount)
    {
        if (NULL != Object->FreeFunction)
        {
            Object->FreeFunction(Object, Object->Context);
        }
    }

    return newRefCount;
}