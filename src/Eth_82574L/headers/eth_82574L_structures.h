#pragma once

#include "eth_82574L_regs.h"
#include "lock_common.h"

#define INTEL_82574L_DEV_ID                     0x10D3

#define ETH_NO_OF_BARS_USED                     4

// actual size is 128 * KB_SIZE but we don't want to map it all
#define ETH_INTERNAL_REGISTER_SIZE              (32*KB_SIZE)

// actual size is between 64 KB and 16MB
#define ETH_FLASH_SIZE                          (4*KB_SIZE)
#define ETH_MSI_X_TABLES_SIZE                   (16*KB_SIZE)

#define ETH_OFFSET_ICR                          0x00C0
#define ETH_OFFSET_IMS                          0x00D0
#define ETH_OFFSET_RCTL                         0x0100
#define ETH_OFFSET_TCTL                         0x0400
#define ETH_OFFSET_RDBAL                        0x2800
#define ETH_OFFSET_TDBAL                        0x3800
#define ETH_OFFSET_RFCTL                        0x5008
#define ETH_OFFSET_TO_IP_ADDRESS_VALID          0x5838

#define ETH_DESCRIPTOR_SIZE                     16

#define ETH_DESCRIPTOR_BUFFER_ALIGNMENT         128
#define ETH_DATA_BUFFER_ALIGNMENT               4

#define ETH_NO_OF_RX_DESCS                      32
#define ETH_NO_OF_TX_DESCS                      32

#define ETH_BUFFER_SIZE                         4*KB_SIZE

#define ETH_BSIZE_4KB_SEX                       0b11

#pragma pack(push,1)

#pragma warning(push)

// warning C4201: nonstandard extension used: nameless struct/union
#pragma warning(disable:4201)

// warning C4214: nonstandard extension used: bit field types other than int
#pragma warning(disable:4214)

typedef union _EERD_REGISTER
{
    struct
    {
        WORD        Start                       :   1;
        WORD        Done                        :   1;

        // address in words
        WORD        Address                     :  14;
        WORD        Data;
    };
    DWORD           Raw;
} EERD_REGISTER, *PEERD_REGISTER;
STATIC_ASSERT(sizeof(EERD_REGISTER) == ETH_INTERNAL_REG_SIZE);

typedef struct _ETH_INTERNAL_REGS
{
    // 0x0 - 0x4 - RW
    VOL_DWORD                               DeviceControlRegister[2];

    // 0x8 - R
    VOL_DWORD                               DeviceStatusRegister;

    // 0xC
    VOL_DWORD                               __Reserved0;

    // 0x10 - RW/R0
    VOL_DWORD                               EepromFlashControlRegister;

    // 0x14 - RW
    VOL_DWORD                               EepromReadRegister;

    BYTE                                    __Reserved1[0xA8];

    // 0xC0 - RC/WC
    VOL_DWORD                               InterruptCauseReadRegister;

    BYTE                                    __Reserved99[0xC];

    // 0xD0 - RW
    VOL_DWORD                               InterruptMaskSetRegister;

    VOL_DWORD                               __Reserved98;

    // 0xD8 - W
    VOL_DWORD                               InterruptMaskClearRegister;

    BYTE                                    __Reserved2[0x24];

    // 0x100 - RW
    VOL_DWORD                               ReceiveControlRegister;

    BYTE                                    __Reserved3[0x2FC];

    // 0x400 - RW
    VOL_DWORD                               TransmitControlRegister;

    BYTE                                    __Reserved4[0x23FC];

    // 0x2800 - RW
    VOL_DWORD                               ReceiveDescriptorAddressLow;

    // 0x2804 - RW
    VOL_DWORD                               ReceiveDescriptorAddressHigh;

    // 0x2808 - RW
    VOL_DWORD                               ReceiveDescriptorLength;

    VOL_DWORD                               __Reserved5;

    // 0x2810 - RW
    VOL_DWORD                               ReceiveDescriptorHead;

    VOL_DWORD                               __Reserved6;

    // 0x2818 - RW
    VOL_DWORD                               ReceiveDescriptorTail;

    VOL_DWORD                               __Reserved7;

    // 0x2820 - RW
    VOL_DWORD                               ReceiveInterruptRelativeDelayTimer;

    VOL_DWORD                               __Reserved8;

    VOL_DWORD                               __Reserved9;

    // 0x282C - RW
    VOL_DWORD                               ReceiveInterruptAbsoluteDelayTimer;

    BYTE                                    __Reserved10[0xFD0];

    // 0x3800 - RW
    VOL_DWORD                               TransmitDescriptorAddressLow;

    // 0x3804 - RW
    VOL_DWORD                               TransmitDescriptorAddressHigh;

    // 0x3808 - RW
    VOL_DWORD                               TransmitDescriptorLength;

    VOL_DWORD                               __Reserved11;

    // 0x3810 - RW
    VOL_DWORD                               TransmitDescriptorHead;

    VOL_DWORD                               __Reserved12;

    // 0x3818 - RW
    VOL_DWORD                               TransmitDescriptorTail;

    VOL_DWORD                               __Reserved13;

    // 0x3820 - RW
    VOL_DWORD                               TransmitInterruptRelativeDelayTimer;

    VOL_DWORD                               __Reserved14[2];

    // 0x382C - RW
    VOL_DWORD                               TransmitInterruptAbsoluteDelayTimer;

    BYTE                                    __Reserved15[0x17D8];

    // 0x5008 - RW
    VOL_DWORD                               ReceiveFilterControlRegister;

    BYTE                                    __Reserved16[0x82C];

    // 0x5838 - RW
    VOL_DWORD                               IpAddressValid;

    // 0x583C
    VOL_DWORD                               __Reserved17;

    // 0x5840
    VOL_DWORD                               IpAddress0;
} ETH_INTERNAL_REGS, *PETH_INTERNAL_REGS;
STATIC_ASSERT(FIELD_OFFSET(ETH_INTERNAL_REGS,InterruptCauseReadRegister) == ETH_OFFSET_ICR);
STATIC_ASSERT(FIELD_OFFSET(ETH_INTERNAL_REGS,InterruptMaskSetRegister) == ETH_OFFSET_IMS);
STATIC_ASSERT(FIELD_OFFSET(ETH_INTERNAL_REGS,ReceiveControlRegister) == ETH_OFFSET_RCTL);
STATIC_ASSERT(FIELD_OFFSET(ETH_INTERNAL_REGS,TransmitControlRegister) == ETH_OFFSET_TCTL);
STATIC_ASSERT(FIELD_OFFSET(ETH_INTERNAL_REGS,ReceiveDescriptorAddressLow) == ETH_OFFSET_RDBAL);
STATIC_ASSERT(FIELD_OFFSET(ETH_INTERNAL_REGS,TransmitDescriptorAddressLow) == ETH_OFFSET_TDBAL);
STATIC_ASSERT(FIELD_OFFSET(ETH_INTERNAL_REGS,ReceiveFilterControlRegister) == ETH_OFFSET_RFCTL);
STATIC_ASSERT(FIELD_OFFSET(ETH_INTERNAL_REGS,IpAddressValid) == ETH_OFFSET_TO_IP_ADDRESS_VALID );
STATIC_ASSERT(sizeof(ETH_INTERNAL_REGS) <= ETH_INTERNAL_REGISTER_SIZE);

typedef struct _RECEIVE_DESCRIPTOR_SHADOW
{
    PHYSICAL_ADDRESS                        BufferAddress;
    WORD                                    Length;
    WORD                                    Checksum;
    union
    {
        struct
        {
            BYTE                            DescriptorDone      : 1;
            BYTE                            EOP                 : 1;
            BYTE                            __Reserved0         : 1;

            // Packet is 802.1q (matched VET)
            BYTE                            VP                  : 1;

            // UDP checksum calculated on packet
            BYTE                            UDPCS               : 1;

            // TCP checksum calculated on packet
            BYTE                            TCPCS               : 1;

            // IPv4 checksum calculated on packet
            BYTE                            IPCS                : 1;
            BYTE                            __Reserved1         : 1;
        };
        BYTE                                Raw;
    } Status;
    BYTE                                    Errors;
    WORD                                    VlanTag;
} RECEIVE_DESCRIPTOR_SHADOW, *PRECEIVE_DESCRIPTOR_SHADOW;
typedef volatile RECEIVE_DESCRIPTOR_SHADOW RECEIVE_DESCRIPTOR, *PRECEIVE_DESCRIPTOR;
STATIC_ASSERT(sizeof(RECEIVE_DESCRIPTOR_SHADOW) == ETH_DESCRIPTOR_SIZE);

typedef struct _TRANSMIT_DESCRIPTOR_SHADOW
{
    PHYSICAL_ADDRESS                        BufferAddress;
    WORD                                    Length;
    BYTE                                    ChecksumOffset;
    struct
    {
        // EOP stands for end - of - packet and when set, indicates the last descriptor making up the
        // packet.
        BYTE                                EOP                 : 1;

        // When IFCS is set, hardware appends the MAC FCS at the end of the packet.When
        // cleared, software should calculate the FCS for proper CRC check.The software must set
        // IFCS in the following instances :
        //  • Transmission of short packets while padding is enabled by the TCTL.PSP bit
        //  • Checksum offload is enabled by the IC bit in the TDESC.CMD
        //  • VLAN header insertion enabled by the VLE bit in the TDESC.CMD
        //  • Large send or TCP / IP checksum offload using context descriptor
        BYTE                                IFCS                : 1;

        // When IC is set, hardware inserts a checksum value calculated from the CSS bit value to
        // the CSE bit value, or to the end of packet.The checksum value is inserted in the header
        // at the CSO bit value location.One or many descriptors can be used to form a packet.
        // Checksum calculations are for the entire packet starting at the byte indicated by the
        // CSS field.A value of zero for CSS corresponds to the first byte in the packet.CSS must
        // be set in the first descriptor for a packet.In addition, IC is ignored if CSO or CSS are
        // out of range.This occurs if () or ().
        BYTE                                IC                  : 1;

        // When the RS bit is set, hardware writes back the DD bit once the DMA fetch completes.
        BYTE                                RS                  : 1;

        BYTE                                __Reserved0         : 1;

        // The DEXT bit identifies this descriptor as either a legacy or an extended descriptor type
        // and must be set to 0b to indicate legacy descriptor.
        BYTE                                DEXT                : 1;

        // VLE indicates that the packet is a VLAN packet(for example, that the hardware should
        // add the VLAN Ether type and an 802.1q VLAN tag to the packet)
        BYTE                                VLE                 : 1;

        // IDE activates a transmit interrupt delay timer.Hardware loads a countdown register
        // when it writes back a transmit descriptor that has RS and IDE set.The value loaded
        // comes from the IDV field of the Interrupt Delay(TIDV) register.When the count
        // reaches zero, a transmit interrupt occurs if transmit descriptor write - back interrupts
        // (TXDW) are enabled.Hardware always loads the transmit interrupt counter whenever it
        // processes a descriptor with IDE set even if it is already counting down due to a
        // previous descriptor.If hardware encounters a descriptor that has RS set, but not IDE, it
        // generates an interrupt immediately after writing back the descriptor and clears the
        // interrupt delay timer.Setting the IDE bit has no meaning without setting the RS bit.
        BYTE                                IDE                 : 1;
    } Command;
    struct
    {
        // DD indicates that the descriptor is done and is written back after the descriptor has
        // been processed(assuming the RS bit was set).The DD bit can be used as an indicator
        // to the software that all descriptors, in the memory descriptor ring, up to and including
        // the descriptor with the DD bit set are again available to the software.
        BYTE                                DescriptorDone      : 1;

        BYTE                                __Reserved0         : 3;

        BYTE                                TS                  : 1;

        // The TS bit indicates to the 82574 to put a time stamp on the packet designated by the
        // descriptor.
        BYTE                                __Reserved1         : 3;
    };
    BYTE                                    ChecksumStart;
    WORD                                    VLAN;
} TRANSMIT_DESCRIPTOR_SHADOW, *PTRANSMIT_DESCRIPTOR_SHADOW;
typedef volatile TRANSMIT_DESCRIPTOR_SHADOW TRANSMIT_DESCRIPTOR, *PTRANSMIT_DESCRIPTOR;
STATIC_ASSERT(sizeof(TRANSMIT_DESCRIPTOR_SHADOW) == ETH_DESCRIPTOR_SIZE);

#pragma pack(pop)

typedef struct _ETH_BUFFERS
{
    WORD                                    NumberOfDescriptors;
    WORD                                    CurrentDescriptor;
    WORD                                    BufferSize;
} ETH_BUFFERS, *PETH_BUFFERS;

typedef struct _RX_DATA
{
    PRECEIVE_DESCRIPTOR                     ReceiveBuffer;
    ETH_BUFFERS                             Buffers;
} RX_DATA, *PRX_DATA;

typedef struct _TX_DATA
{
    PTRANSMIT_DESCRIPTOR                    TransmitBuffer;
    ETH_BUFFERS                             Buffers;
    LOCK                                    TxInterruptLock;
} TX_DATA, *PTX_DATA;

#pragma warning(pop)

typedef struct _ETH_DEVICE
{
    struct _MINIPORT_DEVICE*                MiniportDevice;

    PETH_INTERNAL_REGS                      InternalRegisters;
    volatile DWORD*                         Flash;
    volatile DWORD*                         MsiX;

    RX_DATA                                 RxData;
    TX_DATA                                 TxData;
} ETH_DEVICE, *PETH_DEVICE;

// General
DEVICE_CONTROL_REGISTER
EthGetDeviceControlRegister(
    IN      PETH_DEVICE         Device
    );

void
EthSetDeviceControlRegister(
    IN      PETH_DEVICE                 Device,
    IN      DEVICE_CONTROL_REGISTER     ControlRegister
    );

DEVICE_STATUS_REGISTER
EthGetDeviceStatusRegister(
    IN      PETH_DEVICE         Device
    );

// Interrupt
INT_CAUSE_READ_REGISTER
EthGetInterruptReason(
    IN      PETH_DEVICE         Device
    );

INT_MASK_SET_REGISTER
EthGetInterruptMaskRegister(
    IN      PETH_DEVICE         Device
    );

void
EthSetInterruptMaskSetRegister(
    IN      PETH_DEVICE             Device,
    IN      INT_MASK_SET_REGISTER   Mask
    );

void
EthSetInterruptMaskClearRegister(
    IN      PETH_DEVICE                 Device,
    IN      INT_MASK_CLEAR_REGISTER     Mask
    );

// Receive
DWORD
EthGetRxControlRegister(
    IN      PETH_DEVICE                 Device
    );

void
EthSetRxControlRegister(
    IN      PETH_DEVICE                 Device,
    IN      RECEIVE_CONTROL_REGISTER    ControlRegister
    );

void
EthSetRxFilterControlRegister(
    IN      PETH_DEVICE                         Device,
    IN      RECEIVE_FILTER_CONTROL_REGISTER     FilterRegister
    );

void
EthSetRxRingBufferAddress(
    IN      PETH_DEVICE         Device,
    IN      PHYSICAL_ADDRESS    Address
    );

void
EthSetRxRingBufferSize(
    IN      PETH_DEVICE         Device,
    IN      DWORD               Size
    );

WORD
EthGetRxHead(
    IN      PETH_DEVICE         Device
    );

void
EthSetRxHead(
    IN      PETH_DEVICE         Device,
    IN      WORD                Index
    );

WORD
EthGetRxTail(
    IN      PETH_DEVICE         Device
    );

void
EthSetRxTail(
    IN      PETH_DEVICE         Device,
    IN      WORD                Index
    );

WORD
EthGetRxInterruptRelativeDelay(
    IN      PETH_DEVICE         Device
    );

void
EthSetRxInterruptRelativeDelay(
    IN      PETH_DEVICE         Device,
    IN      WORD                Microseconds
    );

WORD
EthGetRxInterruptAbsoluteDelay(
    IN      PETH_DEVICE         Device
    );

void
EthSetRxInterruptAbsoluteDelay(
    IN      PETH_DEVICE         Device,
    IN      WORD                Microseconds
    );


// Transmit
DWORD
EthGetTxControlRegister(
    IN      PETH_DEVICE                 Device
    );

void
EthSetTxControlRegister(
    IN      PETH_DEVICE                 Device,
    IN      TRANSMIT_CONTROL_REGISTER   ControlRegister
    );

void
EthSetTxRingBufferAddress(
    IN      PETH_DEVICE         Device,
    IN      PHYSICAL_ADDRESS    Address
    );

void
EthSetTxRingBufferSize(
    IN      PETH_DEVICE         Device,
    IN      DWORD               Size
    );

WORD
EthGetTxHead(
    IN      PETH_DEVICE         Device
    );

void
EthSetTxHead(
    IN      PETH_DEVICE         Device,
    IN      WORD                Index
    );

WORD
EthGetTxTail(
    IN      PETH_DEVICE         Device
    );

void
EthSetTxTail(
    IN      PETH_DEVICE         Device,
    IN      WORD                Index
    );

WORD
EthGetTxInterruptRelativeDelay(
    IN      PETH_DEVICE         Device
    );

void
EthSetTxInterruptRelativeDelay(
    IN      PETH_DEVICE         Device,
    IN      WORD                Microseconds
    );

WORD
EthGetTxInterruptAbsoluteDelay(
    IN      PETH_DEVICE         Device
    );

void
EthSetTxInterruptAbsoluteDelay(
    IN      PETH_DEVICE         Device,
    IN      WORD                Microseconds
    );