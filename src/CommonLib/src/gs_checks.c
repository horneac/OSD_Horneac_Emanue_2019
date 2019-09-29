#ifdef CL_NO_RUNTIME_CHECKS

// warning C4206: nonstandard extension used: translation unit is empty
#pragma warning(disable:4206)
#else
#include "common_lib.h"

#define DEFAULT_SECURITY_COOKIE_VALUE       (QWORD)0xBEEF'0301'2497'EC03ULL

/// TODO: should we initialize the cookie at runtime?
const UINT64 __security_cookie = DEFAULT_SECURITY_COOKIE_VALUE;

// called when a buffer bound check fails
// E.g: char buf[10] buf[10] = 'A';
// From what I've seen in the disassembly no parameters
// passed => we can only print the location where the
// instruction occurred
__declspec(noreturn)
void
__report_rangecheckfailure(
    void
)
{
    // warning C4127: conditional expression is constant
#pragma warning(suppress:4127)
    ASSERT_INFO(FALSE, "RA is 0x%X\n", GET_RETURN_ADDRESS);
}

// NOT referenced anywhere in code
__declspec(noreturn)
void
__GSHandlerCheck_SEH(
    void
)
{
    NOT_REACHED;
}

// NOT referenced anywhere in code
__declspec(noreturn)
void
__GSHandlerCheck(
    void
)
{
    NOT_REACHED;
}

// Called for each function which uses a cookie
__declspec(noreturn)
void
__cdecl
__report_cookie_corruption(
    IN UINT64 StackCookie
)
{
    ASSERT_INFO(StackCookie == __security_cookie,
                "Security cookie is 0x%X but should have been 0x%X. RA is 0x%X\n",
                StackCookie, __security_cookie, GET_RETURN_ADDRESS);
}
#endif // CL_NO_RUNTIME_CHECKS
