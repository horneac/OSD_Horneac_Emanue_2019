#pragma once

#include "pci_device.h"
#include "list.h"

#pragma warning(push)

// warning C4214: nonstandard extension used: bit field types other than int
#pragma warning(disable:4214)

// warning C4201: nonstandard extension used: nameless struct/union
#pragma warning(disable:4201)

typedef struct _PCI_DEVICE_LIST_ENTRY
{
    LIST_ENTRY              ListEntry;
    PCI_DEVICE_DESCRIPTION  PciDevice;

    // valid only for bridges
    LIST_ENTRY              BridgeEntry;
} PCI_DEVICE_LIST_ENTRY, *PPCI_DEVICE_LIST_ENTRY;


typedef struct _PCI_DESCRIPTION
{
    WORD    VendorId;
    WORD    DeviceId;
    BYTE    Subclass;
    BYTE    ClassCode;
} PCI_DESCRIPTION, *PPCI_DESCRIPTION;

typedef struct _PCI_SPEC
{
    PCI_DESCRIPTION     Description;

    BOOLEAN             MatchClass      : 1;
    BOOLEAN             MatchSubclass   : 1;
    BOOLEAN             MatchVendor     : 1;
    BOOLEAN             MatchDevice     : 1;
} PCI_SPEC, *PPCI_SPEC;

typedef struct _PCI_SPEC_LOCATION
{
    PCI_DEVICE_LOCATION Location;

    BOOLEAN             MatchBus        : 1;
    BOOLEAN             MatchDevice     : 1;
    BOOLEAN             MatchFunction   : 1;
} PCI_SPEC_LOCATION, *PPCI_SPEC_LOCATION;
#pragma warning(pop)

void
PciSystemPreinit(
    void
    );

STATUS
PciSystemInit(
    void
    );

STATUS
PciSystemRetrieveDevices(
    INOUT   PLIST_ENTRY     PciDeviceList
    );

void
PciSystemEstablishHierarchy(
    IN      PLIST_ENTRY     PciDeviceList,
    INOUT   PLIST_ENTRY     PciBridgeList
    );

STATUS
PciSystemFindDevicesMatchingSpecification(
    IN      PLIST_ENTRY     PciDeviceList,
    IN      PCI_SPEC        Specification,
    OUT_WRITES_OPT(*NumberOfDevices)
            PPCI_DEVICE_DESCRIPTION*    PciDevices,
    _When_(NULL == PciDevices, OUT)
    _When_(NULL != PciDevices, INOUT)
            DWORD*          NumberOfDevices
    );

STATUS
PciSystemFindDevicesMatchingLocation(
    IN      PLIST_ENTRY                 PciDeviceList,
    IN      PCI_SPEC_LOCATION           Specification,
    OUT_WRITES_OPT(*NumberOfDevices)
            PPCI_DEVICE_DESCRIPTION*    PciDevices,
    _When_(NULL == PciDevices, OUT)
    _When_(NULL != PciDevices, INOUT)
            DWORD*                      NumberOfDevices
    );

STATUS
PciSystemReadConfigurationSpaceGeneric(
    IN      PCI_DEVICE_LOCATION     DeviceLocation,
    IN      WORD                    Register,
    IN      BYTE                    Width,
    OUT     QWORD*                  Value
    );

STATUS
PciSystemWriteConfigurationSpaceGeneric(
    IN      PCI_DEVICE_LOCATION     DeviceLocation,
    IN      WORD                    Register,
    IN      BYTE                    Width,
    IN      QWORD                   Value
    );
