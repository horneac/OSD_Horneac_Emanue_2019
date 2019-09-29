#pragma once

#define PREDEFINED_PCI_DEVICE_SPACE_SIZE            256
#define PREDEFINED_PCI_EXPRESS_DEVICE_SPACE_SIZE    4096

#define PREDEFINED_PCI_BRIDGE_HEADER_SIZE           48
#define PREDEFINED_PCI_DEVICE_HEADER_SIZE           48
#define PREDEFINED_PCI_COMMON_HEADER_SIZE           64
#define PREDEFINED_PCI_BAR_SIZE                     4
#define PREDEFINED_PCI_STATUS_SIZE                  2
#define PREDEFINED_PCI_COMMAND_SIZE                 2
#define PREDEFINED_PCI_HEADER_TYPE_SIZE             1
#define PREDEFINED_PCI_CAPABILITY_HEADER_SIZE       2

#define PREDEFINED_PCI_MSI_ADDRESS_REGISTER_SIZE    4
#define PREDEFINED_PCI_MSI_DATA_REGISTER_SIZE       2

#define PCI_DEVICE_NO_OF_BARS                       6U
#define PCI_BRIDGE_NO_OF_BARS                       2U

typedef enum _PCI_VENDOR_ID
{
    PciVendorIdIntel            = 0x8086,
} PCI_VENDOR_ID;

// PCI device classes
typedef enum _PCI_DEVICE_CLASS
{
    PciDeviceClassUnclassified,
    PciDeviceClassMassStorageController,
    PciDeviceClassNetworkController,
    PciDeviceClassDisplayController,
    PciDeviceClassMultimediaController,
    PciDeviceClassMemoryController,
    PciDeviceClassBridge,
    PciDeviceClassCommunicationController,
    PciDeviceClassGenericSystemPeripheral,
    PciDeviceClassInputDeviceController,
    PciDeviceClassDockingStation,
    PciDeviceClassProcessor,
    PciDeviceClassSerialBusController,
    PciDeviceClassWirelessController,
    PciDeviceClassInteligentController,
    PciDeviceClassSatelliteCommunicationController,
    PciDeviceClassEncryptionController,
    PciDeviceClassSignalProcessingController,
    PciDeviceClassProcessingAccelerators,
    PciDeviceClassNonEssentialInstrumentation,
    PciDeviceClassCoprocessor = 0x40
} PCI_DEVICE_CLASS;

// Subclasses corresponding for
// PCI_DEVICE_CLASS == PciDeviceClassMassStorageController
typedef enum _PCI_MASS_STORAGE
{
    PciMassStorageSCSI,
    PciMassStorageIDE,
    PciMassStorageFloppy,
    PciMassStorageIPI,
    PciMassStorageRAID,
    PciMassStorageATA,
    PciMassStorageSATA,
    PciMassStorageSAS,
    PciMassStorageNonVolatileMemoryController,
} PCI_MASS_STORAGE;

// Subclasses corresponding for
// PCI_DEVICE_CLASS == PciDeviceClassBridge
typedef enum _PCI_BRIDGE_SUBCLASS
{
    PciBridgeSubclassHost,
    PciBridgeSubclassISA,
    PciBridgeSubclassEISA,
    PciBridgeSubclassMCA,
    PciBridgeSubclassPCItoPCI,
    PciBridgeSubclassPCMCIA,
    PciBridgeSubclassNuBus,
    PciBridgeSubclassCardBus,
    PciBridgeSubclassRACEway,
} PCI_BRIDGE_SUBCLASS;

#pragma pack(push,1)

#pragma warning(push)

// warning C4214: nonstandard extension used: bit field types other than int
#pragma warning(disable:4214)

// warning C4201: nonstandard extension used: nameless struct/union
#pragma warning(disable:4201)

#define PCI_MEM_SPACE_32_BIT            0b00
#define PCI_MEM_SPACE_64_BIT            0b10

#define PCI_GET_PA_FROM_MEM_ADDR(Bar)   ((PHYSICAL_ADDRESS)((QWORD)(Bar)->Raw & 0xFFFF'FFF0))
#define PCI_GET_PORT_FROM_IO_ADDR(Bar)  ((Bar)->Raw & 0xFFFF'FFFC)

typedef volatile union _PCI_BAR
{
    struct {
        // if Zero => We have a memory mapped address
        DWORD           Zero            :    1;

        // 00 Base register is 32 bits wide and mapping can be
        //    done anywhere in the 32 - bit Memory Space.
        // 01 Reserved46
        // 10 Base register is 64 bits wide and can be mapped
        //    anywhere in the 64 - bit address space.
        // 11 Reserved
        DWORD           Type            :    2;
        DWORD           Prefetchable    :    1;
        DWORD           Address         :   28;
    } MemorySpace;
    struct {
        // if One => we have an I/O mapped address
        DWORD           One             :    1;
        DWORD           Reserved        :    1;
        DWORD           Address         :   30;
    } IoSpace;
    DWORD               Raw;
} PCI_BAR, *PPCI_BAR;
STATIC_ASSERT(sizeof(PCI_BAR) == PREDEFINED_PCI_BAR_SIZE);

typedef union _PCI_STATUS_REGISTER
{
    struct
    {
        WORD            __Reserved0         :   3;
        WORD            InterruptStatus     :   1;
        WORD            CapabilitiesList    :   1;
        WORD            __Reserved1         :  11;
    };
    WORD                Value;
} PCI_STATUS_REGISTER, *PPCI_STATUS_REGISTER;
STATIC_ASSERT(sizeof(PCI_STATUS_REGISTER) == PREDEFINED_PCI_STATUS_SIZE);

typedef union _PCI_COMMAND_REGISTER
{
    struct
    {
        // Controls a device's response to I/O Space accesses. A value of 0
        // disables the device response.A value of 1 allows the device to
        // respond to I / O Space accesses.State after RST# is 0.
        WORD            IoSpaceEnabled      :   1;

        // Controls a device's response to Memory Space accesses. A value of
        // 0 disables the device response.A value of 1 allows the device to
        // respond to Memory Space accesses.State after RST# is 0.
        WORD            MemorySpaceEnabled  :   1;

        // Controls a device's ability to act as a master on the PCI bus. A value
        // of 0 disables the device from generating PCI accesses.A value of 1
        // allows the device to behave as a bus master.State after RST# is 0.
        WORD            BusMaster           :   1;

        // Controls a device's action on Special Cycle operations. A value of 0
        // causes the device to ignore all Special Cycle operations.A value of 1
        // allows the device to monitor Special Cycle operations.State after
        // RST# is 0.
        WORD            SpecialCycle        :   1;

        // This is an enable bit for using the Memory Write and Invalidate
        // command.When this bit is 1, masters may generate the command.
        // When it is 0, Memory Write must be used instead.State after RST#
        // is 0. This bit must be implemented by master devices that can
        // generate the Memory Write and Invalidate command.
        WORD            MemWriteInvalidate  :   1;

        // This bit controls how VGA compatible and graphics devices handle
        // accesses to VGA palette registers.When this bit is 1, palette
        // snooping is enabled(i.e., the device does not respond to palette
        // register writes and snoops the data).When the bit is 0, the device
        // should treat palette write accesses like all other accesses.VGA
        // compatible devices should implement this bit.Refer to Section 3.10
        // for more details on VGA palette snooping.
        WORD            PalleteSnooping     :   1;

        // This bit controls the device's response to parity errors. When the bit
        // is set, the device must take its normal action when a parity error is
        // detected.When the bit is 0, the device sets its Detected Parity Error
        // status bit(bit 15 in the Status register) when an error is detected, but
        // does not assert PERR# and continues normal operation.This bit's
        // state after RST# is 0. Devices that check parity must implement this
        // bit.Devices are still required to generate parity even if parity checking
        // is disabled.
        WORD            BenignParityError   :   1;

        WORD            __Zero              :   1;

        // This bit is an enable bit for the SERR# driver.A value of 0 disables
        // the SERR# driver.A value of 1 enables the SERR# driver.This bit's
        // state after RST# is 0. All devices that have an SERR# pin must
        // implement this bit.Address parity errors are reported only if this bit
        // and bit 6 are 1.
        WORD            SerrDriverEnable    :   1;

        // This optional read / write bit controls whether or not a master can do
        // fast back - to - back transactions to different devices.Initialization
        // software will set the bit if all targets are fast back - to - back capable.A
        // value of 1 means the master is allowed to generate fast back - to - back
        // transactions to different agents as described in Section 3.4.2.A value
        // of 0 means fast back - to - back transactions are only allowed to the
        // same agent.This bit's state after RST# is 0.
        WORD            FastBackToBack      :   1;

        // This bit disables the device / function from asserting INTx#.A value of
        // 0 enables the assertion of its INTx# signal.A value of 1 disables the
        // assertion of its INTx# signal.This bit’s state after RST# is 0. Refer to
        // Section 6.8.1.3 for control of MSI.
        WORD            InterruptDisable    :   1;

        WORD            __Reserved          :   5;
    };
    WORD                Value;
} PCI_COMMAND_REGISTER, *PPCI_COMMAND_REGISTER;
STATIC_ASSERT(sizeof(PCI_COMMAND_REGISTER) == PREDEFINED_PCI_COMMAND_SIZE);

#define PCI_HEADER_LAYOUT_CLASSIC               0
#define PCI_HEADER_LAYOUT_PCI_TO_PCI            1
#define PCI_HEADER_LAYOUT_CARDBUS               2

typedef union _PCI_HEADER_TYPE_REGISTER
{
    struct
    {
        // Bits 6 through 0 identify the layout of the
        // second part of the predefined header.The encoding 00h specifies
        // the layout shown in Figure 6 - 1. The encoding 01h is defined for
        // PCI - to - PCI bridges and is defined in the document PCI to PCI Bridge
        // Architecture Specification.The encoding 02h is defined for a CardBus
        // bridge and is documented in the PC Card Standard.All other
        // encodings are reserved.
        BYTE            Layout              :   7;

        // Bit 7 in this register is used to identify a multi - function device.
        // If the bit is 0, then the device is single function. If the bit is 1,
        // then the device has multiple functions.
        BYTE            Multifunction       :   1;
    };
    BYTE                Value;
} PCI_HEADER_TYPE_REGISTER, *PPCI_HEADER_TYPE_REGISTER;
STATIC_ASSERT(sizeof(PCI_HEADER_TYPE_REGISTER) == PREDEFINED_PCI_HEADER_TYPE_SIZE);

#define PCI_CAPABILITY_ID_RESERVED              0x0
#define PCI_CAPABILITY_ID_POWER_MGMT            0x1
#define PCI_CAPABILITY_ID_MSI                   0x5
#define PCI_CAPABILITY_ID_VENDOR                0x9
#define PCI_CAPABILITY_ID_HOT_PLUG              0xC
#define PCI_CAPABILITY_ID_BRIDGE_SUB_VENDOR     0xD
#define PCI_CAPABILITY_ID_SECURE_DEVICE         0xF
#define PCI_CAPABILITY_ID_PCI_EXPRESS           0x10
#define PCI_CAPABILITY_ID_MSIX                  0x11

typedef BYTE PCI_CAPABILITY_ID;

typedef struct _PCI_CAPABILITY_HEADER
{
    PCI_CAPABILITY_ID           CapabilityId;

    BYTE                        NextPointer;
} PCI_CAPABILITY_HEADER, *PPCI_CAPABILITY_HEADER;
STATIC_ASSERT(sizeof(PCI_CAPABILITY_HEADER) == PREDEFINED_PCI_CAPABILITY_HEADER_SIZE);

// Intel specific MSI definitions
typedef volatile union _PCI_MSI_ADDRESS_REGISTER
{
    struct
    {
        DWORD       __Reserved0             :   2;
        DWORD       DestinationMode         :   1;
        DWORD       RedirectionHint         :   1;
        DWORD       __Reserved1             :   8;

        // This field contains an 8 - bit destination ID.It identifies the message’s target processor(s).
        // The destination ID corresponds to bits 63:56 of the I / O APIC Redirection Table Entry if the IOAPIC is used to
        // dispatch the interrupt to the processor(s).
        DWORD       DestinationId           :   8;

        // These bits contain a fixed value for interrupt messages(0FEEH).This value locates interrupts at
        // the 1 - MByte area with a base address of 4G – 18M.All accesses to this region are directed as interrupt
        // messages.Care must to be taken to ensure that no other device claims the region as I / O space.
        DWORD       UpperFixedAddress       :  12;
    };
    DWORD           Raw;
} PCI_MSI_ADDRESS_REGISTER, *PPCI_MSI_ADDRESS_REGISTER;
STATIC_ASSERT(sizeof(PCI_MSI_ADDRESS_REGISTER) == PREDEFINED_PCI_MSI_ADDRESS_REGISTER_SIZE);

typedef volatile union _PCI_MSI_DATA_REGISTER
{
    struct
    {
        BYTE        Vector;

        BYTE        DeliveryMode            :   3;
        BYTE        __Reserved0             :   3;
        BYTE        Assert                  :   1;
        BYTE        TriggerMode             :   1;
    };
    WORD            Raw;
} PCI_MSI_DATA_REGISTER, *PPCI_MSI_DATA_REGISTER;
STATIC_ASSERT(sizeof(PCI_MSI_DATA_REGISTER) == PREDEFINED_PCI_MSI_DATA_REGISTER_SIZE);

typedef volatile struct _PCI_CAPABILITY_MSI
{
    PCI_CAPABILITY_HEADER               Header;
    union
    {
        struct
        {
            // RW
            BYTE                        MsiEnable                   :   1;

            // RO
            BYTE                        MultipleMessageCapable      :   3;

            // RW
            BYTE                        MultipleMessageEnable       :   3;

            // RO
            BYTE                        Is64BitCapable              :   1;

            // RO
            BYTE                        PerVectorMasking            :   1;

            BYTE                        __Reserved0                 :   7;
        };
        WORD                            Raw;
    } MessageControl;
    PCI_MSI_ADDRESS_REGISTER            MessageAddressLower;
    union
    {
        struct
        {
            PCI_MSI_DATA_REGISTER       MessageData;
        } Capability32Bit;
        struct
        {
            DWORD                       MessageAddressHigher;
            PCI_MSI_DATA_REGISTER       MessageData;
        } Capability64Bit;
    };
} PCI_CAPABILITY_MSI, *PPCI_CAPABILITY_MSI;

typedef volatile struct _PCI_CAPABILITY_MSIX
{
    PCI_CAPABILITY_HEADER       Header;
    WORD                        MessageControl;
} PCI_CAPABILITY_MSIX, *PPCI_CAPABILITY_MSIX;

typedef volatile struct _PCI_DEVICE_HEADER
{
    // 0x10
    PCI_BAR                         Bar[PCI_DEVICE_NO_OF_BARS];

    // 0x28
    DWORD                           CarbusCISPointer;

    // 0x2C
    WORD                            SubSystemVendorID;
    WORD                            SubSystemID;

    // 0x30
    DWORD                           ExpansionROMBaseAddress;

    // 0x34
    BYTE                            CapabilitiesPointer;
    BYTE                            Reserved[7];

    // 0x3C
    BYTE                            InterruptLine;
    BYTE                            InterruptPin;
    BYTE                            MinGrant;
    BYTE                            MaxLatency;
} PCI_DEVICE_HEADER, *PPCI_DEVICE_HEADER;
STATIC_ASSERT(sizeof(PCI_DEVICE_HEADER) == PREDEFINED_PCI_DEVICE_HEADER_SIZE);

typedef volatile struct _PCI_BRIDGE_HEADER
{
    PCI_BAR                         Bar[PCI_BRIDGE_NO_OF_BARS];

    // 0x18

    // The Primary Bus Number register is used to record the bus number of the PCI bus segment to
    // which the primary interface of the bridge is connected.
    BYTE                            PrimaryBusNumber;

    // The Secondary Bus Number register is used to record the bus number of the PCI bus segment to
    // which the secondary interface of the bridge is connected.
    BYTE                            SecondaryBusNumber;

    // The Subordinate Bus Number register is used to record the bus number of the highest numbered
    // PCI bus segment which is behind(or subordinate to) the bridge.
    BYTE                            SubordinateBusNumber;
    BYTE                            SecondaryLatencyTimer;

    // 0x1c
    BYTE                            IoBase;
    BYTE                            IoLimit;
    WORD                            SecondaryStatus;

    // 0x20
    WORD                            MemoryBase;
    WORD                            MemoryLimit;

    // 0x24
    WORD                            PrefetchableMemoryBase;
    WORD                            PrefetchableMemoryLimit;

    // 0x28
    DWORD                           PrefetchableBaseUpper;
    DWORD                           PrefetchableLimitUpper;

    // 0x30
    WORD                            IoBaseUpper;
    WORD                            IoLimitUpper;

    // 0x34
    BYTE                            CapabilitiesPointer;
    BYTE                            __Reserved[3];

    // 0x38
    DWORD                           ExpansionROMBaseAddress;

    // 0x3C
    BYTE                            InterruptLine;
    BYTE                            InterruptPin;
    WORD                            BridgeControl;
} PCI_BRIDGE_HEADER, *PPCI_BRIDGE_HEADER;
STATIC_ASSERT(sizeof(PCI_BRIDGE_HEADER) == PREDEFINED_PCI_BRIDGE_HEADER_SIZE);

typedef volatile struct _PCI_COMMON_HEADER
{
    // PCI Specification 3.0 Section 6.1
    // The first 16 bytes are defined the same for all types of devices.
    WORD                            VendorID;
    WORD                            DeviceID;

    // 0x04
    PCI_COMMAND_REGISTER            Command;
    PCI_STATUS_REGISTER             Status;

    // 0x08
    BYTE                            RevisionID;
    BYTE                            ProgIF;
    BYTE                            Subclass;
    BYTE                            ClassCode;

    // 0x0C
    BYTE                            CacheLineSize;
    BYTE                            LatencyTimer;
    PCI_HEADER_TYPE_REGISTER        HeaderType;
    BYTE                            Bist;

    union
    {
        PCI_DEVICE_HEADER           Device;
        PCI_BRIDGE_HEADER           Bridge;
    };
} PCI_COMMON_HEADER, *PPCI_COMMON_HEADER;
STATIC_ASSERT(sizeof(PCI_COMMON_HEADER) == PREDEFINED_PCI_COMMON_HEADER_SIZE);

#pragma warning(pop)
#pragma pack(pop)
