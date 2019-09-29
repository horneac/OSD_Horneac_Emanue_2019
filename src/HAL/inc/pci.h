#pragma once

#include "pci_registers.h"

typedef struct _PCI_DEVICE_LOCATION
{
    BYTE                    Bus;
    BYTE                    Device;
    BYTE                    Function;
} PCI_DEVICE_LOCATION, *PPCI_DEVICE_LOCATION;

typedef union _PCI_DEVICE
{
    PCI_COMMON_HEADER       Header;
    BYTE                    Data[PREDEFINED_PCI_DEVICE_SPACE_SIZE];
} PCI_DEVICE, *PPCI_DEVICE;
STATIC_ASSERT(sizeof(PCI_DEVICE) == PREDEFINED_PCI_DEVICE_SPACE_SIZE);

typedef struct _PCI_DEVICE_DESCRIPTION
{
    PCI_DEVICE_LOCATION                 DeviceLocation;
    BOOLEAN                             PciExpressDevice;
    PPCI_DEVICE                         DeviceData;

    // non-NULL for devices found behind bridges
    struct _PCI_DEVICE_DESCRIPTION*     Parent;
} PCI_DEVICE_DESCRIPTION, *PPCI_DEVICE_DESCRIPTION;

//******************************************************************************
// Function:     PciRetrieveNextDevice
// Description:  Parses the PCI configuration space and retrieves the next
//               device found on the bus. 
// Returns:      STATUS - STATUS_DEVICE_NO_MORE_DEVICES if no more devices are
//               present on the PCI bus.
// Parameter:    IN BOOLEAN ResetSearch - If TRUE starts searching from bus = 0,
//               device = 0 and function = 0, else continues from where the
//               last call left of.
// Parameter:    OUT PPCI_DEVICE PciDevice
// NOTE:         If the whole configuration space has been traversed the
//               function will return STATUS_DEVICE_NO_MORE_DEVICES until a call
//               with ResetSearch == TRUE will be made.
//******************************************************************************
STATUS
PciRetrieveNextDevice(
    IN      BOOLEAN         ResetSearch,
    IN      WORD            BytesToRead,
    OUT_WRITES_BYTES_ALL(BytesToRead)
            PPCI_DEVICE_DESCRIPTION     PciDevice
    );

DWORD
PciReadConfigurationSpace(
    IN      PCI_DEVICE_LOCATION     DeviceLocation,
    IN      BYTE                    Register
    );

void
PciWriteConfigurationSpace(
    IN      PCI_DEVICE_LOCATION     DeviceLocation,
    IN      BYTE                    Register,
    IN      DWORD                   Value
    );