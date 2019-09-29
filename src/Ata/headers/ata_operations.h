#pragma once

#define ATA_NO_OF_BARS_USED     5

STATUS
AtaInitialize(
    IN                              PPCI_DEVICE_DESCRIPTION     PciDevice,
    IN                              BOOLEAN                     SecondaryChannel,
    IN                              BOOLEAN                     Slave,
    IN                              PDEVICE_OBJECT              Device
    );

STATUS
AtaReadWriteSectors(
    IN                              PATA_DEVICE                 Device,
    IN                              QWORD                       SectorIndex,
    IN                              WORD                        SectorCount,
    _When_(WriteOperation, OUT_WRITES_BYTES(SectorCount*SECTOR_SIZE))
    _When_(!WriteOperation, IN_READS_BYTES(SectorCount*SECTOR_SIZE))
                                    PVOID                       Buffer,
    OUT                             WORD*                       SectorsReadWriten,
    IN                              BOOLEAN                     Asynchronous,
    IN                              BOOLEAN                     WriteOperation
    );