#pragma once

#include "native/string.h"
#include "cpu.h"
//#define NO_COMM

#ifndef NO_COMM

#define LOG_ATC(lvl,comp,buf,...)               LogEx((lvl),(comp),buf,__VA_ARGS__)

#define LOG_AT(lvl,buf,...)                     LOG_ATC((lvl),LogComponentGeneric,(buf),__VA_ARGS__)
#define LOG_ATLC(lvl,comp,buf,...)              LOG_ATC((lvl),(comp),"[%s][%d]"##buf, strrchr(__FILE__,'\\') + 1, __LINE__, __VA_ARGS__)

#define LOG(buf,...)                            LOG_AT(LogLevelInfo, buf, __VA_ARGS__)
#define LOGP(buf,...)                           LOG_AT(LogLevelInfo, "[CPU:%02x]"##buf, CpuGetApicId(), __VA_ARGS__ )
#define LOGP_WARNING(buf,...)                   LOG_ATLC(LogLevelWarning, LogComponentGeneric, "[CPU:%02x]"##buf, CpuGetApicId(), __VA_ARGS__ )
#define LOGP_ERROR(buf,...)                     LOG_ATLC(LogLevelError, LogComponentGeneric, "[CPU:%02x]"##buf, CpuGetApicId(), __VA_ARGS__ )

#define LOGL(buf,...)                           LOG_ATLC( LogLevelInfo, LogComponentGeneric, buf, __VA_ARGS__ )
#define LOGPL(buf,...)                          LOG_ATLC( LogLevelInfo, LogComponentGeneric, "[CPU:%02x]"##buf, CpuGetApicId(), __VA_ARGS__ )
#define LOGTPL(buf,...)                         LOGPL("[TH:%s]"##buf, ThreadGetName(NULL), __VA_ARGS__)

#define LOG_TRACE(buf,...)                      LOG_ATLC( LogLevelTrace, LogComponentGeneric, "[CPU:%02x]"##buf, CpuGetApicId(), __VA_ARGS__ )
#define LOG_TRACE_COMP(comp,buf,...)            LOG_ATLC( LogLevelTrace, (comp), "[CPU:%02x][" #comp "]" ##buf, CpuGetApicId(), __VA_ARGS__ )

#define LOG_TRACE_IO(buf,...)                   LOG_TRACE_COMP(LogComponentIo, buf, __VA_ARGS__ )
#define LOG_TRACE_EXCEPTION(buf,...)            LOG_TRACE_COMP(LogComponentException, buf, __VA_ARGS__ )
#define LOG_TRACE_INTERRUPT(buf,...)            LOG_TRACE_COMP(LogComponentInterrupt, buf, __VA_ARGS__ )
#define LOG_TRACE_VMM(buf,...)                  LOG_TRACE_COMP(LogComponentVmm, buf, __VA_ARGS__ )
#define LOG_TRACE_MMU(buf,...)                  LOG_TRACE_COMP(LogComponentMmu, buf, __VA_ARGS__ )
#define LOG_TRACE_CPU(buf,...)                  LOG_TRACE_COMP(LogComponentCpu, buf, __VA_ARGS__ )
#define LOG_TRACE_ACPI(buf,...)                 LOG_TRACE_COMP(LogComponentAcpi, buf, __VA_ARGS__ )
#define LOG_TRACE_THREAD(buf,...)               LOG_TRACE_COMP(LogComponentThread, buf, __VA_ARGS__ )
#define LOG_TRACE_STORAGE(buf,...)              LOG_TRACE_COMP(LogComponentStorage, buf, __VA_ARGS__ )
#define LOG_TRACE_FILESYSTEM(buf,...)           LOG_TRACE_COMP(LogComponentFileSystem, buf, __VA_ARGS__ )
#define LOG_TRACE_NETWORK(buf,...)              LOG_TRACE_COMP(LogComponentNetwork, buf, __VA_ARGS__ )
#define LOG_TRACE_USERMODE(buf,...)             LOG_TRACE_COMP(LogComponentUserMode, buf, __VA_ARGS__ )
#define LOG_TRACE_PROCESS(buf,...)              LOG_TRACE_COMP(LogComponentProcess, buf, __VA_ARGS__)
#define LOG_TRACE_PCI(buf,...)                  LOG_TRACE_COMP(LogComponentPci, buf, __VA_ARGS__)

#define LOG_WARNING(buf,...)                    LOG_ATLC( LogLevelWarning, LogComponentGeneric, buf, __VA_ARGS__ )

#define LOG_ERROR(buf,...)                      LOG_ATLC( LogLevelError, LogComponentGeneric, buf, __VA_ARGS__ )
#define LOG_FUNC_ERROR(func,status)             LOG_ERROR("Function %s failed with status 0x%x\n", (func), (status) )
#define LOG_FUNC_ERROR_ALLOC(func,size)         LOG_ERROR("Function %s failed alloc for size 0x%x\n", (func), (size))

#define LOG_FUNC_START                          LOG_TRACE("Entering function %s\n", __FUNCTION__)
#define LOG_FUNC_END                            LOG_TRACE("Leaving function %s\n", __FUNCTION__)

#define LOG_FUNC_START_CPU                      LOG_FUNC_START
#define LOG_FUNC_END_CPU                        LOG_FUNC_END

#define LOG_FUNC_START_THREAD                   LOG_TRACE("[TH:%s]Entering function %s\n", ThreadGetName(NULL), __FUNCTION__)
#define LOG_FUNC_END_THREAD                     LOG_TRACE("[TH:%s]Leaving function %s\n", ThreadGetName(NULL), __FUNCTION__)
#else
#define LOG_AT(lvl,buf,...)
#define LOG_ATL(lvl,buf,...)

#define LOG(buf,...)
#define LOGP(buf,...)
#define LOGP_ERROR(buf,...)

#define LOGL(buf,...)
#define LOGPL(buf,...)
#define LOGTPL(buf,...)

#define LOG_TRACE(buf,...)
#define LOG_TRACE_COMP(comp,buf,...)

#define LOG_TRACE_IO(buf,...)
#define LOG_TRACE_EXCEPTION(buf,...)
#define LOG_TRACE_INTERRUPT(buf,...)
#define LOG_TRACE_VMM(buf,...)
#define LOG_TRACE_MMU(buf,...)
#define LOG_TRACE_CPU(buf,...)
#define LOG_TRACE_ACPI(buf,...)
#define LOG_TRACE_THREAD(buf,...)
#define LOG_TRACE_STORAGE(buf,...)
#define LOG_TRACE_FILESYSTEM(buf,...)
#define LOG_TRACE_NETWORK(buf,...)
#define LOG_TRACE_USERMODE(buf,...)
#define LOG_TRACE_PROCESS(buf,...)
#define LOG_TRACE_PCI(buf,...)

#define LOG_WARNING(buf,...)

#define LOG_ERROR(buf,...)
#define LOG_FUNC_ERROR(func,status)
#define LOG_FUNC_ERROR_ALLOC(func,size)

#define LOG_FUNC_START
#define LOG_FUNC_END

#define LOG_FUNC_START_CPU
#define LOG_FUNC_END_CPU

#define LOG_FUNC_START_THREAD
#define LOG_FUNC_END_THREAD

#endif

typedef enum _LOG_LEVEL
{
    LogLevelTrace,
    LogLevelInfo,
    LogLevelWarning,
    LogLevelError
} LOG_LEVEL;
STATIC_ASSERT_INFO(sizeof(LOG_LEVEL) == sizeof(DWORD), "We are using _InterlockedExchange for levels!");

typedef enum _LOG_COMPONENT
{
    LogComponentGeneric     = 0b0000'0000'0000'0000'0000'0000'0000'0001,
    LogComponentIo          = 0b0000'0000'0000'0000'0000'0000'0000'0010,
    LogComponentException   = 0b0000'0000'0000'0000'0000'0000'0000'0100,
    LogComponentInterrupt   = 0b0000'0000'0000'0000'0000'0000'0000'1000,
    LogComponentVmm         = 0b0000'0000'0000'0000'0000'0000'0001'0000,
    LogComponentMmu         = 0b0000'0000'0000'0000'0000'0000'0010'0000,
    LogComponentCpu         = 0b0000'0000'0000'0000'0000'0000'0100'0000,
    LogComponentAcpi        = 0b0000'0000'0000'0000'0000'0000'1000'0000,
    LogComponentThread      = 0b0000'0000'0000'0000'0000'0001'0000'0000,
    LogComponentStorage     = 0b0000'0000'0000'0000'0000'0010'0000'0000,
    LogComponentFileSystem  = 0b0000'0000'0000'0000'0000'0100'0000'0000,
    LogComponentNetwork     = 0b0000'0000'0000'0000'0000'1000'0000'0000,
    LogComponentUserMode    = 0b0000'0000'0000'0000'0001'0000'0000'0000,
    LogComponentProcess     = 0b0000'0000'0000'0000'0010'0000'0000'0000,
    LogComponentPci         = 0b0000'0000'0000'0000'0100'0000'0000'0000,
    LogComponentTest        = 0b0000'0000'0000'0000'1000'0000'0000'0000,

    LogComponentAll         = 0b1111'1111'1111'1111'1111'1111'1111'1111
} _Enum_is_bitflag_ LOG_COMPONENT;
STATIC_ASSERT_INFO(sizeof(LOG_COMPONENT) == sizeof(DWORD), "We are using _InterlockedExchange for components!");

_No_competing_thread_
void
LogSystemPreinit(
    void
    );

_No_competing_thread_
void
LogSystemInit(
    IN _Strict_type_match_
                LOG_LEVEL       LogLevel,
    IN
                LOG_COMPONENT   LogComponenets,
    IN          BOOLEAN         Enable
    );

#define Log(lvl,buf,...)        LogEx((lvl),LogComponentGeneric,(buf), __VA_ARGS__)

void
LogEx(
    IN _Strict_type_match_
                LOG_LEVEL       LogLevel,
    IN
                LOG_COMPONENT   LogComponent,
    IN_Z        char*           FormatBuffer,
    ...
    );

BOOLEAN
LogSetState(
    IN          BOOLEAN     Enable
    );

LOG_LEVEL
LogGetLevel(
    void
    );

LOG_LEVEL
LogSetLevel(
    IN          LOG_LEVEL   NewLogLevel
    );

LOG_COMPONENT
LogGetTracedComponents(
    void
    );

LOG_COMPONENT
LogSetTracedComponents(
    IN          LOG_COMPONENT   Components
    );

#define LogIsComponentTraced(Comp)      (LogGetLevel() <= LogLevelTrace && IsFlagOn(LogGetTracedComponents(), (Comp)))
