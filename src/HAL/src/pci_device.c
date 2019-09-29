#include "hal_base.h"
#include "pci_device.h"

static
void
_PciDevProgramIoPortMsiInterrupt(
    IN      PPCI_DEVICE_DESCRIPTION Device,
    IN      PPCI_CAPABILITY_MSI     PciCap
    );

STATUS
PciDevRetrieveCapabilityById(
    IN      PPCI_DEVICE             Device,
    IN      PCI_CAPABILITY_ID       Id,
    OUT_PTR PPCI_CAPABILITY_HEADER* Capability
    )
{
    BYTE capPtr;
    PPCI_CAPABILITY_HEADER pciCap;

    if (NULL == Device)
    {
        return STATUS_INVALID_PARAMETER1;
    }

    if (NULL == Capability)
    {
        return STATUS_INVALID_PARAMETER3;
    }

    if (!Device->Header.Status.CapabilitiesList)
    {
        return STATUS_DEVICE_CAPABILITIES_NOT_SUPPORTED;
    }

    capPtr = Device->Header.Device.CapabilitiesPointer;


    while (0 != capPtr)
    {
        pciCap = (PPCI_CAPABILITY_HEADER)(Device->Data + capPtr);

        if (pciCap->CapabilityId == Id)
        {
            *Capability = pciCap;
            return STATUS_SUCCESS;
        }

        capPtr = pciCap->NextPointer;
    }

    return STATUS_DEVICE_CAPABILITY_DOES_NOT_EXIST;
}

STATUS
PciDevRetrieveNextCapability(
    IN      PPCI_DEVICE             Device,
    IN_OPT  PPCI_CAPABILITY_HEADER  PreviousCapability,
    OUT_PTR PPCI_CAPABILITY_HEADER* NextCapability
    )
{
    BYTE capPtr;
    PPCI_CAPABILITY_HEADER pciCap;

    if (NULL == Device)
    {
        return STATUS_INVALID_PARAMETER1;
    }

    if (NULL == NextCapability)
    {
        return STATUS_INVALID_PARAMETER3;
    }

    if (!Device->Header.Status.CapabilitiesList)
    {
        return STATUS_DEVICE_CAPABILITIES_NOT_SUPPORTED;
    }

    capPtr = NULL != PreviousCapability ? PreviousCapability->NextPointer : Device->Header.Device.CapabilitiesPointer;

    if (0 != capPtr)
    {
        pciCap = (PPCI_CAPABILITY_HEADER)(Device->Data + capPtr);

        *NextCapability = pciCap;

        return STATUS_SUCCESS;
    }

    return STATUS_DEVICE_CAPABILITY_DOES_NOT_EXIST;
}

STATUS
PciDevDisableLegacyInterrupts(
    IN      PPCI_DEVICE_DESCRIPTION Device
    )
{
    if (NULL == Device)
    {
        return STATUS_INVALID_PARAMETER1;
    }

    Device->DeviceData->Header.Command.InterruptDisable = TRUE;

    if (!Device->PciExpressDevice)
    {
        // disable legacy interrupts
        PciWriteConfigurationSpace(Device->DeviceLocation,
                                   FIELD_OFFSET(PCI_COMMON_HEADER, Command),
                                   *(DWORD*)&Device->DeviceData->Header.Command
                                   );
    }

    return STATUS_SUCCESS;
}

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
    )
{
    STATUS status;
    PPCI_CAPABILITY_MSI pciCap;
    PCI_MSI_DATA_REGISTER msgData;
    PCI_MSI_ADDRESS_REGISTER msgAddrLower;

    if (NULL == Device)
    {
        return STATUS_INVALID_PARAMETER1;
    }

    status = STATUS_SUCCESS;
    pciCap = NULL;
    msgData.Raw = 0;
    msgAddrLower.Raw = 0;

    status = PciDevRetrieveCapabilityById(Device->DeviceData,
                                          PCI_CAPABILITY_ID_MSI,
                                          (PPCI_CAPABILITY_HEADER*)&pciCap
    );
    if (!SUCCEEDED(status))
    {
        return STATUS_DEVICE_INTERRUPT_TYPE_NOT_SUPPORTED;
    }
    ASSERT(NULL != pciCap);

    msgAddrLower.DestinationId = Destination;
    msgAddrLower.DestinationMode = DestinationMode;
    msgAddrLower.RedirectionHint = TRUE;
    msgAddrLower.UpperFixedAddress = 0xFEE;

    msgData.Vector = Vector;
    msgData.DeliveryMode = DeliveryMode;
    msgData.Assert = PinPolarity;
    msgData.TriggerMode = TriggerMode;

    pciCap->MessageAddressLower.Raw = msgAddrLower.Raw;

    if (pciCap->MessageControl.Is64BitCapable)
    {
        pciCap->Capability64Bit.MessageAddressHigher = 0;

        pciCap->Capability64Bit.MessageData.Raw = msgData.Raw;
    }
    else
    {
        // 32 bit capable
        pciCap->Capability32Bit.MessageData.Raw = msgData.Raw;
    }

    pciCap->MessageControl.MsiEnable = TRUE;

    if (!Device->PciExpressDevice)
    {
        _PciDevProgramIoPortMsiInterrupt(Device, pciCap);
    }

    return STATUS_SUCCESS;
}

static
void
_PciDevProgramIoPortMsiInterrupt(
    IN      PPCI_DEVICE_DESCRIPTION Device,
    IN      PPCI_CAPABILITY_MSI     PciCap
    )
{  

    ASSERT(NULL != Device);
    ASSERT(NULL != PciCap);

    // write Message Address Lower
    PciWriteConfigurationSpace(Device->DeviceLocation,
                               (BYTE)(PtrDiff(PciCap,Device->DeviceData) + FIELD_OFFSET(PCI_CAPABILITY_MSI,MessageAddressLower)),
                               *((PDWORD)PciCap + 1)
                               );

    if (PciCap->MessageControl.Is64BitCapable)
    {
        // write message address upper
        PciWriteConfigurationSpace(Device->DeviceLocation,
                                   (BYTE)(PtrDiff(PciCap,Device->DeviceData) + FIELD_OFFSET(PCI_CAPABILITY_MSI,Capability64Bit.MessageAddressHigher)),
                                   *((PDWORD)PciCap + 2)
                                   );

        // write message data
        PciWriteConfigurationSpace(Device->DeviceLocation,
                                   (BYTE)(PtrDiff(PciCap, Device->DeviceData) + FIELD_OFFSET(PCI_CAPABILITY_MSI, Capability64Bit.MessageData)),
                                   *((PDWORD)PciCap + 3)
                                   );
    }
    else
    {
        // write message data
        PciWriteConfigurationSpace(Device->DeviceLocation,
                                   (BYTE)(PtrDiff(PciCap, Device->DeviceData) + FIELD_OFFSET(PCI_CAPABILITY_MSI, Capability32Bit.MessageData)),
                                   *((PDWORD)PciCap + 2)
                                   );
    }

    // write Message Control, this will enable MSI interrupts
    PciWriteConfigurationSpace(Device->DeviceLocation,
                               (BYTE)(PtrDiff(PciCap, Device->DeviceData)),
                               *(PDWORD)PciCap
                               );
}