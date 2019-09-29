#include "hal_base.h"
#include "pci.h"
#include "pci_common.h"

#define PCI_CONFIG_ADDRESS                              0xCF8
#define PCI_CONFIG_DATA                                 0xCFC

#pragma pack(push,1)

#pragma warning(push)

// warning C4201: nonstandard extension used: nameless struct/union
#pragma warning(disable:4201)

// warning C4214: nonstandard extension used: bit field types other than int
#pragma warning(disable:4214)

typedef union _PCI_CONFIG_REGISTER
{
    struct
    {
        // Since all reads and writes must be both 32-bits and aligned to work on all
        // implementations, the two lowest bits of PCI_CONFIG_REGISTER must always be zero,
        // with the remaining six bits allowing you to choose each of the 64 32-bit words.
        BYTE    __Reserved0                 : 2;

        BYTE    RegisterNumber              : 6;
        BYTE    FunctionNumber              : 3;
        BYTE    DeviceNumber                : 5;
        BYTE    BusNumber;
        BYTE    __Reserved1                 : 7;
        BYTE    EnableBit                   : 1;
    };
    DWORD       Raw;
} PCI_CONFIG_REGISTER, *PPCI_CONFIG_REGISTER;
STATIC_ASSERT(sizeof(PCI_CONFIG_REGISTER) == sizeof(DWORD));

#pragma warning(pop)
#pragma pack(pop)

#define PCI_SET_CONFIG_REGISTER(X,Bus,Dev,Func,Reg)     \
            memzero(&(X), sizeof(PCI_CONFIG_REGISTER)); \
            (X).EnableBit = 1;                          \
            (X).BusNumber = (Bus);                      \
            (X).DeviceNumber = (Dev);                   \
            (X).FunctionNumber = (Func);                \
            (X).RegisterNumber = (Reg);

static
__forceinline
DWORD
_PciReadRegister(
    IN      PCI_CONFIG_REGISTER ConfigRegister
    )
{
    __outdword(PCI_CONFIG_ADDRESS, ConfigRegister.Raw);
    return __indword(PCI_CONFIG_DATA);
}

static
__forceinline
void
_PciWriteRegister(
    IN      PCI_CONFIG_REGISTER ConfigRegister,
    IN      DWORD               Data
    )
{
    __outdword(PCI_CONFIG_ADDRESS, ConfigRegister.Raw);
    __outdword(PCI_CONFIG_DATA, Data);
}

//******************************************************************************
// Function:     _PciRetrieveDevice
// Description:  Retrieves the device found at the specified PCI address.
// Returns:      BOOLEAN - TRUE => found device
//                         FALSE => no device present at the specified address.
// Parameter:    IN BYTE Bus
// Parameter:    IN BYTE Device
// Parameter:    IN BYTE Function
// Parameter:    OUT PPCI_DEVICE PciDevice
//******************************************************************************
static
BOOL_SUCCESS
BOOLEAN
_PciRetrieveDevice(
    IN      PCI_DEVICE_LOCATION DeviceLocation,
    IN      WORD                BytesToRead,
    OUT_WRITES_BYTES_ALL(BytesToRead)
            PPCI_DEVICE_DESCRIPTION         PciDevice
    );

STATUS
PciRetrieveNextDevice(
    IN      BOOLEAN         ResetSearch,
    IN      WORD            BytesToRead,
    OUT_WRITES_BYTES_ALL(BytesToRead)
            PPCI_DEVICE_DESCRIPTION     PciDevice
    )
{
    BOOLEAN         foundDevice;

    static PCI_DEVICE_LOCATION __currentDeviceLocation = { 0 };
    static BOOLEAN  __exhausedPciSpace = FALSE;

    if (BytesToRead > PREDEFINED_PCI_DEVICE_SPACE_SIZE)
    {
        return STATUS_INVALID_PARAMETER2;
    }

    if (NULL == PciDevice)
    {
        return STATUS_INVALID_PARAMETER3;
    }

    foundDevice = FALSE;

    if (ResetSearch)
    {
        memzero(&__currentDeviceLocation, sizeof(PCI_DEVICE_LOCATION));
        __exhausedPciSpace = FALSE;
    }

    if (__exhausedPciSpace)
    {
        return STATUS_DEVICE_NO_MORE_DEVICES;
    }

    do
    {
        foundDevice = _PciRetrieveDevice(__currentDeviceLocation, BytesToRead, PciDevice);

        __currentDeviceLocation.Function = (__currentDeviceLocation.Function + 1) % PCI_NO_OF_FUNCTIONS;
        if (0 == __currentDeviceLocation.Function)
        {
            __currentDeviceLocation.Device = (__currentDeviceLocation.Device + 1) % PCI_NO_OF_DEVICES;
            if (0 == __currentDeviceLocation.Device)
            {
                __currentDeviceLocation.Bus = (__currentDeviceLocation.Bus + 1) % PCI_NO_OF_BUSES;
                if (0 == __currentDeviceLocation.Bus)
                {
                    // because we found a device we can't return an error yet =>
                    // we just set the static variable
                    __exhausedPciSpace = TRUE;
                    if (!foundDevice)
                    {
                        // we didn't manage to find another PCI device so we can return
                        // the status right now
                        return STATUS_DEVICE_NO_MORE_DEVICES;
                    }
                }
            }
        }

        // we loop until we find a PCI device or until we exhausted
        // all the PCI space
    } while (!foundDevice);

    ASSERT(foundDevice);
    return STATUS_SUCCESS;
}

static
BOOL_SUCCESS
BOOLEAN
_PciRetrieveDevice(
    IN      PCI_DEVICE_LOCATION DeviceLocation,
    IN      WORD                BytesToRead,
    OUT_WRITES_BYTES_ALL(BytesToRead)
            PPCI_DEVICE_DESCRIPTION         PciDevice
    )
{
    STATUS status;
    WORD currentRegister;
    PCI_CONFIG_REGISTER addrMsg;
    DWORD dataRead;

    ASSERT( BytesToRead <= PREDEFINED_PCI_DEVICE_SPACE_SIZE );
    ASSERT( NULL != PciDevice );

    status = STATUS_SUCCESS;
    currentRegister = 0;
    PCI_SET_CONFIG_REGISTER(addrMsg, DeviceLocation.Bus, DeviceLocation.Device, DeviceLocation.Function, 0);

    ASSERT(NULL != PciDevice);

    for (currentRegister = 0;
    currentRegister < BytesToRead;
        currentRegister = currentRegister + sizeof(PCI_CONFIG_REGISTER)
        )
    {
        // we shift by 2 because the register represents an index
        // in a DWORD array :)
        addrMsg.RegisterNumber = (BYTE) ( currentRegister >> 2 );

        dataRead = _PciReadRegister(addrMsg);

        if (0 == currentRegister)
        {
            if (MAX_DWORD == dataRead)
            {
                return FALSE;
            }
        }

        memcpy( PciDevice->DeviceData->Data + currentRegister, &dataRead, sizeof(PCI_CONFIG_REGISTER));
    }

    PciDevice->PciExpressDevice = FALSE;
    memcpy(&PciDevice->DeviceLocation, (PVOID) &DeviceLocation, sizeof(PCI_DEVICE_LOCATION));

    return TRUE;
}

DWORD
PciReadConfigurationSpace(
    IN      PCI_DEVICE_LOCATION     DeviceLocation,
    IN      BYTE                    Register
    )
{
    PCI_CONFIG_REGISTER addrMsg;

    ASSERT(IsAddressAligned(Register, sizeof(PCI_CONFIG_REGISTER)));

    PCI_SET_CONFIG_REGISTER(addrMsg, DeviceLocation.Bus, DeviceLocation.Device, DeviceLocation.Function, Register >> 2);

    return _PciReadRegister(addrMsg);
}

void
PciWriteConfigurationSpace(
    IN      PCI_DEVICE_LOCATION     DeviceLocation,
    IN      BYTE                    Register,
    IN      DWORD                   Value
    )
{
    PCI_CONFIG_REGISTER addrMsg;

    ASSERT(IsAddressAligned(Register, sizeof(PCI_CONFIG_REGISTER)));

    PCI_SET_CONFIG_REGISTER(addrMsg, DeviceLocation.Bus, DeviceLocation.Device, DeviceLocation.Function, Register >> 2 );

    _PciWriteRegister(addrMsg, Value );
}