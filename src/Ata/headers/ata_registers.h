#pragma once

typedef enum _ATA_REGISTER
{
    AtaRegisterData,                                        // 0x0  16b register
    AtaRegisterError,                                       // 0x1
    AtaRegisterFeatures = AtaRegisterError,                 // 0x1  Read - Write
    AtaRegisterSectorCount,                                 // 0x2
    AtaRegisterLbaLowRegister,                              // 0x3
    AtaRegisterLbaMidRegister,                              // 0x4
    AtaRegisterLbaHighRegister,                             // 0x5
    AtaRegisterDevice,                                      // 0x6
    AtaRegisterStatus,                                      // 0x7
    AtaRegisterCommand = AtaRegisterStatus,                 // 0x7  Read - Write

                                                            // these 4 registers are pseudo-registers
                                                            // When using 48-bit LBA addressing these registers together with
                                                            // their counterparts represent a two-byte deep FIFO
    AtaRegisterSectorCountHigh,                             // 0x8 (0x2)
    AtaRegisterLbaLowRegisterHigh,                          // 0x9 (0x3)
    AtaRegisterLbaMidRegisterHigh,                          // 0xA (0x4)
    AtaRegisterLbaHighRegisterHigh,                         // 0xB (0x5)

                                                            // these 3 registers start at a different base
    AtaRegisterAlternateStatus,                             // 0xC
    AtaRegisterDeviceControl = AtaRegisterAlternateStatus,  // 0xC Read - Write
    AtaRegisterDeviceAddress,                               // 0xD

                                                            // registers for the Bus controller
    AtaRegisterBusCommand,                                  // 0xE
    AtaRegisterBusStatus = AtaRegisterBusCommand + 2,       // 0x10
    AtaRegisterPrdtAddress = AtaRegisterBusStatus + 2,      // 0x12
    AtaRegisterReserved = AtaRegisterPrdtAddress + 4        // 0x16
} ATA_REGISTER;

// Status register values
// device is busy
#define ATA_SREG_BUSY                           (1<<7)

// device is ready
#define ATA_SREG_DRDY                           (1<<6)

// device fault
#define ATA_SREG_DF                             (1<<5)

// disk drive is ready to transmit data
#define ATA_SREG_DRQ                            (1<<3)

// error
#define ATA_SREG_ERR                            (1<<0)

// Error register values

// Device register values
#define ATA_DEV_REG_LBA                         (1<<6)
#define ATA_DEV_REG_DEV                         (1<<4)

// Device control register values
#define ATA_DCTRL_REG_HOB                       (1<<7)
#define ATA_DCTRL_REG_NIEN                      (1<<1)

// BUS command
#define ATA_BUS_CMD_START_BIT                   (1<<0)
#define ATA_BUS_CMD_READ_BIT                    (1<<3)

// Status command
#define ATA_BUS_DMA_IRQ                         (1<<2)
#define ATA_BUS_STATUS_MASTER_DMA_INITIALIZED   (1<<5)
#define ATA_BUS_STATUS_SLAVE_DMA_INITIALIZED    (1<<6)

#define ATA_BASE_PRIMARY_CHANNEL                0x1F0
#define ATA_BASE_SECONDARY_CHANNEL              0x170

#define ATA_CONTROL_PRIMARY_CHANNEL             0x3F6
#define ATA_CONTROL_SECONDARY_CHANNEL           0x376

#define ATA_NO_OF_CHANNELS                      2

#define ATA_PRD_ENTRY_PREDEFINED_SIZE           8
#define ATA_PRD_ALIGNMENT                       4

#define ATA_DMA_MAX_PHYSICAL_LEN                (64 * KB_SIZE - 1)
#define ATA_DMA_PHYSICAL_BOUNDARY               (64 * KB_SIZE)
#define ATA_DMA_MAX_PHYSICAL_ADDRESS            MAX_DWORD
#define ATA_DMA_ALIGNMENT                       4

// PRD (Physical Region Descriptor)
#pragma pack(push,1)

#pragma warning(push)

// warning C4201: nonstandard extension used: nameless struct/union
#pragma warning(disable:4201)

typedef union _PRD_ENTRY
{
    struct
    {
        // Physical address of the buffer
        // addresses must be below 4 GB

        // And this address CANNOT cross a 64K boundary
        // Q: Does this mean 0xF000 -> 0x11000 is not valid as an example?
        // A: Yes
        DWORD                               PhysicalAddress;

        // ByteCount of 0 => 64K
        // Must match the sector count else controller error
        WORD                                ByteCount;

        WORD                                __Reserved0 : 15;

        // if set => this is the last entry in the PRD table
        WORD                                LastEntry : 1;
    };

    QWORD                                   Raw;
} PRD_ENTRY, *PPRD_ENTRY;
STATIC_ASSERT(ATA_PRD_ENTRY_PREDEFINED_SIZE == sizeof(PRD_ENTRY));

#pragma warning(pop)
#pragma pack(pop)
