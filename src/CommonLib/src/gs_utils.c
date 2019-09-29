#ifdef CL_NO_RUNTIME_CHECKS
// warning C4206: nonstandard extension used: translation unit is empty
#pragma warning(disable:4206)
#else

#include "common_lib.h"

extern UINT64 __security_cookie;

void
GSNotifyStackChange(
    IN  PVOID       OldStackBase,
    IN  PVOID       NewStackBase,
    IN  DWORD       OldStackSize
    )
{
    PDWORD pOldStackCookie;
    QWORD usedStackSize;
    PVOID newRsp;

    // our algorithm works only if the stacks are page aligned
    ASSERT(IsAddressAligned(OldStackBase,PAGE_SIZE));
    ASSERT(IsAddressAligned(NewStackBase,PAGE_SIZE));

    newRsp = _AddressOfReturnAddress();
    usedStackSize = min(OldStackSize, PtrDiff(NewStackBase, newRsp));

    // Search [OldRsp, OldStackBase) for cookies
    for (pOldStackCookie = (PDWORD) PtrDiff(OldStackBase, usedStackSize - sizeof(DWORD));
         pOldStackCookie < (PDWORD) OldStackBase;
         pOldStackCookie += 2)
    {
        // we assume that if the upper DWORD of the cookie XORed with the upper DWORD of the old RSP
        // address is equal to the upper DWORD of the __security_cookie that we found a cookie
        if ((*pOldStackCookie ^ QWORD_HIGH(OldStackBase)) == QWORD_HIGH(__security_cookie))
        {
            // PtrDiff(OldStackBase, pCookie - 1) => Offset from stack top to the cookie
            // PtrDiff(NewStackBase, RES ) => Pointer to cookie on the new stack
            PQWORD pNewStackCookie = (PQWORD)PtrDiff(NewStackBase, PtrDiff(OldStackBase, pOldStackCookie - 1));

            // offsets within page coincide due to previous assumption (stack addresses are PAGE aligned)
            QWORD xorResult = AlignAddressLower(pOldStackCookie, PAGE_SIZE) ^ AlignAddressLower(pNewStackCookie, PAGE_SIZE);

            *pNewStackCookie = (*pNewStackCookie ^ xorResult);
        }
    }
}
#endif // CL_NO_RUNTIME_CHECKS
