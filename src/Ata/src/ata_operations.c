#include "ata_base.h"
#include "ata_operations.h"
#include "ata_commands.h"
#include "ata_registers.h"

/// to remove
#include "../../HAL9000/headers/dmp_ata.h"

#define ATA_DEFAULT_IRQ_NO                                  14

static const DWORD ATA_FIXED_BASE_ADDRESS[ATA_NO_OF_CHANNELS] = { ATA_BASE_PRIMARY_CHANNEL, ATA_BASE_SECONDARY_CHANNEL };
static const DWORD ATA_FIXED_CONTROL_ADDRESS[ATA_NO_OF_CHANNELS] = { ATA_CONTROL_PRIMARY_CHANNEL, ATA_CONTROL_SECONDARY_CHANNEL };

static FUNC_InterruptFunction           _AtaDmaInterrupt;

static
void
_AtaWriteRegister(
    IN      PATA_DEVICE_REGISTERS       AtaDevice,
    IN      BYTE                        RegisterOffset,
    IN      BYTE                        Data
    );

static
BYTE
_AtaReadRegister(
    IN      PATA_DEVICE_REGISTERS       AtaDevice,
    IN      BYTE                        RegisterOffset
    );

static
void
_AtaReadBuffer(
    IN                          PATA_DEVICE_REGISTERS       AtaDevice,
    OUT_WRITES_ALL(WordsToRead) PWORD                       Buffer,
    IN                          DWORD                       WordsToRead
    );

static
void
_AtaWriteBuffer(
    IN                          PATA_DEVICE_REGISTERS       AtaDevice,
    IN_READS(WordsToRead)       PWORD                       Buffer,
    IN                          DWORD                       WordsToRead
    );

static
STATUS
_AtaPrepareDmaTransfer(
    IN          PATA_DEVICE_REGISTERS                       AtaDevice,
    INOUT       PATA_CURRENT_TRANSPER                       CurrentTransfer,
    IN          PVOID                                       Buffer,
    IN          WORD                                        SectorCount,
    IN          BOOLEAN                                     WriteOperation
    );

static
STATUS
_AtaValidateTranslationPair(
    IN          PMDL_TRANSLATION_PAIR                       TranslationPair
    );

static
void
_AtaWriteDmaRegisters(
    IN          PATA_DEVICE_REGISTERS                       AtaDevice,
    IN          DWORD                                       Prdt,
    IN          BOOLEAN                                     WriteOperation
    );

__forceinline
static
void
_AtaWaitIdle(
    IN      PATA_DEVICE_REGISTERS       Device
    )
{
    while (IsBooleanFlagOn(__inbyte(Device->BaseRegister + AtaRegisterStatus), ATA_SREG_BUSY));
}

__forceinline
static
void
_AtaWaitDataRequest(
    IN      PATA_DEVICE_REGISTERS       Device
    )
{
    while (!IsBooleanFlagOn(__inbyte(Device->BaseRegister + AtaRegisterStatus), ATA_SREG_DRQ));
}

__forceinline
static
void
_AtaWaitDeviceReady(
    IN      PATA_DEVICE_REGISTERS       Device
    )
{
    while (!IsBooleanFlagOn(__inbyte(Device->BaseRegister + AtaRegisterStatus), ATA_SREG_DRDY));
}

__forceinline
static
void
_AtaSelectDevice(
    IN      PATA_DEVICE_REGISTERS       Device,
    IN      BOOLEAN                     Slave
    )
{
    // Warning C6323 Use of arithmetic operator on Boolean type(s)
#pragma warning(suppress:6323)
    _AtaWriteRegister(Device, AtaRegisterDevice, ATA_DEV_REG_LBA | ( ATA_DEV_REG_DEV * Slave ) );
}

static
void
_AtaWriteIOParameters(
    IN      PATA_DEVICE_REGISTERS       Device,
    IN      QWORD                       LbaAddress,
    IN      WORD                        SectorCount
    )
{
    PBYTE lba;

    ASSERT(NULL != Device);

    lba = (PBYTE)&LbaAddress;

    // WRITE HIGH-order values
    _AtaWriteRegister(Device, AtaRegisterSectorCountHigh, WORD_HIGH(SectorCount));

    // bits 47:40
    _AtaWriteRegister(Device, AtaRegisterLbaHighRegisterHigh, lba[5]);

    // bits 39:32
    _AtaWriteRegister(Device, AtaRegisterLbaMidRegisterHigh, lba[4]);

    // bits 31:24
    _AtaWriteRegister(Device, AtaRegisterLbaLowRegisterHigh, lba[3]);

    // WRITE LOW-order values
    _AtaWriteRegister(Device, AtaRegisterSectorCount, WORD_LOW(SectorCount));

    // bits 23:16
    _AtaWriteRegister(Device, AtaRegisterLbaHighRegister, lba[2]);

    // bits 15:8
    _AtaWriteRegister(Device, AtaRegisterLbaMidRegister, lba[1]);

    // bits 7:0
    _AtaWriteRegister(Device, AtaRegisterLbaLowRegister, lba[0]);
}

static
void
_AtaWriteRegister(
    IN      PATA_DEVICE_REGISTERS       AtaDevice,
    IN      BYTE                        RegisterOffset,
    IN      BYTE                        Data
    )
{
    ASSERT(NULL != AtaDevice);

    if (RegisterOffset >= AtaRegisterSectorCountHigh && RegisterOffset <= AtaRegisterLbaHighRegisterHigh)
    {
        // we need to set HOB
        _AtaWriteRegister(AtaDevice, AtaRegisterDeviceControl, ATA_DCTRL_REG_HOB | AtaDevice->NoInterrupt);
    }

    if (RegisterOffset < AtaRegisterSectorCountHigh)
    {
        // normal registers
        __outbyte(AtaDevice->BaseRegister + RegisterOffset, Data);
    }
    else if( RegisterOffset < AtaRegisterAlternateStatus)
    {
        // 2 byte FIFO registers
        __outbyte(AtaDevice->BaseRegister + RegisterOffset - (AtaRegisterSectorCountHigh - AtaRegisterSectorCount), Data);
    }
    else if (RegisterOffset <= AtaRegisterDeviceAddress)
    {
        // write to control register
        __outbyte(AtaDevice->ControlBase + RegisterOffset - AtaRegisterAlternateStatus, Data);
    }
    else if (RegisterOffset < AtaRegisterPrdtAddress)
    {
        // write to bus controller
        __outbyte(AtaDevice->BusMasterBase + RegisterOffset - AtaRegisterBusCommand, Data);
    }
    else
    {
        NOT_REACHED;
    }

    if (RegisterOffset >= AtaRegisterSectorCountHigh && RegisterOffset <= AtaRegisterLbaHighRegisterHigh)
    {
        /// Note is this really necessary? According to L10 IOS writing to any register except
        /// Device Control resets HOB
        // we need to remove HOB
        _AtaWriteRegister(AtaDevice, AtaRegisterDeviceControl, AtaDevice->NoInterrupt);
    }
}

static
BYTE
_AtaReadRegister(
    IN      PATA_DEVICE_REGISTERS       AtaDevice,
    IN      BYTE                        RegisterOffset
    )
{
    BYTE result;

    result = 0;

    ASSERT(NULL != AtaDevice);

    if (RegisterOffset >= AtaRegisterSectorCountHigh && RegisterOffset <= AtaRegisterLbaHighRegisterHigh)
    {
        // we need to set HOB
        _AtaWriteRegister(AtaDevice, AtaRegisterDeviceControl, ATA_DCTRL_REG_HOB | AtaDevice->NoInterrupt);
    }

    if (RegisterOffset < AtaRegisterSectorCountHigh)
    {
        // normal registers
        result = __inbyte(AtaDevice->BaseRegister + RegisterOffset);
    }
    else if (RegisterOffset < AtaRegisterAlternateStatus)
    {
        // 2 byte FIFO registers
        result = __inbyte(AtaDevice->BaseRegister + RegisterOffset - (AtaRegisterSectorCountHigh - AtaRegisterSectorCount));
    }
    else if (RegisterOffset <= AtaRegisterDeviceAddress)
    {
        // write to control register
        // the control registers we are interested in start at BAR1 + 2
        result = __inbyte(AtaDevice->ControlBase + RegisterOffset - (AtaRegisterAlternateStatus - 2));
    }
    else if (RegisterOffset < AtaRegisterPrdtAddress)
    {
        // write to bus controller
        result = __inbyte(AtaDevice->BusMasterBase + RegisterOffset - AtaRegisterBusCommand);
    }
    else
    {
        NOT_REACHED;
    }

    if (RegisterOffset >= AtaRegisterSectorCountHigh && RegisterOffset <= AtaRegisterLbaHighRegisterHigh)
    {
        /// Note is this really necessary? According to L10 IOS writing to any register except
        /// Device Control resets HOB
        // we need to remove HOB
        _AtaWriteRegister(AtaDevice, AtaRegisterDeviceControl, AtaDevice->NoInterrupt);
    }

    return result;
}

static
void
_AtaReadBuffer(
    IN                          PATA_DEVICE_REGISTERS       AtaDevice,
    OUT_WRITES_ALL(WordsToRead) PWORD                       Buffer,
    IN                          DWORD                       WordsToRead
    )
{
    DWORD i;
    WORD tempData;

    ASSERT(NULL != AtaDevice);
    ASSERT(NULL != Buffer);

    for (i = 0; i < WordsToRead; ++i)
    {
        // wait device to be IDLE
        _AtaWaitIdle(AtaDevice);

        // wait device to be ready
        _AtaWaitDataRequest(AtaDevice);

        tempData = __inword(AtaDevice->BaseRegister + AtaRegisterData);
        memcpy(Buffer + i, &tempData, sizeof(WORD));
    }
}

static
void
_AtaWriteBuffer(
    IN                          PATA_DEVICE_REGISTERS       AtaDevice,
    IN_READS(WordsToRead)       PWORD                       Buffer,
    IN                          DWORD                       WordsToRead
    )
{
    DWORD i;

    ASSERT(NULL != AtaDevice);
    ASSERT(NULL != Buffer);

    for (i = 0; i < WordsToRead; ++i)
    {
        // wait device to be IDLE
        _AtaWaitIdle(AtaDevice);

        // wait device to be ready
        _AtaWaitDataRequest(AtaDevice);

        __outword(AtaDevice->BaseRegister + AtaRegisterData, Buffer[i]);
    }
}

static
STATUS
_AtaPrepareDmaTransfer(
    IN          PATA_DEVICE_REGISTERS                       AtaDevice,
    INOUT       PATA_CURRENT_TRANSPER                       CurrentTransfer,
    IN          PVOID                                       Buffer,
    IN          WORD                                        SectorCount,
    IN          BOOLEAN                                     WriteOperation
    )
{
    PMDL pMdl;
    STATUS status;
    DWORD byteCount;
    DWORD noOfMdlTranslationEntries;
    DWORD indexInPrdEntries;
    PPRD_ENTRY prdTable;
    DWORD i;
    PHYSICAL_ADDRESS prdtPa;
    DWORD bytesRemaining;
    DWORD allocationSize;

    LOG_FUNC_START;

    ASSERT( NULL != AtaDevice );
    ASSERT( NULL != CurrentTransfer );
    ASSERT( NULL != Buffer );
    ASSERT( 0 != SectorCount );

    pMdl = NULL;
    status = STATUS_SUCCESS;
    byteCount = SectorCount * SECTOR_SIZE;
    noOfMdlTranslationEntries = 0;
    prdTable = NULL;
    prdtPa = NULL;
    bytesRemaining = SectorCount * SECTOR_SIZE;
    indexInPrdEntries = 0;
    allocationSize = 0;

    status = IoAllocateMdl(Buffer,byteCount,NULL,&pMdl);
    if (!SUCCEEDED(status))
    {
        LOG_FUNC_ERROR("IoAllocateMdl", status);
        return status;
    }

    noOfMdlTranslationEntries = IoMdlGetNumberOfPairs(pMdl);
    ASSERT( 0 != noOfMdlTranslationEntries );

    // we allocate twice as many entries because we may have translations which cross
    // the 64KB boundary => we will split such translations into 2 distinct PRD entries
    allocationSize = sizeof(PRD_ENTRY) * noOfMdlTranslationEntries * 2;

    __try
    {
        // also we align at allocationSize so that we can make sure we do not cross page boundaries
        prdTable = IoAllocateContinuousMemoryEx(allocationSize, TRUE);
        if (NULL == prdTable)
        {
            LOG_FUNC_ERROR("HeapAllocatePoolWithTag", allocationSize);
            status = STATUS_HEAP_INSUFFICIENT_RESOURCES;
            __leave;
        }
        ASSERT(IsInSameBoundary(prdTable, allocationSize, PAGE_SIZE));

        for (i = 0; i < noOfMdlTranslationEntries; ++i)
        {
            WORD byteCountForPrd;
            WORD byteCountForAlignment;

            MDL_TRANSLATION_PAIR* pCurPair = IoMdlGetTranslationPair(pMdl, i);
            ASSERT(NULL != pCurPair);

            status = _AtaValidateTranslationPair(pCurPair);
            if (!SUCCEEDED(status))
            {
                __leave;
            }

            byteCountForPrd = (WORD)min(pCurPair->NumberOfBytes, bytesRemaining);
            byteCountForAlignment = 0;

            if (!IsInSameBoundary(pCurPair->Address, byteCountForPrd, ATA_DMA_PHYSICAL_BOUNDARY))
            {
                byteCountForAlignment = (WORD)(ATA_DMA_PHYSICAL_BOUNDARY - AddressOffset(pCurPair->Address, ATA_DMA_PHYSICAL_BOUNDARY));

                // warning C4311: 'type cast': pointer truncation from 'PHYSICAL_ADDRESS' to 'DWORD'
#pragma warning(suppress:4311)
                prdTable[indexInPrdEntries].PhysicalAddress = (DWORD)pCurPair->Address;
                prdTable[indexInPrdEntries].ByteCount = byteCountForAlignment;
                prdTable[indexInPrdEntries].LastEntry = 0;

                indexInPrdEntries++;
            }

            // warning C4311: 'type cast': pointer truncation from 'PHYSICAL_ADDRESS' to 'DWORD'
#pragma warning(suppress:4311)
            prdTable[indexInPrdEntries].PhysicalAddress = (DWORD)pCurPair->Address + byteCountForAlignment;
            prdTable[indexInPrdEntries].ByteCount = byteCountForPrd - byteCountForAlignment;
            prdTable[indexInPrdEntries].LastEntry = 0;

            // increment lbaOffset
            bytesRemaining = bytesRemaining - byteCountForPrd;

            indexInPrdEntries++;
        }

        ASSERT(0 == bytesRemaining);

        // mark last entry
        prdTable[indexInPrdEntries - 1].LastEntry = 1;

        LOG_TRACE_STORAGE("Number of entries: 0x%x\n", indexInPrdEntries);
        for (i = 0; i < indexInPrdEntries; ++i)
        {
            LOG_TRACE_STORAGE("prdTable[i].PhysicalAddress: 0x%x\n", prdTable[i].PhysicalAddress);
            LOG_TRACE_STORAGE("prdTable[i].ByteCount: 0x%x\n", prdTable[i].ByteCount);
            LOG_TRACE_STORAGE("prdTable[i].LastEntry: 0x%x\n", prdTable[i].LastEntry);
            LOG_TRACE_STORAGE("prdTable[i].Raw: 0x%X\n", prdTable[i].Raw);
        }

        prdtPa = IoGetPhysicalAddress(prdTable);
        ASSERT(NULL != prdtPa);
        if ((QWORD)prdtPa > MAX_DWORD)
        {
            status = STATUS_DEVICE_DMA_PHYSICAL_ADDRESS_TOO_HIGH;
            __leave;
        }

        LOG_TRACE_STORAGE("About to set up DMA registers!\n");

        // setup DMA registers

    // warning C4311: 'type cast': pointer truncation from 'PHYSICAL_ADDRESS' to 'DWORD'
#pragma warning(suppress:4311)
        _AtaWriteDmaRegisters(AtaDevice, (DWORD)prdtPa, WriteOperation);

        CurrentTransfer->Prdt = prdTable;
    }
    __finally
    {
        if (!SUCCEEDED(status))
        {
            if (NULL != prdTable)
            {
                IoFreeContinuousMemory(prdTable);
                prdTable = NULL;
            }
        }
        if (NULL != pMdl)
        {
            IoFreeMdl(pMdl);
            pMdl = NULL;
        }

        LOG_FUNC_END;
    }

    return status;
}

static
STATUS
_AtaValidateTranslationPair(
    IN          PMDL_TRANSLATION_PAIR                       TranslationPair
    )
{
    ASSERT( NULL != TranslationPair );

    if ((QWORD)TranslationPair->Address > ATA_DMA_MAX_PHYSICAL_ADDRESS)
    {
        return STATUS_DEVICE_DMA_PHYSICAL_ADDRESS_TOO_HIGH;
    }

    if (!IsAddressAligned(TranslationPair->Address, ATA_DMA_ALIGNMENT))
    {
        return STATUS_DEVICE_DATA_ALIGNMENT_ERROR;
    }

    if (TranslationPair->NumberOfBytes > ATA_DMA_MAX_PHYSICAL_LEN)
    {
        return STATUS_DEVICE_DMA_PHYSICAL_SPAN_TOO_LARGE;
    }

    if (!IsAddressAligned(TranslationPair->NumberOfBytes, ATA_DMA_ALIGNMENT))
    {
        return STATUS_DEVICE_DATA_ALIGNMENT_ERROR;
    }

    return STATUS_SUCCESS;
}

static
void
_AtaWriteDmaRegisters(
    IN          PATA_DEVICE_REGISTERS                       AtaDevice,
    IN          DWORD                                       Prdt,
    IN          BOOLEAN                                     WriteOperation
    )
{
    BYTE dmaStatus;

    ASSERT( NULL != AtaDevice );

    dmaStatus = _AtaReadRegister( AtaDevice, AtaRegisterBusStatus );
    LOG_TRACE_STORAGE("DMA status: 0x%x\n", dmaStatus );
    LOG_TRACE_STORAGE("Prdt: 0x%x\n", Prdt );

    // set PRDT
    __outdword(AtaDevice->BusMasterBase + AtaRegisterPrdtAddress - AtaRegisterBusCommand, Prdt);

    _AtaWriteRegister(AtaDevice, AtaRegisterBusStatus, (!WriteOperation * ATA_BUS_CMD_READ_BIT) );
}

STATUS
AtaInitialize(
    IN                              PPCI_DEVICE_DESCRIPTION     PciDevice,
    IN                              BOOLEAN                     SecondaryChannel,
    IN                              BOOLEAN                     Slave,
    IN                              PDEVICE_OBJECT              Device
    )
{
    STATUS status;
    DWORD baseAddress;
    DWORD ctrlAddress;
    DWORD busAddress;
    BYTE data;
    DWORD j;
    ATA_IDENTIFY_RESPONSE identify;
    PATA_DEVICE pDeviceExtension;
    IO_INTERRUPT ioInterrupt;
    BOOLEAN bLegacyDevice;

    LOG_FUNC_START;

    if (NULL == PciDevice)
    {
        return STATUS_INVALID_PARAMETER1;
    }

    if (NULL == Device)
    {
        return STATUS_INVALID_PARAMETER4;
    }

    status = STATUS_SUCCESS;
    data = 0;
    pDeviceExtension = NULL;
    memzero(&ioInterrupt, sizeof(IO_INTERRUPT));

    pDeviceExtension = IoGetDeviceExtension(Device);
    ASSERT(NULL != pDeviceExtension);

    baseAddress = (PciDevice->DeviceData->Header.Device.Bar[2 * BooleanToInteger(SecondaryChannel)].IoSpace.Address) << 2;
    ctrlAddress = (PciDevice->DeviceData->Header.Device.Bar[2 * BooleanToInteger(SecondaryChannel) + 1].IoSpace.Address) << 2;
    busAddress = (PciDevice->DeviceData->Header.Device.Bar[4].IoSpace.Address << 2) + (8 * BooleanToInteger(SecondaryChannel));

    ASSERT(baseAddress <= MAX_WORD);
    ASSERT(ctrlAddress <= MAX_WORD);
    ASSERT(busAddress <= MAX_WORD);

    LOG_TRACE_STORAGE("Base address: 0x%x\n", baseAddress);
    LOG_TRACE_STORAGE("Ctrl address: 0x%x\n", ctrlAddress);
    LOG_TRACE_STORAGE("Bus address: 0x%x\n", busAddress);
    LOG_TRACE_STORAGE("Bars[4].IoSpace.Address: 0x%x\n", PciDevice->DeviceData->Header.Device.Bar[4].IoSpace.Address << 2);
    LOG_TRACE_STORAGE("Secondary channel: 0x%x\n", SecondaryChannel);
    LOG_TRACE_STORAGE("Slave: 0x%x\n", Slave);

    bLegacyDevice = ( 0 == baseAddress ) && ( 0 == ctrlAddress );
    ASSERT( bLegacyDevice || ( ( 0 != baseAddress ) && ( 0 != ctrlAddress ) ) );

    pDeviceExtension->DeviceRegisters.BaseRegister = (WORD)((0 != baseAddress) ? baseAddress : ATA_FIXED_BASE_ADDRESS[SecondaryChannel]);

    // we subtract 2 from ctrlAddress because BAR gives of address of ctrlRegisters + 2 (dunno why)
    pDeviceExtension->DeviceRegisters.ControlBase = (WORD)((0 != ctrlAddress) ? (ctrlAddress + 2) : ATA_FIXED_CONTROL_ADDRESS[SecondaryChannel]);
    pDeviceExtension->DeviceRegisters.BusMasterBase = (WORD) busAddress;
    pDeviceExtension->DeviceRegisters.NoInterrupt = ATA_DCTRL_REG_NIEN;
    pDeviceExtension->SecondaryChannel = SecondaryChannel;
    pDeviceExtension->Slave = Slave;

    LOG_TRACE_STORAGE("Base address: 0x%x\n", pDeviceExtension->DeviceRegisters.BaseRegister);
    LOG_TRACE_STORAGE("Ctrl address: 0x%x\n", pDeviceExtension->DeviceRegisters.ControlBase);

    // disable interrupts on current drive
    _AtaWriteRegister(&pDeviceExtension->DeviceRegisters, AtaRegisterDeviceControl, pDeviceExtension->DeviceRegisters.NoInterrupt);

    // select drive
    _AtaSelectDevice(&pDeviceExtension->DeviceRegisters, Slave);

    // disable interrupts on selected drive
    _AtaWriteRegister(&pDeviceExtension->DeviceRegisters, AtaRegisterDeviceControl, pDeviceExtension->DeviceRegisters.NoInterrupt);

    LOG_TRACE_STORAGE("Device selected\n");

    /// sleep a little
    for (j = 0; j < 10; ++j)
        _AtaReadRegister(&pDeviceExtension->DeviceRegisters,AtaRegisterAlternateStatus);

    LOG_TRACE_STORAGE("About to send identify\n");

    // send identify command
    _AtaWriteRegister(&pDeviceExtension->DeviceRegisters, AtaRegisterCommand, ATA_CMD_IDENTIFY);

    /// sleep a little
    for (j = 0; j < 10; ++j)
        _AtaReadRegister(&pDeviceExtension->DeviceRegisters, AtaRegisterAlternateStatus);

    if (0 == _AtaReadRegister(&pDeviceExtension->DeviceRegisters, AtaRegisterStatus))
    {
        // no device
        status = STATUS_DEVICE_DOES_NOT_EXIST;
        LOG("Device does not exist\n");
        return status;
    }

    LOG_TRACE_STORAGE("Device does exist\n");

    // wait for device to be ready
#pragma warning(suppress:4127)
    while (TRUE)
    {
        data = _AtaReadRegister(&pDeviceExtension->DeviceRegisters, AtaRegisterStatus);
        if (IsBooleanFlagOn(data, ATA_SREG_ERR))
        {
            LOG("Not ATA, maybe ATAPI\n");
            status = STATUS_DEVICE_NOT_SUPPORTED;
            return status;
        }

        // if device is not busy and it is ready we can continue
        if ((!IsBooleanFlagOn(data, ATA_SREG_BUSY)) && (IsBooleanFlagOn(data, ATA_SREG_DRQ)))
        {
            break;
        }
    }

    // specify we do not want interrupts
    _AtaWriteRegister(&pDeviceExtension->DeviceRegisters, AtaRegisterDeviceControl, pDeviceExtension->DeviceRegisters.NoInterrupt);

    // lets read identify command response
    _AtaReadBuffer(&pDeviceExtension->DeviceRegisters, (PWORD) &identify, SECTOR_SIZE / sizeof(WORD));

    if (LogIsComponentTraced(LogComponentStorage))
    {
        DumpAtaIdentifyCommand(&identify);
    }

    if (!identify.Features.SupportLba48)
    {
        status = STATUS_DEVICE_NOT_SUPPORTED;
        LOG_WARNING("We do not know how to operate disks without 48-bit address support :(\n");
        return status;
    }

    pDeviceExtension->TotalSectors = identify.Address48Bit;

    // initialize current transfer structure
    _InterlockedExchange(&pDeviceExtension->CurrentTransfer.State, AtaTransferStateFree);
    status = ExEventInit(&pDeviceExtension->CurrentTransfer.TransferReady, ExEventTypeSynchronization, FALSE );
    if (!SUCCEEDED(status))
    {
        LOG_FUNC_ERROR("ExEventInit", status );
        return status;
    }

    ioInterrupt.Type = bLegacyDevice ? IoInterruptTypeLegacy : IoInterruptTypePci;
    ioInterrupt.Irql = IrqlStorageLevel;
    ioInterrupt.ServiceRoutine = _AtaDmaInterrupt;
    ioInterrupt.Exclusive = FALSE;

    if (bLegacyDevice)
    {
        ioInterrupt.Legacy.Irq = ATA_DEFAULT_IRQ_NO + BooleanToInteger(SecondaryChannel);
    }
    else
    {
        ioInterrupt.Pci.PciDevice = PciDevice;
    }

    status = IoRegisterInterrupt( &ioInterrupt, Device );
    if (!SUCCEEDED(status))
    {
        LOG_FUNC_ERROR("IoRegisterInterrupt", status);
        return status;
    }

    // make sure DMA transfer is stopped
    _AtaWriteRegister(&pDeviceExtension->DeviceRegisters, AtaRegisterBusCommand, 0 );

    pDeviceExtension->Initialized = TRUE;

    return status;
}

STATUS
AtaReadWriteSectors(
    IN                                          PATA_DEVICE     Device,
    IN                                          QWORD           SectorIndex,
    IN                                          WORD            SectorCount,
    _When_(WriteOperation,OUT_WRITES_BYTES(SectorCount*SECTOR_SIZE))
    _When_(!WriteOperation,IN_READS_BYTES(SectorCount*SECTOR_SIZE))
                                                PVOID           Buffer,
    OUT                                         WORD*           SectorsReadWriten,
    IN                                          BOOLEAN         Asynchronous,
    IN                                          BOOLEAN         WriteOperation
    )
{
    STATUS status;
    PATA_DEVICE_REGISTERS pDevRegisters;
    BYTE ataCmd;

    if (NULL == Device)
    {
        return STATUS_INVALID_PARAMETER1;
    }

    if (0 == SectorCount)
    {
        return STATUS_INVALID_PARAMETER3;
    }

    if (NULL == Buffer)
    {
        return STATUS_INVALID_PARAMETER4;
    }

    if (NULL == SectorsReadWriten)
    {
        return STATUS_INVALID_PARAMETER5;
    }

    status = STATUS_SUCCESS;
    pDevRegisters = &Device->DeviceRegisters;

    // 1. wait for device to become idle
    _AtaWaitIdle(pDevRegisters);

    LOG_TRACE_STORAGE("Device is idle\n");

    // 2. select device
    _AtaSelectDevice(pDevRegisters, Device->Slave);

    LOG_TRACE_STORAGE("Device selected\n");

    // 3. wait device to be IDLE and ready
    _AtaWaitIdle(pDevRegisters);

    LOG_TRACE_STORAGE("Device is idle\n");

    // wait device to be ready
    _AtaWaitDeviceReady(pDevRegisters);

    LOG_TRACE_STORAGE("Device is ready\n");
    LOG_TRACE_STORAGE("Asynchronous: 0x%x\n", Asynchronous );

    // we don't want interrupts if we're performing a synchronous transfer
    pDevRegisters->NoInterrupt = Asynchronous ? 0 : ATA_DCTRL_REG_NIEN;

    // specify if we want interrupts or not
    _AtaWriteRegister(pDevRegisters, AtaRegisterDeviceControl, pDevRegisters->NoInterrupt);

    if (pDevRegisters->NoInterrupt)
    {
        LOG_TRACE_STORAGE("We don't want interrupts\n");
    }
    else
    {
        LOG_TRACE_STORAGE("We'll use interrupts\n");
    }

    // 4. write command parameters
    _AtaWriteIOParameters(pDevRegisters, SectorIndex, SectorCount);
    LOG_TRACE_STORAGE("IO parameters written\n");

    if (Asynchronous)
    {
        // 4.5 write DMA parameters

        // configure bus master register for DMA transfer
        status = _AtaPrepareDmaTransfer(pDevRegisters, &Device->CurrentTransfer, Buffer, SectorCount, WriteOperation );
        if (!SUCCEEDED(status))
        {
            LOG_FUNC_ERROR("_AtaPrepareDmaTransfer", status);
            return status;
        }

        LOG_TRACE_STORAGE("DMA parameters written\n");
    }

    // set command type
    if (WriteOperation)
    {
        ataCmd = Asynchronous ? ATA_CMD_WRITE_DMA_EXT : ATA_CMD_WRITE_SECTORS_EXT;
    }
    else
    {
        ataCmd = Asynchronous ? ATA_CMD_READ_DMA_EXT : ATA_CMD_READ_SECTORS_EXT;
    }

    // 5. write command
    _AtaWriteRegister(pDevRegisters, AtaRegisterCommand, ataCmd);

    if (Asynchronous)
    {
        // set direction, it's weird this must be set after the command is written
        // yeah Read => bit set, Write => bit cleared
        _AtaWriteRegister(pDevRegisters, AtaRegisterBusCommand, (!WriteOperation * ATA_BUS_CMD_READ_BIT) | ATA_BUS_CMD_START_BIT);

        // wait for DMA transfer to complete
        ExEventWaitForSignal(&Device->CurrentTransfer.TransferReady);

        // check if transfer actually finished
        ASSERT( AtaTransferStateFinished == _InterlockedAnd( &Device->CurrentTransfer.State, MAX_DWORD ) );
        ASSERT( NULL != Device->CurrentTransfer.Prdt );

        LOG_TRACE_STORAGE("About to free PRDT\n");
        IoFreeContinuousMemory(Device->CurrentTransfer.Prdt );
        Device->CurrentTransfer.Prdt = NULL;

        LOG_TRACE_STORAGE("DMA transfer complete\n");
    }
    else
    {
        // read buffer synchronously
        if (WriteOperation)
        {
            _AtaWriteBuffer(pDevRegisters, Buffer, SectorCount * (SECTOR_SIZE / sizeof(WORD)));

            LOG_TRACE_STORAGE("Buffer written\n");
        }
        else
        {
            _AtaReadBuffer(pDevRegisters, Buffer, SectorCount * (SECTOR_SIZE / sizeof(WORD)));

            LOG_TRACE_STORAGE("Buffer read\n");
        }
    }

    *SectorsReadWriten = SectorCount;

    status = STATUS_SUCCESS;

    return status;
}

BOOLEAN
(__cdecl _AtaDmaInterrupt)(
    IN      PDEVICE_OBJECT  Device
    )
{
    PATA_DEVICE pAtaDev;
    PATA_DEVICE_REGISTERS pDevRegisters;
    BYTE busStatus;
    BYTE devStatus;

    LOG_FUNC_START;

    ASSERT( NULL != Device );

    pAtaDev = IoGetDeviceExtension(Device);
    ASSERT( NULL != pAtaDev );

    pDevRegisters = &pAtaDev->DeviceRegisters;

    _AtaSelectDevice(pDevRegisters, pAtaDev->Slave);

    busStatus = _AtaReadRegister(pDevRegisters, AtaRegisterBusStatus);
    if (!IsBooleanFlagOn(busStatus, ATA_BUS_DMA_IRQ))
    {
        // means other device is interrupting
        LOGL("This is not the interrupt we're looking for!");
        return FALSE;
    }

    devStatus = _AtaReadRegister(pDevRegisters, AtaRegisterStatus);
    ASSERT(!IsBooleanFlagOn(devStatus, ATA_SREG_DF));

    if (IsBooleanFlagOn(devStatus, ATA_SREG_ERR))
    {
        LOG_ERROR("DMA command failed, error register: 0x%x\n", _AtaReadRegister(pDevRegisters, AtaRegisterError ));
        NOT_REACHED;
    }

    // must set Stop bit in command register
    _AtaWriteRegister(pDevRegisters, AtaRegisterBusCommand, 0 );

    ASSERT( AtaTransferStateInProgress == _InterlockedCompareExchange( &pAtaDev->CurrentTransfer.State, AtaTransferStateFinished, AtaTransferStateInProgress ) );

    ExEventSignal(&pAtaDev->CurrentTransfer.TransferReady);

    // clear IRQ bit
    // apparently this status register is R/W
    _AtaWriteRegister(&pAtaDev->DeviceRegisters, AtaRegisterBusStatus, ATA_BUS_DMA_IRQ );

    LOG_FUNC_END;

    // we solved the interrupt
    return TRUE;
}