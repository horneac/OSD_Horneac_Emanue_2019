#pragma once

#include "vmcs_fields.h"
#include "vmx_exit.h"

QWORD
VmxRead(
    IN _Strict_type_match_
            VMCS_FIELD Field
    );

void
VmxWrite(
    IN _Strict_type_match_
            VMCS_FIELD  Field,
    IN      QWORD       Value
    );
