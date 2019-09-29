#pragma once

#include "pci.h"

typedef struct _PCI_ROOT_COMPLEX
{
    volatile DWORD* BaseAddress;
    BYTE            StartBusNumber;
    BYTE            EndBusNumber;
} PCI_ROOT_COMPLEX, *PPCI_ROOT_COMPLEX;

STATUS
PciExpressRetrieveNextDevice(
    IN      PPCI_ROOT_COMPLEX           PciRootComplex,
    IN      BOOLEAN                     ResetSearch,
    OUT     PPCI_DEVICE_DESCRIPTION     PciDevice
    );

DWORD
PciExpressReadConfigurationSpace(
    IN      PPCI_ROOT_COMPLEX       PciRootComplex,
    IN      PCI_DEVICE_LOCATION     DeviceLocation,
    IN      WORD                    Register
    );

void
PciExpressWriteConfigurationSpace(
    IN      PPCI_ROOT_COMPLEX       PciRootComplex,
    IN      PCI_DEVICE_LOCATION     DeviceLocation,
    IN      WORD                    Register,
    IN      DWORD                   Value
    );