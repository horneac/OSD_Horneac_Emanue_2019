#include "hal_base.h"
#include "vmx.h"

#pragma warning(push)

// warning C28039: The type of actual parameter '24576|2048|((0<<1))|0' should exactly match the type 'VMCS_FIELD':
#pragma warning(disable: 28039)

#define VMX_RESULT_SUCCESS                  (VMX_RESULT)0
#define VMX_RESULT_FAIL_WITH_ERROR          (VMX_RESULT)1
#define VMX_RESULT_FAIL_NO_ERROR            (VMX_RESULT)2

typedef DWORD VM_INSTR_ERROR;

typedef enum _VM_INSTR_ERROR
{
    VmInstrErrorReserved                    = 0,

    VmInstrErrorVmCallInVmxRoot,
    VmInstrErrorVmClearWithInvalidPhysAddr,
    VmInstrErrorVmClearWithVmxonPointer,
    VmInstrErrorVmLaunchWithNonClearVmcs,
    VmInstrErrorVmResumeWithNonLaunchedVmcs,
    VmInstrErrorVmResumeAfterVmxOff,
    VmInstrErrorVmEntryWithInvalidControlFields,
    VmInstrErrorVmEntryWithInvalidHostStateFields,
    VmInstrErrorVmPtrldWithInvalidPhysAddr,
    VmInstrErrorVmPtrldWithVmxonPointer,
    VmInstrErrorVmPtrldWithIncorrectVmcsRevision,
    VmInstrErrorVmReadFromUnsupportedVmcsField,
    VmInstrErrorVmWriteToUnsupportedVmcsField = VmInstrErrorVmReadFromUnsupportedVmcsField,
    VmInstrErrorVmWriteToReadOnlyVmcsField,

    VmInstrErrorVmxonInVmxRoot = 15,
    VmInstrErrorVmEntryWithInvalidExecutiveVmcsPointer,
    VmInstrErrorVmEntryWithNonLaunchedExecutiveVmcs,
    VmInstrErrorVmEntryWithExecutiveVmcsPointerNotVmxonPointer,
    VmInstrErrorVmCallWithNonClearVmcs,
    VmInstrErrorVmCallWithInvalidVmExitControlFields,

    VmInstrErrorVmCallWithIncorrectMSEG = 22,
    VmInstrErrorVmxoffUnderDualMonitor,
    VmInstrErrorVmCallWithInvalidSmmFeatures,
    VmInstrErrorVmEntryWithInvalidVmExecutionFieldsInExecutiveVmcs,
    VmInstrErrorVmEntryWithEventsBlockedByMovSS,
    VmInstrErrorInvalidOperandToInveptInvvpid,

    VmInstrErrorInstrErrorNotValid           = MAX_DWORD
} VM_INSTR_ERROR;


static
__forceinline
VM_INSTR_ERROR
_VmxGetInstrError(
    IN _Strict_type_match_
            VMX_RESULT  Result
    )
{
    if (Result == VMX_RESULT_SUCCESS) return VmInstrErrorReserved;
    if (Result == VMX_RESULT_FAIL_NO_ERROR) return (VM_INSTR_ERROR) VmInstrErrorInstrErrorNotValid;

    return (VM_INSTR_ERROR) VmxRead(VMCS_EXIT_INSTRUCTION_ERROR);
}

QWORD
VmxRead(
    IN _Strict_type_match_
            VMCS_FIELD Field
    )
{
    QWORD fieldValue;

    VMX_RESULT vmxResult = __vmx_vmread(Field, &fieldValue);
    VM_INSTR_ERROR errCode = _VmxGetInstrError(vmxResult);

    ASSERT_INFO(
        errCode == VmInstrErrorReserved,
        "Failed VMREAD on field 0x%08x with error code %u\n",
        Field, errCode);

    return fieldValue;
}

void
VmxWrite(
    IN _Strict_type_match_
            VMCS_FIELD  Field,
    IN      QWORD       Value
    )
{
    VMX_RESULT vmxResult = __vmx_vmwrite(Field, Value);
    VM_INSTR_ERROR errCode = _VmxGetInstrError(vmxResult);

    ASSERT_INFO(
        errCode == VmInstrErrorReserved,
        "Failed VMRITE on field 0x%08x with value 0x%X with error code %u\n",
        Field, Value, errCode);
}

#pragma warning(pop)
