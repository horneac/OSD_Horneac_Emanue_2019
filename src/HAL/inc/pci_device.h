#pragma once

#include "pci.h"
#include "apic_common.h"

STATUS
PciDevRetrieveCapabilityById(
    IN      PPCI_DEVICE             Device,
    IN      PCI_CAPABILITY_ID       Id,
    OUT_PTR PPCI_CAPABILITY_HEADER* Capability
    );

STATUS
PciDevRetrieveNextCapability(
    IN      PPCI_DEVICE             Device,
    IN_OPT  PPCI_CAPABILITY_HEADER  PreviousCapability,
    OUT_PTR PPCI_CAPABILITY_HEADER* NextCapability
    );

STATUS
PciDevDisableLegacyInterrupts(
    IN      PPCI_DEVICE_DESCRIPTION Device
    );

STATUS
PciDevProgramMsiInterrupt(
    IN      PPCI_DEVICE_DESCRIPTION Device,
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