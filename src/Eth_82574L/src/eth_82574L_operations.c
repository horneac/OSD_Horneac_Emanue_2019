#include "eth_82574L_base.h"
#include "eth_82574L_operations.h"
#include "eth_eeprom.h"
#include "network_port.h"

__forceinline
static
void
_EthChangeRxStatus(
    IN      PETH_DEVICE         Device,
    IN      BOOLEAN             NewStatus
    )
{
    RECEIVE_CONTROL_REGISTER ctrlRegister;

    ASSERT( NULL != Device );

    ctrlRegister.Raw = EthGetRxControlRegister(Device);
    ctrlRegister.Enable = NewStatus;
    EthSetRxControlRegister(Device, ctrlRegister);
}

__forceinline
static
void
_EthChangeTxStatus(
    IN      PETH_DEVICE         Device,
    IN      BOOLEAN             NewStatus
    )
{
    TRANSMIT_CONTROL_REGISTER ctrlRegister;

    ASSERT(NULL != Device);

    ctrlRegister.Raw = EthGetTxControlRegister(Device);
    ctrlRegister.Enable = NewStatus;
    EthSetTxControlRegister(Device, ctrlRegister);
}

static
PTR_SUCCESS
PVOID
_EthMapMemoryMappedBar(
    IN      PPCI_BAR            Bar,
    IN      DWORD               BytesToMap
    );

static
void
_EthRetrieveMacAddress(
    IN      PETH_INTERNAL_REGS  EthRegisters,
    OUT     PMAC_ADDRESS        MacAddress
    );

static
STATUS
_EthRxInit(
    IN      PETH_DEVICE         Device
    );

static
STATUS
_EthTxInit(
    IN      PETH_DEVICE         Device
    );

static
STATUS
_EthInterruptInit(
    IN      PETH_DEVICE         Device
    );

static
void
_EthDeviceControlsInit(
    IN      PETH_DEVICE         Device
    );

static
void
_EthSignalTxQueueFullIfNecessary(
    IN      PETH_DEVICE         Device
    );

STATUS
EthInitializeDevice(
    IN_READS(ETH_NO_OF_BARS_USED)   PPCI_BAR        Bars,
    INOUT                           PETH_DEVICE     Device
    )
{
    STATUS status;
    PETH_INTERNAL_REGS pInternalRegs;
    PVOID pFlash;
    PVOID pMsiXTables;

    ASSERT( NULL != Bars );
    ASSERT( NULL != Device );

    LOG_FUNC_START;

    status = STATUS_SUCCESS;
    pInternalRegs = NULL;
    pFlash = NULL;
    pMsiXTables = NULL;

    __try
    {
        pInternalRegs = _EthMapMemoryMappedBar(&Bars[0], ETH_INTERNAL_REGISTER_SIZE);
        if (NULL == pInternalRegs)
        {
            LOG_ERROR("_EthMapMemoryMappedBar could not map internal registers\n");
            status = STATUS_MEMORY_CANNOT_BE_MAPPED;
            __leave;
        }
        LOG_TRACE_NETWORK("Internal registers successfully mapped!\n");

        Device->InternalRegisters = pInternalRegs;

        pFlash = _EthMapMemoryMappedBar(&Bars[1], ETH_FLASH_SIZE);
        if (NULL == pFlash)
        {
            LOG_WARNING("Flash memory could not be mapped\n");
        }
        else
        {
            Device->Flash = pFlash;
            LOG_TRACE_NETWORK("Flash successfully mapped!\n");
        }

        pMsiXTables = _EthMapMemoryMappedBar(&Bars[3], ETH_MSI_X_TABLES_SIZE);
        if (NULL == pMsiXTables)
        {
            LOG_WARNING("Msi-X tables could not be mapped\n");
        }
        else
        {
            Device->MsiX = pMsiXTables;
            LOG_TRACE_NETWORK("Msi-X tables successfully mapped!\n");
        }

        _EthRetrieveMacAddress(pInternalRegs, &Device->MiniportDevice->PhysicalAddress);

        LOG("Ip address valid: 0x%x\n", pInternalRegs->IpAddressValid);
        LOG("Ip address 0: 0x%x\n", pInternalRegs->IpAddress0);

        status = _EthRxInit(Device);
        if (!SUCCEEDED(status))
        {
            LOG_FUNC_ERROR("_EthRxInit", status);
            __leave;
        }
        LOG_TRACE_NETWORK("_EthRxInit succeeded\n");
        Device->MiniportDevice->DeviceStatus.RxEnabled = TRUE;

        status = _EthTxInit(Device);
        if (!SUCCEEDED(status))
        {
            LOG_FUNC_ERROR("_EthTxInit", status);
            __leave;
        }
        LOG_TRACE_NETWORK("_EthTxInit succeeded\n");
        Device->MiniportDevice->DeviceStatus.TxEnabled = TRUE;

        status = _EthInterruptInit(Device);
        if (!SUCCEEDED(status))
        {
            LOG_FUNC_ERROR("_EthInterruptInit", status);
            __leave;
        }
        LOG_TRACE_NETWORK("_EthInterruptInit succeeded\n");

        _EthDeviceControlsInit(Device);
        LOG_TRACE_NETWORK("_EthDeviceControlsInit succeeded\n");

        Device->MiniportDevice->LinkUp = (BOOLEAN)(EthGetDeviceStatusRegister(Device).LinkUp);
    }
    __finally
    {
        if (!SUCCEEDED(status))
        {
            if (NULL != pMsiXTables)
            {
                IoUnmapMemory(pMsiXTables, ETH_MSI_X_TABLES_SIZE);
                pMsiXTables = NULL;
            }

            if (NULL != pFlash)
            {
                IoUnmapMemory(pFlash, ETH_FLASH_SIZE);
                pFlash = NULL;
            }

            if (NULL != pInternalRegs)
            {
                IoUnmapMemory(pInternalRegs, ETH_INTERNAL_REGISTER_SIZE);
                pInternalRegs = NULL;
            }
        }

        LOG_FUNC_END;
    }

    return status;
}

_No_competing_thread_
STATUS
EthReceiveFrame(
    IN                              PETH_DEVICE     Device,
    IN_OPT                          WORD            MaximumNumberOfFrames
    )
{
    STATUS status;
    WORD curRxIndex;
    WORD prevRxIndex;
    WORD noOfFramesReceived;

    ASSERT( NULL != Device );

    status = STATUS_SUCCESS;
    curRxIndex = Device->RxData.Buffers.CurrentDescriptor;
    ASSERT( curRxIndex < Device->RxData.Buffers.NumberOfDescriptors );

    noOfFramesReceived = 0;

    while (Device->RxData.ReceiveBuffer[curRxIndex].Status.DescriptorDone)
    {
        WORD len = Device->RxData.ReceiveBuffer[curRxIndex].Length;

        ASSERT( len <= Device->RxData.Buffers.BufferSize );
        ASSERT( 1 == Device->RxData.ReceiveBuffer[curRxIndex].Status.EOP );

        status = NetworkPortNotifyReceiveBuffer(Device->MiniportDevice, curRxIndex, len );
        ASSERT( SUCCEEDED(status));

        Device->RxData.ReceiveBuffer[curRxIndex].Status.DescriptorDone = 0;
        prevRxIndex = curRxIndex;
        curRxIndex = (curRxIndex + 1) % Device->RxData.Buffers.NumberOfDescriptors;
        EthSetRxTail(Device, prevRxIndex);

        noOfFramesReceived = noOfFramesReceived + 1;

        if (noOfFramesReceived == MaximumNumberOfFrames)
        {
            break;
        }
    }

    Device->RxData.Buffers.CurrentDescriptor = curRxIndex;

    return status;
}

_No_competing_thread_
STATUS
EthSendFrame(
    IN                              PETH_DEVICE     Device,
    IN                              WORD            DescriptorIndex,
    IN                              WORD            Length
    )
{
    WORD curTxIndex;
    PTRANSMIT_DESCRIPTOR pDescriptor;

    ASSERT( NULL != Device );
    ASSERT( Length <= Device->TxData.Buffers.BufferSize );

    curTxIndex = DescriptorIndex;
    ASSERT( curTxIndex == Device->TxData.Buffers.CurrentDescriptor );
    ASSERT( curTxIndex < Device->TxData.Buffers.NumberOfDescriptors );
    pDescriptor = &Device->TxData.TransmitBuffer[curTxIndex];
    ASSERT(pDescriptor->DescriptorDone);

    pDescriptor->Command.DEXT = FALSE;
    pDescriptor->Command.IC = FALSE;
    pDescriptor->Command.IFCS = TRUE;
    pDescriptor->Command.EOP = TRUE;
    pDescriptor->Command.IDE = TRUE;
    pDescriptor->Command.RS = TRUE;
    pDescriptor->Command.VLE = FALSE;

    pDescriptor->DescriptorDone = 0;
    pDescriptor->Length = Length;

    curTxIndex = (curTxIndex + 1) % Device->TxData.Buffers.NumberOfDescriptors;
    Device->TxData.Buffers.CurrentDescriptor = curTxIndex;

    _EthSignalTxQueueFullIfNecessary(Device);

    EthSetTxTail(Device, curTxIndex);

    return STATUS_SUCCESS;
}

_No_competing_thread_
BOOLEAN
EthHandleInterrupt(
    IN                              PETH_DEVICE     Device
    )
{
    INT_CAUSE_READ_REGISTER intReason;
    STATUS status;
    BOOLEAN bSolvedInterrupt;

    ASSERT( NULL != Device );

    status = STATUS_SUCCESS;
    bSolvedInterrupt = FALSE;

    intReason = EthGetInterruptReason(Device);
    LOG_TRACE_COMP(LogComponentNetwork | LogComponentInterrupt,
                   "intReason: 0x%x on device 0x%X\n", intReason.Raw, Device);

    // not our interrupt, sorry
    if (!intReason.IntAsserted)
    {
        return FALSE;
    }

    if (intReason.RdMinimumThresholdHit || intReason.ReceiverTimerInterrupt)
    {
        status = EthReceiveFrame(Device, 0);
        if (!SUCCEEDED(status))
        {
            LOG_FUNC_ERROR("EthReceiveFrame", status);
            return FALSE;
        }
        bSolvedInterrupt = TRUE;
    }

    if (intReason.TdWrittenBack || intReason.TxQueueEmpty)
    {
        INTR_STATE dummyState;

        LockAcquire(&Device->TxData.TxInterruptLock, &dummyState );

        // notify port driver we have free descriptors
        NetworkPortNotifyTxDescriptorAvailable(Device->MiniportDevice);

        LockRelease(&Device->TxData.TxInterruptLock, INTR_OFF );

        bSolvedInterrupt = TRUE;
    }

    if (intReason.LinkStatusChange)
    {
        DEVICE_STATUS_REGISTER devStatus = EthGetDeviceStatusRegister(Device);

        LOG("Link status is [%s]\n", devStatus.LinkUp ? "UP" : "DOWN" );

        NetworkPortNotifyLinkStatusChange(Device->MiniportDevice,
                                          (BOOLEAN) devStatus.LinkUp
                                          );

        bSolvedInterrupt = TRUE;
    }

    return bSolvedInterrupt;
}

_No_competing_thread_
void
EthChangeDeviceStatus(
    IN                              PETH_DEVICE             Device,
    IN                              PNETWORK_DEVICE_STATUS  DeviceStatus
    )
{
    BOOLEAN newRxStatus;
    BOOLEAN newTxStatus;

    ASSERT( NULL != Device );
    ASSERT( NULL != DeviceStatus );

    LOG_FUNC_START;

    newRxStatus = DeviceStatus->RxEnabled;
    newTxStatus = DeviceStatus->TxEnabled;

    if (newRxStatus ^ Device->MiniportDevice->DeviceStatus.RxEnabled)
    {
        // change RX status
        LOG("Will change RX status %u -> %u\n",
             Device->MiniportDevice->DeviceStatus.RxEnabled,
             newRxStatus
             );
        _EthChangeRxStatus(Device, newRxStatus);
    }

    if (newTxStatus ^ Device->MiniportDevice->DeviceStatus.TxEnabled)
    {
        // change TX status
        LOG("Will change TX status %u -> %u\n",
             Device->MiniportDevice->DeviceStatus.TxEnabled,
             newTxStatus
             );
        _EthChangeTxStatus(Device, newRxStatus);
    }

    LOG_FUNC_END;
}

static
PTR_SUCCESS
PVOID
_EthMapMemoryMappedBar(
    IN              PPCI_BAR            Bar,
    IN              DWORD               BytesToMap
    )
{
    PHYSICAL_ADDRESS tempPa;
    PVOID pResult;

    ASSERT( NULL != Bar );
    ASSERT( 0 != BytesToMap );

    ASSERT(0 == Bar->MemorySpace.Zero);
    ASSERT(PCI_MEM_SPACE_32_BIT == Bar->MemorySpace.Type);

    pResult = NULL;
    tempPa = PCI_GET_PA_FROM_MEM_ADDR(Bar);

    LOG_TRACE_NETWORK("BAR PA at 0x%X\n", tempPa);

    if( NULL == tempPa )
    {
        return NULL;
    }

    pResult = IoMapMemory(tempPa, BytesToMap, PAGE_RIGHTS_READWRITE);
    if (NULL == pResult)
    {
        LOG_ERROR("IoMapMemory could not map PA 0x%X\n", tempPa);
        return NULL;
    }

    return pResult;
}

static
void
_EthRetrieveMacAddress(
    IN              PETH_INTERNAL_REGS  EthRegisters,
    OUT             PMAC_ADDRESS        MacAddress
    )
{
    WORD tmp;

    ASSERT(NULL != EthRegisters);
    ASSERT(NULL != MacAddress);

    tmp = EthEepromReadWord(EthRegisters, 0x0);
    memcpy(&MacAddress->Value[0], &tmp, sizeof(WORD));

    tmp = EthEepromReadWord(EthRegisters, 0x1);
    memcpy(&MacAddress->Value[2], &tmp, sizeof(WORD));

    tmp = EthEepromReadWord(EthRegisters, 0x2);
    memcpy(&MacAddress->Value[4], &tmp, sizeof(WORD));
}

static
STATUS
_EthRxInit(
    IN      PETH_DEVICE         Device
    )
{
    STATUS status;
    PHYSICAL_ADDRESS ringBufferPa;
    RECEIVE_CONTROL_REGISTER ctrlRegister;
    RECEIVE_FILTER_CONTROL_REGISTER filterRegister;

    ASSERT(NULL != Device);

    LOG_FUNC_START;

    status = STATUS_SUCCESS;
    ringBufferPa = NULL;
    ctrlRegister.Raw = 0;
    filterRegister.Raw = 0;

    ringBufferPa = IoGetPhysicalAddress((PVOID)Device->RxData.ReceiveBuffer);
    if (NULL == ringBufferPa)
    {
        LOG_ERROR("IoGetPhysicalAddress cannot map VA 0x%X\n", Device->RxData.ReceiveBuffer);
        return STATUS_MEMORY_CANNOT_BE_MAPPED;
    }

    LOG_TRACE_NETWORK("Ring buffer PA: 0x%X\n", ringBufferPa );

    EthSetRxRingBufferAddress(Device, ringBufferPa);

    EthSetRxRingBufferSize(Device, Device->RxData.Buffers.NumberOfDescriptors * ETH_DESCRIPTOR_SIZE );

    EthSetRxHead(Device, 0 );

    // this is a HACK to simplify LIFE
    // simply state that the last descriptor is not available
    // and make it available only after the first packet is processed
    EthSetRxTail(Device, Device->RxData.Buffers.NumberOfDescriptors - 1);

    // Enable RX
    ctrlRegister.Enable = TRUE;

    // promiscuous mode
    ctrlRegister.UnicastPromiscuousEnable = TRUE;
    ctrlRegister.MulticastPromiscuousEnable = TRUE;

    // when half our descriptors are full maybe we ought to
    // interrupt
    ctrlRegister.ReceiveDescriptorMinThSize = RDMTS_HALF;

    // accept broadcast, why not
    ctrlRegister.BroadcastAcceptMode = TRUE;

    ctrlRegister.BufferSizeExtension = TRUE;
    ctrlRegister.ReceiveBufferSize = ETH_BSIZE_4KB_SEX;
    ctrlRegister.StoreBadPackets = TRUE;
    ctrlRegister.StripEthCRC = TRUE;

    EthSetRxControlRegister(Device, ctrlRegister );

    EthSetRxFilterControlRegister( Device, filterRegister );

    EthSetRxInterruptRelativeDelay( Device, 10 * MS_IN_US );
    EthSetRxInterruptAbsoluteDelay( Device, 64 * MS_IN_US );

    LOG_TRACE_NETWORK("Relative delay: %u, absolute delay: %u\n",
         EthGetRxInterruptRelativeDelay(Device),
         EthGetRxInterruptAbsoluteDelay(Device));

    LOG_FUNC_END;

    return status;
}

static
STATUS
_EthTxInit(
    IN      PETH_DEVICE         Device
    )
{
    STATUS status;
    PHYSICAL_ADDRESS ringBufferPa;
    TRANSMIT_CONTROL_REGISTER ctrlRegister;

    ASSERT( NULL != Device );

    LOG_FUNC_START;

    status = STATUS_SUCCESS;
    ctrlRegister.Raw = 0;
    ringBufferPa = NULL;

    ringBufferPa = IoGetPhysicalAddress((PVOID)Device->TxData.TransmitBuffer);
    if (NULL == ringBufferPa)
    {
        LOG_ERROR("IoGetPhysicalAddress cannot map VA 0x%X\n", Device->TxData.TransmitBuffer);
        return STATUS_MEMORY_CANNOT_BE_MAPPED;
    }

    LOG_TRACE_NETWORK("Ring buffer PA: 0x%X\n", ringBufferPa);

    EthSetTxRingBufferAddress(Device, ringBufferPa);

    EthSetTxRingBufferSize(Device, Device->TxData.Buffers.NumberOfDescriptors * ETH_DESCRIPTOR_SIZE);

    EthSetTxHead(Device, 0);

    EthSetTxTail(Device, 0);

    // enable TX
    ctrlRegister.Enable = TRUE;

    ctrlRegister.PadShortPackets = TRUE;
    ctrlRegister.CollisionDistance = IEEE_802_3_MINIMUM_FRAME_SIZE;

    EthSetTxControlRegister(Device, ctrlRegister );

    EthSetTxInterruptRelativeDelay(Device, 10 * MS_IN_US);
    EthSetTxInterruptAbsoluteDelay(Device, 64 * MS_IN_US);

    LOG_TRACE_NETWORK("Relative delay: %u, absolute delay: %u\n",
         EthGetTxInterruptRelativeDelay(Device),
         EthGetTxInterruptAbsoluteDelay(Device));

    LockInit(&Device->TxData.TxInterruptLock);

    LOG_FUNC_END;

    return status;
}

static
STATUS
_EthInterruptInit(
    IN      PETH_DEVICE         Device
    )
{
    STATUS status;
    INT_MASK_SET_REGISTER intSetMaskReg;
    INT_MASK_CLEAR_REGISTER intClearMaskReg;

    ASSERT( NULL != Device );

    LOG_FUNC_START;

    status = STATUS_SUCCESS;
    intSetMaskReg.Raw = 0;
    intClearMaskReg.Raw = MAX_DWORD;

    intClearMaskReg.__Reserved0 = 0;
    intClearMaskReg.__Reserved1 = 0;
    intClearMaskReg.__Reserved2 = 0;
    intClearMaskReg.__Reserved3 = 0;
    intClearMaskReg.__Reserved4 = 0;
    intClearMaskReg.__Reserved5 = 0;

    // clear all interrupts then set those which we want to intercept

    intSetMaskReg.RdMinimumThresholdHit = TRUE;
    intSetMaskReg.ReceiverOverrun = TRUE;
    intSetMaskReg.ReceiverTimerInterrupt = TRUE;
    intSetMaskReg.TdWrittenBack = TRUE;
    intSetMaskReg.LinkStatusChange = TRUE;

    EthSetInterruptMaskClearRegister(Device, intClearMaskReg);
    EthSetInterruptMaskSetRegister(Device, intSetMaskReg);

    LOG_FUNC_END;

    return status;
}

static
void
_EthDeviceControlsInit(
    IN      PETH_DEVICE         Device
    )
{
    DEVICE_CONTROL_REGISTER devCtrl;

    ASSERT(NULL != Device);

    devCtrl = EthGetDeviceControlRegister(Device);

    // make sure set link up is set
    devCtrl.SetLinkUp = TRUE;

    // disable VLAN
    devCtrl.VlanModeEnable = FALSE;

    EthSetDeviceControlRegister(Device, devCtrl );
}

static
void
_EthSignalTxQueueFullIfNecessary(
    IN      PETH_DEVICE         Device
    )
{
    WORD nextTxIndex;
    INTR_STATE intrState;

    ASSERT( NULL != Device );

    nextTxIndex = ( Device->TxData.Buffers.CurrentDescriptor + 1 ) % Device->TxData.Buffers.NumberOfDescriptors;

    // the check is done twice because we don't want each time we send a packet to take the interrupt
    // lock => in most cases the validation will be quick
    if (nextTxIndex == EthGetTxHead(Device))
    {
        LockAcquire(&Device->TxData.TxInterruptLock, &intrState);

        if (nextTxIndex == EthGetTxHead(Device))
        {
            LOGL("Queue is full\n");
            NetworkPortNotifyTxQueueFull(Device->MiniportDevice);
        }

        LockRelease(&Device->TxData.TxInterruptLock, intrState);
    }
}