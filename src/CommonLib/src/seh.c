#ifdef CL_NO_RUNTIME_CHECKS
// warning C4206: nonstandard extension used: translation unit is empty
#pragma warning(disable:4206)
#else

#include "common_lib.h"
#include "seh.h"

EXCEPTION_DISPOSITION
__C_specific_handler(
    _In_    struct _EXCEPTION_RECORD*   ExceptionRecord,
    _In_    void*                       EstablisherFrame,
    _Inout_ struct _CONTEXT*            ContextRecord,
    _Inout_ struct _DISPATCHER_CONTEXT* DispatcherContext
)
{
    UNREFERENCED_PARAMETER(ExceptionRecord);
    UNREFERENCED_PARAMETER(EstablisherFrame);
    UNREFERENCED_PARAMETER(ContextRecord);
    UNREFERENCED_PARAMETER(DispatcherContext);

    NOT_REACHED;

    return ExceptionContinueExecution;
}
#endif // CL_NO_RUNTIME_CHECKS
