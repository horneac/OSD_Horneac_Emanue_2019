#include "hal_base.h"
#include "pcie.h"
#include "pci_common.h"

#define PCI_EXPRESS_FORM_ADDRESS(Base,Bus,Dev,Func)     (volatile DWORD*) (((PBYTE)(Base))        + \
                                                                           ((QWORD)(Bus)<<20)     + \
                                                                           ((QWORD)(Dev)<<15)     + \
                                                                           ((QWORD)(Func)<<12))

STATUS
PciExpressRetrieveNextDevice(
    IN      PPCI_ROOT_COMPLEX           PciRootComplex,
    IN      BOOLEAN                     ResetSearch,
    OUT     PPCI_DEVICE_DESCRIPTION     PciDevice
    )
{
    BOOLEAN         foundDevice;

    static PCI_DEVICE_LOCATION __currentDeviceLocation = { 0 };
    static BOOLEAN  __exhausedPciSpace = FALSE;

    if (NULL == PciRootComplex)
    {
        return STATUS_INVALID_PARAMETER1;
    }

    if (NULL == PciDevice)
    {
        return STATUS_INVALID_PARAMETER3;
    }

    foundDevice = FALSE;

    if (ResetSearch)
    {
        memzero(&__currentDeviceLocation, sizeof(PCI_DEVICE_LOCATION));
        __currentDeviceLocation.Bus = PciRootComplex->StartBusNumber;

        __exhausedPciSpace = FALSE;
    }

    if (__exhausedPciSpace)
    {
        return STATUS_DEVICE_NO_MORE_DEVICES;
    }

    do
    {
        volatile DWORD* pFunctionBase = PCI_EXPRESS_FORM_ADDRESS(PciRootComplex->BaseAddress,
                                                                 __currentDeviceLocation.Bus,
                                                                 __currentDeviceLocation.Device,
                                                                 __currentDeviceLocation.Function);

        foundDevice = MAX_DWORD != *pFunctionBase;

        if (foundDevice)
        {
            memcpy(&PciDevice->DeviceLocation, &__currentDeviceLocation, sizeof(PCI_DEVICE_LOCATION));
            PciDevice->PciExpressDevice = TRUE;
            PciDevice->DeviceData = (PPCI_DEVICE) pFunctionBase;
        }

        __currentDeviceLocation.Function = (__currentDeviceLocation.Function + 1) % PCI_NO_OF_FUNCTIONS;
        if (0 == __currentDeviceLocation.Function)
        {
            __currentDeviceLocation.Device = (__currentDeviceLocation.Device + 1) % PCI_NO_OF_DEVICES;
            if (0 == __currentDeviceLocation.Device)
            {
                __currentDeviceLocation.Bus = (__currentDeviceLocation.Bus + 1) % PCI_NO_OF_BUSES;
                if (0 == __currentDeviceLocation.Bus || 
                    (PciRootComplex->EndBusNumber + 1) == __currentDeviceLocation.Bus)
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

DWORD
PciExpressReadConfigurationSpace(
    IN      PPCI_ROOT_COMPLEX       PciRootComplex,
    IN      PCI_DEVICE_LOCATION     DeviceLocation,
    IN      WORD                    Register
    )
{
    ASSERT( NULL != PciRootComplex );
    ASSERT( NULL != PciRootComplex->BaseAddress );
    ASSERT( Register < PREDEFINED_PCI_EXPRESS_DEVICE_SPACE_SIZE );

    volatile DWORD* pFunctionBase = PCI_EXPRESS_FORM_ADDRESS(PciRootComplex->BaseAddress,
                                                             DeviceLocation.Bus,
                                                             DeviceLocation.Device,
                                                             DeviceLocation.Function);

    return *((volatile DWORD*)PtrOffset(pFunctionBase, Register));
}

void
PciExpressWriteConfigurationSpace(
    IN      PPCI_ROOT_COMPLEX       PciRootComplex,
    IN      PCI_DEVICE_LOCATION     DeviceLocation,
    IN      WORD                    Register,
    IN      DWORD                   Value
    )
{
    ASSERT(NULL != PciRootComplex);
    ASSERT(NULL != PciRootComplex->BaseAddress);
    ASSERT(Register < PREDEFINED_PCI_EXPRESS_DEVICE_SPACE_SIZE);

    volatile DWORD* pFunctionBase = PCI_EXPRESS_FORM_ADDRESS(PciRootComplex->BaseAddress,
                                                             DeviceLocation.Bus,
                                                             DeviceLocation.Device,
                                                             DeviceLocation.Function);

    *((volatile DWORD*)PtrOffset(pFunctionBase, Register)) = Value;
}