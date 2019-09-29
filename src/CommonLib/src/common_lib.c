#include "common_lib.h"
#include "lock_common.h"

STATUS
CommonLibInit(
    IN      PCOMMON_LIB_INIT        InitSettings
    )
{
    STATUS status;

    if (NULL == InitSettings)
    {
        return STATUS_INVALID_PARAMETER1;
    }

    if (InitSettings->Size != sizeof(COMMON_LIB_INIT))
    {
        return STATUS_INCOMPATIBLE_INTERFACE;
    }

    status = STATUS_SUCCESS;

#ifndef _COMMONLIB_NO_LOCKS_
    LockSystemInit(InitSettings->MonitorSupport);
#endif // _COMMONLIB_NO_LOCKS_

    AssertSetFunction(InitSettings->AssertFunction);

    return status;
}