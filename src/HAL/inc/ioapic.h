#pragma once

#include "apic_common.h"

#define IO_APIC_VERSION_PCI_22_COMPLIANT        0x20

BYTE
IoApicGetId(
    IN      PVOID                   IoApic
    );

BYTE
IoApicGetVersion(
    IN      PVOID                   IoApic
    );

BYTE
IoApicGetMaximumRedirectionEntry(
    IN      PVOID                   IoApic
    );

void
IoApicSetRedirectionTableEntry(
    IN      PVOID                   IoApic,
    IN      BYTE                    Index,
    IN      BYTE                    Vector,
    IN _Strict_type_match_
            APIC_DESTINATION_MODE   DestinationMode,
    IN      BYTE                    Destination,
    IN _Strict_type_match_
            APIC_DELIVERY_MODE      DeliveryMode,
    IN _Strict_type_match_
            APIC_PIN_POLARITY       PinPolarity,
    IN _Strict_type_match_
            APIC_TRIGGER_MODE       TriggerMode
    );

BYTE
IoApicGetRedirectionTableEntryVector(
    IN      PVOID                   IoApic,
    IN      BYTE                    Index
    );

void
IoApicSetRedirectionTableEntryMask(
    IN      PVOID                   IoApic,
    IN      BYTE                    Index,
    IN      BOOLEAN                 Masked
    );

__declspec(deprecated)
void
IoApicSendEOI(
    IN      PVOID                   IoApic,
    IN      BYTE                    Vector
    );