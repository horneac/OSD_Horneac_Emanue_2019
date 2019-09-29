#include "hal_base.h"
#include "ioapic.h"
#include "ioapic_registers.h"

__forceinline
static
DWORD
_IoApicReadOffset(
    IN      PIO_APIC        IoApic,
    IN      BYTE            Offset
    )
{
    IO_APIC_REG_SEL regSel;

    ASSERT( NULL != IoApic );

    regSel.Value = 0;

    regSel.ApicAddress = Offset;

    IoApic->IoRegSel.Value = regSel.Value;

    return IoApic->IoRegData.Value;
}

__forceinline
static
void
_IoApicWriteOffset(
    IN      PIO_APIC        IoApic,
    IN      BYTE            Offset,
    IN      DWORD           Data
    )
{
    IO_APIC_REG_SEL regSel;

    ASSERT(NULL != IoApic);

    regSel.Value = 0;
    regSel.ApicAddress = Offset;

    IoApic->IoRegSel.Value = regSel.Value;
    IoApic->IoRegData.Value = Data;
}

__forceinline
static
void
_IoApicWriteRedirectionEntry(
    IN      PIO_APIC                    IoApic,
    IN      BYTE                        Index,
    IN      IO_APIC_REDIR_TABLE_ENTRY   Entry
    )
{
    BYTE offsetInIoApicRegisters;
    IO_APIC_REDIR_TABLE_ENTRY prevValue;

    ASSERT( NULL != IoApic );

    offsetInIoApicRegisters = IO_APIC_REDIRECTION_TABLE_BASE_OFFSET + Index * IO_APIC_REDIRECTION_TABLE_ENTRY_SIZE;

    // mask redirection entry
    prevValue.LowDword = _IoApicReadOffset(IoApic, offsetInIoApicRegisters );
    prevValue.Masked = TRUE;
    _IoApicWriteOffset(IoApic, offsetInIoApicRegisters, prevValue.LowDword);

    _IoApicWriteOffset(IoApic, offsetInIoApicRegisters + 1, Entry.HighDword);
    _IoApicWriteOffset(IoApic, offsetInIoApicRegisters, Entry.LowDword);
}

IO_APIC_REDIR_TABLE_ENTRY
_IoApicReadRedirectionEntry(
    IN      PIO_APIC        IoApic,
    IN      BYTE            Index
    )
{
    BYTE offsetInIoApicRegisters;
    IO_APIC_REDIR_TABLE_ENTRY value;

    ASSERT(NULL != IoApic);

    offsetInIoApicRegisters = IO_APIC_REDIRECTION_TABLE_BASE_OFFSET + Index * IO_APIC_REDIRECTION_TABLE_ENTRY_SIZE;

    value.LowDword = _IoApicReadOffset(IoApic, offsetInIoApicRegisters);
    value.HighDword = _IoApicReadOffset(IoApic, offsetInIoApicRegisters + 1);

    return value;
}

BYTE
IoApicGetId(
    IN      PVOID               IoApic
    )
{
    IO_APIC_ID_REGISTER ioApicReg;

    ASSERT( NULL != IoApic );

    ioApicReg.Value = _IoApicReadOffset(IoApic, IO_APIC_IDENTIFICATION_REGISTER_OFFSET );

    return ioApicReg.ApicId;
}

BYTE
IoApicGetVersion(
    IN      PVOID                   IoApic
    )
{
    IO_APIC_VERSION_REGISTER ioApicReg;

    ASSERT(NULL != IoApic);

    ioApicReg.Value = _IoApicReadOffset(IoApic, IO_APIC_VERSION_REGISTER_OFFSET);

    return ioApicReg.ApicVersion;
}

BYTE
IoApicGetMaximumRedirectionEntry(
    IN      PVOID               IoApic
    )
{
    IO_APIC_VERSION_REGISTER ioApicReg;

    ASSERT( NULL != IoApic );

    ioApicReg.Value = _IoApicReadOffset(IoApic, IO_APIC_VERSION_REGISTER_OFFSET);

    return ioApicReg.MaximumRedirectionEntry;
}

void
IoApicSetRedirectionTableEntry(
    IN      PVOID                   IoApic,
    IN      BYTE                    Index,
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
    IO_APIC_REDIR_TABLE_ENTRY redirEntry;

    ASSERT( NULL != IoApic );

    memzero(&redirEntry, sizeof(IO_APIC_REDIR_TABLE_ENTRY));

    redirEntry.DeliveryMode = DeliveryMode;
    redirEntry.Vector = Vector;
    redirEntry.DestinationMode = DestinationMode;
    redirEntry.Destination = Destination;
    redirEntry.Masked = TRUE;
    redirEntry.PinPolarity = PinPolarity;
    redirEntry.TriggerMode = TriggerMode;

    _IoApicWriteRedirectionEntry(IoApic, Index, redirEntry );
}

BYTE
IoApicGetRedirectionTableEntryVector(
    IN      PVOID                   IoApic,
    IN      BYTE                    Index
    )
{
    IO_APIC_REDIR_TABLE_ENTRY entry;
    
    entry = _IoApicReadRedirectionEntry(IoApic, Index);

    return (BYTE) entry.Vector;
}

void
IoApicSetRedirectionTableEntryMask(
    IN      PVOID               IoApic,
    IN      BYTE                Index,
    IN      BOOLEAN             Masked
    )
{
    IO_APIC_REDIR_TABLE_ENTRY entry;

    ASSERT( NULL != IoApic );

    entry = _IoApicReadRedirectionEntry(IoApic, Index);
    entry.Masked = Masked;
    _IoApicWriteRedirectionEntry(IoApic, Index, entry );
}

void
IoApicSendEOI(
    IN      PVOID                   IoApic,
    IN      BYTE                    Vector
    )
{
    PIO_APIC pIoApic;

    ASSERT( NULL != IoApic );

    pIoApic = IoApic;

    pIoApic->IoRegEOI.Value = Vector;
}