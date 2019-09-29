#pragma once

#include "native/string.h"

#define LOG(buf,...)                            LogBuffer(buf,__VA_ARGS__)
#define LOGL(buf,...)                           LOG("[%s][%d]"##buf, strrchr(__FILE__,'\\') + 1, __LINE__, __VA_ARGS__)

#define LOG_WARNING(buf,...)                    LOGL("[WARNING]"##buf, __VA_ARGS__ )
#define LOG_ERROR(buf,...)                      LOGL("[ERROR]"##buf, __VA_ARGS__ )
#define LOG_FUNC_ERROR(func,status)             LOG_ERROR("Function %s failed with status 0x%x\n", (func), (status) )
#define LOG_FUNC_ERROR_ALLOC(func,size)         LOG_ERROR("Function %s failed alloc for size 0x%x\n", (func), (size))

#define LOG_TEST_PASS                           LOG("\n[PASS]\n");

void
LogBuffer(
    IN_Z    char*                   FormatBuffer,
    ...
    );

STATUS
UmThreadCreate(
    IN      PFUNC_ThreadStart       StartFunction,
    IN_OPT  PVOID                   Context,
    OUT     UM_HANDLE*              ThreadHandle
    );
