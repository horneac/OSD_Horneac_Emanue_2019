#pragma once

#include "cpu.h"
#include "apic_common.h"

void
LapicInitialize(
    IN      PVOID                           ApicBaseAddress
    );

void
LapicSetLogicalApicId(
    IN      PVOID                           ApicBaseAddress,
    IN      _Strict_type_match_
            APIC_ID                         LogicalApicId,
    IN      BYTE                            DestinationFormat
    );

// Disables or enables the LAPIC in SW
void
LapicSetState(
    IN      PVOID                           ApicBaseAddress,
    IN      BYTE                            SpuriousInterruptVector,
    IN      BOOLEAN                         Enable
    );

BOOLEAN
LapicGetState(
    IN      PVOID                           ApicBaseAddress
    );

void
LapicConfigureTimer(
    IN      PVOID                           ApicBaseAddress,
    IN      BYTE                            TimerInterruptVector,
    IN     _Strict_type_match_
            APIC_DIVIDE_VALUE               DivideValue
    );

void
LapicSetTimerInterval(
    IN      PVOID                           ApicBaseAddress,
    IN      DWORD                           TimerCount
    );

void
LapicConfigureLvtRegisters(
    IN      PVOID                           ApicBaseAddress,
    IN      BYTE                            ErrorInterruptVector
    );

void
LapicSendEOI(
    IN      PVOID                           ApicBaseAddress,
    IN      BYTE                            Vector
    );

void
LapicSendIpi(
    IN      PVOID                           ApicBaseAddress,
    IN      _Strict_type_match_
            APIC_ID                         ApicId,
    IN      _Strict_type_match_
            APIC_DELIVERY_MODE              DeliveryMode,
    IN      _Strict_type_match_
            APIC_DESTINATION_SHORTHAND      DestinationShorthand,
    IN      _Strict_type_match_
            APIC_DESTINATION_MODE           DestinationMode,
    IN_OPT  BYTE*                           Vector
    );

BYTE
LapicGetPpr(
    IN      PVOID                           ApicBaseAddress
    );

DWORD
LapicGetTimerCount(
    IN      PVOID                           ApicBaseAddress
    );

// new functions
PHYSICAL_ADDRESS
LapicGetBasePhysicalAddress(
    void
    );

void
LapicDetermineDividedBusFrequency(
    IN      PVOID                           ApicBaseAddress,
    OUT     DWORD*                          DividedBusFrequency
    );

DWORD
LapicGetErrorRegister(
    IN      PVOID                           ApicBaseAddress
    );

BOOLEAN
LapicIsInterruptServiced(
    IN      PVOID                           ApicBaseAddress,
    IN      BYTE                            Vector
    );