#pragma once

C_HEADER_START
#ifndef CL_NON_NATIVE
#pragma warning(push)

// warning C4391: incorrect return type for intrinsic function
#pragma warning(disable:4391)

// annotations for some intrinsics
PVOID
_AddressOfReturnAddress(
    void
    );

void
__cpuid(
    OUT_WRITES_ALL(4)    int cpuInfo[4],
    IN                   int function_id
    );

void
__cpuidex(
    OUT_WRITES_ALL(4)    int cpuInfo[4],
    IN                   int function_id,
    IN                   int sub_id
    );

_Success_(TRUE)
QWORD
__readmsr(
    IN   DWORD   register
    );

void
__movsb(
    OUT_WRITES_BYTES_ALL(Count)                 PVOID   Destination,
    IN_READS_BYTES(Count)                       PVOID   Source,
    IN                                          QWORD   Count
    );

void
__movsq(
    OUT_WRITES_BYTES_ALL(Count*sizeof(QWORD))   PVOID   Destination,
    IN_READS_BYTES(Count*sizeof(QWORD))         PVOID   Source,
    IN                                          QWORD   Count
    );

_Success_(return == 0)
VMX_RESULT
__vmx_vmread(
    IN   DWORD  Field,
    OUT  QWORD* FieldValue
    );

_Success_(return == 0)
VMX_RESULT
__vmx_vmwrite(
    IN   DWORD  Field,
    IN   QWORD  FieldValue
    );

PVOID
__readcr2(
    void
    );

PHYSICAL_ADDRESS
__readcr3(
    void
    );

_Ret_range_(<=, 0xF)
BYTE
__readcr8(
    void
    );

void
__writecr8(
    IN_RANGE_UPPER(0xF) BYTE    Irql
    );

BYTE
_InterlockedExchange8(
    INOUT _Interlocked_operand_
        BYTE volatile * _Target,
    IN  BYTE _Value
    );

DWORD
_InterlockedIncrement(
    INOUT _Interlocked_operand_ DWORD volatile * _Addend
    );

BYTE
_InterlockedCompareExchange8(
    INOUT _Interlocked_operand_
        BYTE volatile * _Destination,
    IN  BYTE _Exchange,
    IN  BYTE _Comparand
    );

WORD
_InterlockedDecrement16(
    INOUT _Interlocked_operand_ WORD volatile *Destination
    );

DWORD _InterlockedCompareExchange(
    INOUT _Interlocked_operand_
        DWORD volatile * _Destination,
    IN  DWORD _Exchange,
    IN  DWORD _Comparand
    );

WORD
_InterlockedCompareExchange16(
    INOUT _Interlocked_operand_
        WORD volatile *Destination,
    IN  WORD ExChange,
    IN  WORD Comperand
    );

WORD
_InterlockedOr16(
    INOUT _Interlocked_operand_
        WORD volatile *Destination,
    IN  WORD Value
    );

WORD
_InterlockedIncrement16(
    INOUT _Interlocked_operand_ WORD volatile *Destination
    );

DWORD
_InterlockedDecrement(
    INOUT _Interlocked_operand_ DWORD volatile * _Addend
    );

_Success_(return == TRUE)
BOOLEAN
_rdrand16_step(
    OUT WORD*       Value
    );

_Success_(return == TRUE)
BOOLEAN
_rdrand32_step(
    OUT DWORD*      Value
    );

_Success_(return == TRUE)
BOOLEAN
_rdrand64_step(
    OUT QWORD*      Value
    );

#pragma warning(pop)
#endif // CL_NON_NATIVE
C_HEADER_END
