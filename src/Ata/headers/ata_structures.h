#pragma once

#include "ex_event.h"

typedef enum _ATA_TRANSFER_STATE
{
    AtaTransferStateFree,
    AtaTransferStateInProgress,
    AtaTransferStateFinished
} ATA_TRANSFER_STATE;

typedef struct _ATA_CURRENT_TRANSFER
{
    volatile DWORD              State;
    EX_EVENT                    TransferReady;

    union _PRD_ENTRY*           Prdt;
} ATA_CURRENT_TRANSFER, *PATA_CURRENT_TRANSPER;

typedef struct _ATA_DEVICE_REGISTERS
{
    WORD                        BaseRegister;
    WORD                        ControlBase;
    WORD                        BusMasterBase;
    BYTE                        NoInterrupt;
} ATA_DEVICE_REGISTERS, *PATA_DEVICE_REGISTERS;

typedef struct _ATA_DEVICE
{
    ATA_DEVICE_REGISTERS        DeviceRegisters;
    QWORD                       TotalSectors;
    BOOLEAN                     SecondaryChannel;
    BOOLEAN                     Slave;
    BOOLEAN                     Initialized;

    ATA_CURRENT_TRANSFER        CurrentTransfer;
} ATA_DEVICE, *PATA_DEVICE;