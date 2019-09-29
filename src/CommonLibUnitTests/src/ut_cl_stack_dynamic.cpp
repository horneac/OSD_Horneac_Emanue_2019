#include "ut_base.h"
#include "ut_cl_stack_dynamic.h"
#include "stack_interface.h"
#include <stack>
#include "ut_cl_rng.h"

typedef struct _UT_STACK_ELEM
{
    STACK_ITEM                  Entry;

    DWORD                       Value;
} UT_STACK_ELEM, *PUT_STACK_ELEM;

typedef struct _UT_PHASE
{
    DWORD                       ElemsToInsert;
    DWORD                       ElemsToPop;
} UT_PHASE, *PUT_PHASE;

static constexpr auto NO_OF_PHASES = 2;

typedef struct _STACK_UT_PARAMS
{
    const std::string           TestName;

    UT_PHASE                    Phases[NO_OF_PHASES];
} STACK_UT_PARAMS, *PSTACK_UT_PARAMS;

static const STACK_UT_PARAMS UT_PARAMS[] =
{
    {"No elems", 0, 0, 0, 0},
    {"One insert, one pop", 1, 1, 0, 0},
    {"Two inserts, one pop", 2, 1, 2, 1},
    {"Inserts with no pops", 100, 0, 100, 0},
    {"Inserts with not enough pops", 100, 99, 100, 99},
    {"Many inserts with no pops", 3'000'000, 0, 0, 0},
    {"Just pops", 0, 1, 0, 10},
    {"Many Elems", 1'000'000, 900'000, 2'000'000, 2'210'001},
    {"Huge Elems", 100'000'000, 100'000'010, 200'000'000, 100'000'010},
};

static
STATUS
_StackCreate(
    _In_    STACK_TYPE          Type,
    _In_    DWORD               MaxElements,
    _Out_   STACK_INTERFACE*    StackInterface
    )
{
    DWORD stackSize = StackGetRequiredSize(MaxElements, Type);

    if (stackSize == 0) return CL_STATUS_SIZE_INVALID;

    PSTACK pStack = (PSTACK) new BYTE[stackSize];

    return StackCreate(
        StackInterface,
        Type,
        pStack);
}

static
STATUS
_StackInsertElements(
    _Inout_     STACK_INTERFACE*    StackInterface,
    _In_        DWORD               ElementsToInsert,
    _Outref_    PUT_STACK_ELEM&        StackElems,
    _Inout_     std::stack<DWORD>&  ShadowStack
    )
{
    BOOLEAN bSuccess = TRUE;
    UtCl::RNG rngInstance = UtCl::RNG::GetInstance();

    ASSERT(StackInterface != nullptr);

    PUT_STACK_ELEM pStackElems = new UT_STACK_ELEM[ElementsToInsert];

    for (DWORD i = 0; i < ElementsToInsert; ++i)
    {
        pStackElems[i].Value = rngInstance.GetNextRandom();

        ShadowStack.push(pStackElems[i].Value);
        bSuccess = StackInterface->Funcs.Push(StackInterface->Stack, &pStackElems[i].Entry);
        if (!bSuccess)
        {
            LOG_ERROR("Failed to insert element %u/%u. Stack size at this point is %u, shadow stack size is %zu\n",
                i, ElementsToInsert, StackInterface->Funcs.Size(StackInterface->Stack), ShadowStack.size());
            break;
        }
    }

    StackElems = pStackElems;

    return (bSuccess ? CL_STATUS_SUCCESS : CL_STATUS_LIMIT_REACHED);
}

static
STATUS
_StackPopElements(
    _Inout_ STACK_INTERFACE*    StackInterface,
    _In_    DWORD               ElementsToPop,
    _Inout_ std::stack<DWORD>&  ShadowStack
    )
{
    STATUS status = CL_STATUS_SUCCESS;

    for (DWORD i = 0; i < ElementsToPop; ++i)
    {
        PSTACK_ITEM pStackItem = StackInterface->Funcs.Pop(StackInterface->Stack);
        PUT_STACK_ELEM pStackElem = nullptr;

        bool bShadowStackEmpty = ShadowStack.empty();
        DWORD value = 0;

        if (pStackItem != nullptr)
        {
            pStackElem = CONTAINING_RECORD(pStackItem, UT_STACK_ELEM, Entry);
            value = pStackElem->Value;
        }

        if (bShadowStackEmpty)
        {
            if (pStackElem != nullptr)
            {
                LOG_ERROR("The shadow stack is empty, however we were able to remove element at %p with value %u from stack!\n",
                    pStackElem, value);
                status = CL_STATUS_ELEMENT_FOUND;
            }
        }
        else
        {
            DWORD shadowStackValue = ShadowStack.top();

            if (pStackElem == nullptr)
            {
                LOG_ERROR("The stack is empty, however the shadow stack still has %I64u elements, with the current one being %u!\n",
                    ShadowStack.size(), shadowStackValue);
                status = CL_STATUS_LIST_EMPTY;
            }
            else if (value != shadowStackValue)
            {
                LOG_ERROR("Values at the top of the stack differs for element at %p, value is %u while on shadow stack value is %u\n",
                    pStackElem, value, shadowStackValue);
                status = CL_STATUS_VALUE_MISMATCH;
            }

            ShadowStack.pop();
        }

        if (!SUCCEEDED(status)) break;
    }

    return status;
}

static
STATUS
_StackCompareSizes(
    _In_ const STACK_INTERFACE*    StackInterface,
    _In_ const std::stack<DWORD>&  ShadowStack
    )
{
    DWORD stackSize;
    size_t shadowStackSize;

    ASSERT(StackInterface != nullptr);

    stackSize = StackInterface->Funcs.Size(StackInterface->Stack);
    shadowStackSize = ShadowStack.size();
    ASSERT(shadowStackSize <= MAX_DWORD);

    if (stackSize == shadowStackSize) return CL_STATUS_SUCCESS;

    LOG_ERROR("Our reported stack size is %u, while the shadow stack size is %zu\n",
        stackSize, shadowStackSize);

    return CL_STATUS_SIZE_INVALID;
}

static
void
(_cdecl _StackFreeItem)(
    IN      PVOID       Object,
    IN_OPT  PVOID       Context
    )
{
    std::vector<DWORD> *vect = (std::vector<DWORD>*) Context;

    ASSERT(Object != nullptr);
    ASSERT(Context != nullptr);

    PUT_STACK_ELEM pStackElem = CONTAINING_RECORD(Object, UT_STACK_ELEM, Entry);

    vect->push_back(pStackElem->Value);
}

static
STATUS
_StackClear(
    _Inout_ STACK_INTERFACE*    StackInterface,
    _Inout_ std::stack<DWORD>&  ShadowStack
    )
{
    STATUS status;
    std::vector<DWORD> removedElems;

    ASSERT(StackInterface != nullptr);

    status = CL_STATUS_SUCCESS;

    StackInterface->Funcs.Clear(
        StackInterface->Stack,
        _StackFreeItem,
        &removedElems);

    for (const auto& value : removedElems)
    {
        bool bShadowStackEmpty = ShadowStack.empty();

        if (bShadowStackEmpty)
        {
            LOG_ERROR("The shadow stack is empty, however we were able to remove element with value %u from stack!\n",
                value);
            status = CL_STATUS_ELEMENT_FOUND;
        }
        else
        {
            DWORD shadowStackValue = ShadowStack.top();

            if (value != shadowStackValue)
            {
                LOG_ERROR("Values at the top of the stack differ: value is %u while on shadow stack value is %u\n",
                    value, shadowStackValue);
                status = CL_STATUS_VALUE_MISMATCH;
            }

            ShadowStack.pop();
        }

        if (!SUCCEEDED(status)) break;
    }

    if (SUCCEEDED(status))
    {
        if (!ShadowStack.empty())
        {
            LOG_ERROR("We have cleared our whole stack, however the shadow stack still has %zu elements!\n",
                ShadowStack.size());
            return CL_STATUS_SIZE_INVALID;
        }
    }

    return status;
}

STATUS
UtClStackDynamic()
{
    STATUS status = CL_STATUS_SUCCESS;

    for (const auto& ut : UT_PARAMS)
    {
        STACK_INTERFACE stackInterface;
        std::stack<DWORD> shadowStack;
        PUT_STACK_ELEM elems[NO_OF_PHASES] = {};

        status = _StackCreate(
            StackTypeDynamic,
            MAX_DWORD,
            &stackInterface);
        if (!SUCCEEDED(status))
        {
            LOG_FUNC_ERROR("_UtClStackCreate", status);
            goto cleanup;
        }

        for (auto i = 0; i < NO_OF_PHASES; ++i)
        {
            status = _StackInsertElements(
                &stackInterface,
                ut.Phases[i].ElemsToInsert,
                elems[i],
                shadowStack);
            if (!SUCCEEDED(status))
            {
                LOG_FUNC_ERROR("_StackInsertElements", status);
                goto cleanup;
            }

            status = _StackCompareSizes(
                &stackInterface,
                shadowStack);
            if (!SUCCEEDED(status))
            {
                LOG_FUNC_ERROR("_StackCompareSizes", status);
                goto cleanup;
            }

            status = _StackPopElements(
                &stackInterface,
                ut.Phases[i].ElemsToPop,
                shadowStack);
            if (!SUCCEEDED(status))
            {
                LOG_FUNC_ERROR("_StackInsertElements", status);
                goto cleanup;
            }

            status = _StackCompareSizes(
                &stackInterface,
                shadowStack);
            if (!SUCCEEDED(status))
            {
                LOG_FUNC_ERROR("_StackCompareSizes", status);
                goto cleanup;
            }
        }

        status = _StackClear(
            &stackInterface,
            shadowStack
            );
        if (!SUCCEEDED(status))
        {
            LOG_FUNC_ERROR("_StackClear", status);
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

        if (!SUCCEEDED(status))
        {
            LOG_ERROR("Failed test [%s] with status 0x%X\n",
                ut.TestName.c_str(), status);
            break;
        }

    }

    return status;
}
