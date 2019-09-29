#pragma once

#define ETH_INTERNAL_REG_SIZE                   4

#pragma pack(push,1)

#pragma warning(push)

// warning C4201: nonstandard extension used: nameless struct/union
#pragma warning(disable:4201)

// warning C4214: nonstandard extension used: bit field types other than int
#pragma warning(disable:4214)

//////////////////////////////////////////////////////////////////////////////////////
//////                       General Register Descriptors                      ///////
//////////////////////////////////////////////////////////////////////////////////////

// 0x04 - RW
typedef union _DEVICE_CONTROL_REGISTER
{
    struct
    {
        DWORD               FullDuplex                      : 1;

        DWORD               __Reserved0                     : 1;

        DWORD               GIOMasterDisable                : 1;

        DWORD               __Reserved1                     : 2;

        DWORD               AutoSpeedDetectionEnable        : 1;

        DWORD               SetLinkUp                       : 1;

        DWORD               __Reserved2                     : 1;

        DWORD               Speed                           : 2;

        DWORD               __Reserved3                     : 1;

        DWORD               ForceSpeed                      : 1;

        DWORD               ForceDuplex                     : 1;

        DWORD               __Reserved4                     : 7;

        DWORD               __D3ColdWakeupAdEnable          : 1;

        DWORD               __Reserved5                     : 5;

        DWORD               DeviceReset                     : 1;

        DWORD               RxFlowControlEnable             : 1;

        DWORD               TxFlowControlEnable             : 1;

        DWORD               __Reserved6                     : 1;

        DWORD               VlanModeEnable                  : 1;

        DWORD               PhyReset                        : 1;
    };
    DWORD                   Raw;
} DEVICE_CONTROL_REGISTER, *PDEVICE_CONTROL_REGISTER;
STATIC_ASSERT(sizeof(DEVICE_CONTROL_REGISTER) == ETH_INTERNAL_REG_SIZE);

// 0x08 - R
typedef union _DEVICE_STATUS_REGISTER
{
    struct
    {
        DWORD               FullDuplex                      : 1;

        DWORD               LinkUp                          : 1;

        DWORD               __Reserved0                     : 2;

        DWORD               TxPaused                        : 1;

        DWORD               __Reserved1                     : 1;

        DWORD               Speed                           : 2;

        DWORD               AutoSpeedDetectionValue         : 2;

        DWORD               PhyResetAsserted                : 1;

        DWORD               __Reserved2                     : 8;

        DWORD               GIOMasterEnableStatus           : 1;

        DWORD               __Reserved3                     : 12;
    };
    DWORD                   Raw;
} DEVICE_STATUS_REGISTER, *PDEVICE_STATUS_REGISTER;
STATIC_ASSERT(sizeof(DEVICE_STATUS_REGISTER) == ETH_INTERNAL_REG_SIZE);

//////////////////////////////////////////////////////////////////////////////////////
//////                      Interrupt Register Descriptors                     ///////
//////////////////////////////////////////////////////////////////////////////////////

// 0xC0 - RC/WC
typedef union _INT_CAUSE_READ_REGISTER
{
    struct
    {
        // Set when hardware processes a descriptor with RS set. If using
        // delayed interrupts(IDE set), the interrupt is delayed until after one of
        // the delayed - timers(TIDV or TADV) expires.
        DWORD               TdWrittenBack                   : 1;

        // Set when the last descriptor block for a transmit queue has been
        // used.When configured to use more than one transmit queue this
        // interrupt indication is issued if one of the queues is
        DWORD               TxQueueEmpty                    : 1;

        // This bit is set whenever the link status changes(either from up to
        // down, or from down to up).This bit is affected by the link indication
        // from the PHY.
        DWORD               LinkStatusChange                : 1;

        DWORD               __Reserved0                     : 1;

        // This bit indicates that the number of receive descriptors has reached
        // the minimum threshold as set in RCTL.RDMTS.This indicates to the
        // software to load more receive descriptors.
        DWORD               RdMinimumThresholdHit           : 1;

        DWORD               __Reserved1                     : 1;

        // Set on receive data FIFO overrun.Could be caused either because
        // there are no available buffers or because PCIe receive bandwidth is
        // inadequate.
        DWORD               ReceiverOverrun                 : 1;

        DWORD               ReceiverTimerInterrupt          : 1;

        DWORD               __Reserved2                     : 1;

        DWORD               __Reserved3                     : 22;

        // This bit is set when the LAN port has a pending interrupt.If the
        // interrupt is enabled in the PCI configuration space, an interrupt is
        // asserted.
        DWORD               IntAsserted                     : 1;
    };
    DWORD                   Raw;
} INT_CAUSE_READ_REGISTER, *PINT_CAUSE_READ_REGISTER;
STATIC_ASSERT(sizeof(INT_CAUSE_READ_REGISTER) == ETH_INTERNAL_REG_SIZE);

// 0xD0 - RW
typedef union _INT_MASK_SET_REGISTER
{
    struct
    {
        // Sets the mask for transmit descriptor written back.
        DWORD               TdWrittenBack                   : 1;

        // Sets the mask for transmit queue empty.
        DWORD               TxQueueEmpty                    : 1;

        // Sets the mask for link status change.
        DWORD               LinkStatusChange                : 1;

        DWORD               __Reserved0                     : 1;

        // Sets the mask for receive descriptor minimum threshold hit.
        DWORD               RdMinimumThresholdHit           : 1;

        DWORD               __Reserved1                     : 1;

        // Sets mask for receiver overrun. Set on receive data FIFO overrun.
        DWORD               ReceiverOverrun                 : 1;

        // Sets mask for receiver timer interrupt.
        DWORD               ReceiverTimerInterrupt          : 1;

        DWORD               __Reserved2                     : 1;

        // Sets mask for MDIO access complete interrupt.
        DWORD               MdioAccessComplete              : 1;

        DWORD               __Reserved3                     : 5;

        // Sets the mask for transmit descriptor low threshold hit.
        DWORD               TdLowThresholdHit               : 1;

        // Sets the mask for small receive packet detection.
        DWORD               SmallReceivePacketDetection     : 1;

        // Sets the mask for receive ACK frame detection.
        DWORD               AckFrameDetection               : 1;

        // Sets a manageability event.
        DWORD               ManageabilityEvent              : 1;

        DWORD               __Reserved4                     : 1;

        // Sets the mask for receive queue 0 interrupt.
        DWORD               RxQueue0                        : 1;

        // Sets the mask for receive queue 1 interrupt.
        DWORD               RxQueue1                        : 1;

        // Sets the mask for transmit queue 0 interrupt.
        DWORD               TxQueue0                        : 1;

        // Sets the mask for transmit queue 1 interrupt.
        DWORD               TxQueue1                        : 1;

        // Sets the mask for other interrupt.
        DWORD               OtherInterrupt                  : 1;

        DWORD               __Reserved5                     : 7;
    };
    DWORD                   Raw;
} INT_MASK_SET_REGISTER, *PINT_MASK_SET_REGISTER;
STATIC_ASSERT(sizeof(INT_MASK_SET_REGISTER) == ETH_INTERNAL_REG_SIZE);

// 0xD8 - W
typedef union _INT_MASK_CLEAR_REGISTER
{
    struct
    {
        // Clears the mask for transmit descriptor written back.
        DWORD               TdWrittenBack                   : 1;

        // Clears the mask for transmit queue empty.
        DWORD               TxQueueEmpty                    : 1;

        // Clears the mask for link status change.
        DWORD               LinkStatusChange                : 1;

        DWORD               __Reserved0                     : 1;

        // Clears the mask for receive descriptor minimum threshold hit.
        DWORD               RdMinimumThresholdHit           : 1;

        DWORD               __Reserved1                     : 1;

        // Clears mask for receiver overrun. Set on receive data FIFO overrun.
        DWORD               ReceiverOverrun                 : 1;

        // Clears mask for receiver timer interrupt.
        DWORD               ReceiverTimerInterrupt          : 1;

        DWORD               __Reserved2                     : 1;

        // Clears mask for MDIO access complete interrupt.
        DWORD               MdioAccessComplete              : 1;

        DWORD               __Reserved3                     : 5;

        // Clears the mask for transmit descriptor low threshold hit.
        DWORD               TdLowThresholdHit               : 1;

        // Clears the mask for small receive packet detection.
        DWORD               SmallReceivePacketDetection     : 1;

        // Clears the mask for receive ACK frame detection.
        DWORD               AckFrameDetection               : 1;

        // Clears a manageability event.
        DWORD               ManageabilityEvent              : 1;

        DWORD               __Reserved4                     : 1;

        // Clears the mask for receive queue 0 interrupt.
        DWORD               RxQueue0                        : 1;

        // Clears the mask for receive queue 1 interrupt.
        DWORD               RxQueue1                        : 1;

        // Clears the mask for transmit queue 0 interrupt.
        DWORD               TxQueue0                        : 1;

        // Clears the mask for transmit queue 1 interrupt.
        DWORD               TxQueue1                        : 1;

        // Clears the mask for other interrupt.
        DWORD               OtherInterrupt                  : 1;

        DWORD               __Reserved5                     : 7;
    };
    DWORD                   Raw;
} INT_MASK_CLEAR_REGISTER, *PINT_MASK_CLEAR_REGISTER;

//////////////////////////////////////////////////////////////////////////////////////
//////                      Receive Register Descriptors                       ///////
//////////////////////////////////////////////////////////////////////////////////////

#define RDMTS_HALF                  0b00
#define RDMTS_QUARTER               0b01
#define RDMTS_HALF_QUARTER          0b10

// 0x100 - RW
typedef union _RECEIVE_CONTROL_REGISTER
{
    struct
    {
        DWORD               __Reserved0                     : 1;

        // The receiver is enabled when this bit is set to 1b. Writing this bit to
        // 0b, stops reception after receipt of any in progress packet.All
        // subsequent packets are then immediately dropped until this bit is set to 1b.
        DWORD               Enable                          : 1;

        DWORD               StoreBadPackets                 : 1;

        DWORD               UnicastPromiscuousEnable        : 1;

        DWORD               MulticastPromiscuousEnable      : 1;

        DWORD               LongPacketEnable                : 1;

        // Should always be set to 00b
        DWORD               LoopbackMode                    : 2;

        // The corresponding interrupt is set whenever the fractional number of
        // free descriptors becomes equal to RDMTS.
        //     00b 1 / 2
        //     01b 1 / 4
        //     10b 1 / 8
        //     11b RESERVED
        DWORD               ReceiveDescriptorMinThSize      : 2;

        // 00b = Legacy descriptor type.
        // 01b = Packet split descriptor type.
        // 10b, 11b = Reserved.
        DWORD               DescriptorType                  : 2;

        DWORD               MulticastOffset                 : 2;

        DWORD               __Reserved1                     : 1;

        DWORD               BroadcastAcceptMode             : 1;

        //  If RCTL.BSEX = 0b:
        //     00b = 2048 bytes.
        //     01b = 1024 bytes.
        //     10b = 512 bytes.
        //     11b = 256 bytes.
        //  If RCTL.BSEX = 1b :
        //     00b = reserved; software should not set to this value.
        //     01b = 16384 bytes.
        //     10b = 8192 bytes.
        //     11b = 4096 bytes.
        //  BSIZE is only used when DTYP = 00b.When DTYP = 01b, the buffer
        //  sizes for the descriptor are controlled by fields in the PSRCTL register.
        //  BSIZE is not relevant when FLXBUF is different from 0x0, in that case,
        //  FLXBUF determines the buffer size.
        DWORD               ReceiveBufferSize               : 2;

        DWORD               VlanFilterEnable                : 1;

        DWORD               CanonicalFormIndicatorEnable    : 1;

        DWORD               CanonicalFormIndiicatorValue    : 1;

        DWORD               __Reserved2                     : 1;

        DWORD               DiscardPauseFrames              : 1;

        DWORD               PassMacControlFrames            : 1;

        DWORD               __Reserved3                     : 1;

        // Modifies the buffer size indication(BSIZE).When set to 1b, the
        // original BSIZE values are multiplied by 16.
        DWORD               BufferSizeExtension             : 1;

        DWORD               StripEthCRC                     : 1;

        // Determines a flexible buffer size.When this field is 0x0000, the buffer
        // size is determined by BSIZE.If this field is different from 0x0000, the
        // receive buffer size is the number represented in KB.For example,
        // 0x0001 = 1 KB(1024 bytes).
        DWORD               FlexibleBufferSize              : 4;

        DWORD               __Reserved4                     : 1;
    };
    DWORD                   Raw;
} RECEIVE_CONTROL_REGISTER, *PRECEIVE_CONTROL_REGISTER;
STATIC_ASSERT(sizeof(RECEIVE_CONTROL_REGISTER) == ETH_INTERNAL_REG_SIZE);

// 0x2800 - RW
typedef union _RD_BASE_ADDRESS_LOW
{
    struct
    {
        DWORD                   __Ignored0                      :  4;

        DWORD                   BaseAddressLow                  : 28;
    };
    DWORD                       Raw;
} RD_BASE_ADDRESS_LOW, *PRD_BASE_ADDRESS_LOW;
STATIC_ASSERT(sizeof(RD_BASE_ADDRESS_LOW) == ETH_INTERNAL_REG_SIZE);

// 0x2804 - RW
typedef union _RD_BASE_ADDRESS_HIGH
{
    DWORD                   BaseAddressHigh;
    DWORD                   Raw;
} RD_BASE_ADDRESS_HIGH, *PRD_BASE_ADDRESS_HIGH;
STATIC_ASSERT(sizeof(RD_BASE_ADDRESS_HIGH) == ETH_INTERNAL_REG_SIZE);

// 0x2808 - RW
typedef union _RD_LENGTH
{
    struct
    {
        // Number of bytes allocated for descriptors in the circular descriptor
        // buffer.It must be 128 - byte aligned.
        DWORD                   Length                          : 20;

        DWORD                   __Reserved0                     : 12;
    };
    DWORD                       Raw;
} RD_LENGTH, *PRD_LENGTH;
STATIC_ASSERT(sizeof(RD_LENGTH) == ETH_INTERNAL_REG_SIZE);

// 0x2810 - RW
typedef union _RD_HEAD
{
    struct
    {
        // This register contains the head pointer for the receive descriptor buffer.The register
        // points to a 16 - byte datum.Hardware controls the pointer.The only time that software
        // should write to this register is after a reset(hardware reset or CTRL.RST) and before
        // enabling the receive function(RCTL.EN).
        /// ATTENTION: If software were to write to this register while the receive function was
        /// enabled, the on - chip descriptor buffers might be invalidated and the hardware
        /// could be become unstable.
        WORD                    Head;

        WORD                    __Reserved0;
    };
    DWORD                       Raw;
} RD_HEAD, *PRD_HEAD;
STATIC_ASSERT(sizeof(RD_HEAD) == ETH_INTERNAL_REG_SIZE);

// 0x2818 - RW
typedef union _RD_TAIL
{
    struct
    {
        // This register contains the tail pointers for the receive descriptor buffer.The register
        // points to a 16 - byte datum.Software writes the tail register to add receive descriptors
        // to the hardware free list for the ring.
        WORD                   Tail;

        WORD                   __Reserved0;
    };
    DWORD                       Raw;
} RD_TAIL, *PRD_TAIL;
STATIC_ASSERT(sizeof(RD_TAIL) == ETH_INTERNAL_REG_SIZE);

// 0x2820 - RW
typedef union _RECEIVE_INTERRUPT_RELATIVE_DELAY_TIMER
{
    struct
    {
        // Receive packet delay timer measured in increments of 1.024 in us
        WORD                    Delay;

        WORD                    __Reserved                      : 15;

        // When set to 1b, flushes the partial descriptor block; ignored
        // otherwise.Reads 0b.
        WORD                    FlushPartialDescriptorBlock     :  1;
    };
    DWORD                       Raw;
} RECEIVE_INTERRUPT_RELATIVE_DELAY_TIMER, *PRECEIVE_INTERRUPT_RELATIVE_DELAY_TIMER;
STATIC_ASSERT(sizeof(RECEIVE_INTERRUPT_RELATIVE_DELAY_TIMER) == ETH_INTERNAL_REG_SIZE);

// 0x282C - RW
typedef union _RECEIVE_INTERRUPT_ABSOLUTE_DELAY_TIMER
{
    struct
    {
        // Receive absolute delay timer measured in increments of 1.024 in us (0=
        // disabled).
        WORD                    Delay;

        WORD                    __Reserved;
    };
    DWORD                       Raw;
} RECEIVE_INTERRUPT_ABSOLUTE_DELAY_TIMER, *PRECEIVE_INTERRUPT_ABSOLUTE_DELAY_TIMER;
STATIC_ASSERT(sizeof(RECEIVE_INTERRUPT_ABSOLUTE_DELAY_TIMER) == ETH_INTERNAL_REG_SIZE);

// 0x3820 - RW
typedef union _TRANSMIT_INTERRUPT_RELATIVE_DELAY_TIMER
{
    struct
    {
        // Counts in units of 1.024 microseconds. A value of 0 is not allowed.
        WORD                    Delay;

        WORD                    __Reserved                      : 15;

        // Flush Partial Descriptor Block
        WORD                    FlushPartialDescriptorBlock     :  1;
    };
    DWORD                       Raw;
} TRANSMIT_INTERRUPT_RELATIVE_DELAY_TIMER, *PTRANSMIT_INTERRUPT_RELATIVE_DELAY_TIMER;
STATIC_ASSERT(sizeof(TRANSMIT_INTERRUPT_RELATIVE_DELAY_TIMER) == ETH_INTERNAL_REG_SIZE);

// 0x382C - RW
typedef union _TRANSMIT_INTERRUPT_ABSOLUTE_DELAY_TIMER
{
    struct
    {
        // Counts in units of 1.024 microseconds. (0b = disabled).
        WORD                    Delay;

        WORD                    __Reserved;
    };
    DWORD                       Raw;
} TRANSMIT_INTERRUPT_ABSOLUTE_DELAY_TIMER, *PTRANSMIT_INTERRUPT_ABSOLUTE_DELAY_TIMER;
STATIC_ASSERT(sizeof(TRANSMIT_INTERRUPT_ABSOLUTE_DELAY_TIMER) == ETH_INTERNAL_REG_SIZE);

// 0x5008 - RW
typedef union _RECEIVE_FILTER_CONTROL_REGISTER
{
    struct
    {
        // Disable the iSCSI filtering.
        DWORD                   IScsiDisable                    :  1;

        // This field indicates the Dword count of the iSCSI header, which is used
        // for packet split mechanism.
        DWORD                   IScsiDwordCount                 :  5;

        // Disable filtering of NFS write request headers.
        DWORD                   NfsWriteDisable                 :  1;

        // Disable filtering of NFS read reply headers.
        DWORD                   NfsReadDisable                  :  1;

        // 00b = NFS version 2.
        // 01b = NFS version 3.
        // 10b = NFS version 4.
        // 11b = Reserved for future use.
        DWORD                   NfsVersion                      :  2;

        // Disable IPv6 packet filtering.
        DWORD                   Ipv6Disable                     :  1;

        // Disable XSUM on IPv6 packets.
        DWORD                   Ipv6XSumDisable                 :  1;

        // When this bit is set, the 82574 does not accelerate interrupt on TCP
        // ACK packets
        DWORD                   TcpAckAccelerateDisable         :  1;

        // 1b = The 82574L recognizes ACK packets according to the ACK bit in
        // the TCP header + No –CP data
        // 0b = The 82574L recognizes ACK packets according to the ACK bit
        // only.
        // This bit is relevant only if the ACKDIS bit is not set.
        DWORD                   TcpAckDataDisable               :  1;

        // When this bit is set, the header of IP fragmented packets are not set.
        DWORD                   IpFragmentSplitDisable          :  1;

        // When the EXSTEN bit is set or when the packet split receive
        // descriptor is used, the 82574 writes the extended status to the Rx
        // descriptor.
        DWORD                   ExtendedStatusEnable            :  1;

        DWORD                   __Reserved1                     : 16;
    };
    DWORD                       Raw;
} RECEIVE_FILTER_CONTROL_REGISTER, *PRECEIVE_FILTER_CONTROL_REGISTER;
STATIC_ASSERT(sizeof(RECEIVE_FILTER_CONTROL_REGISTER) == ETH_INTERNAL_REG_SIZE);

//////////////////////////////////////////////////////////////////////////////////////
//////                      Transmit Register Descriptors                      ///////
//////////////////////////////////////////////////////////////////////////////////////

// 0x400 - RW
typedef union _TRANSMIT_CONTROL_REGISTER
{
    struct
    {
        DWORD               __Reserved0                     :  1;

        // The transmitter is enabled when this bit is set to 1b. Writing this bit to
        // 0b stops transmission after any in progress packets are sent.Data
        // remains in the transmit FIFO until the device is re - enabled.Software
        // should combine this with a reset if the packets in the FIFO need to be
        // flushed.
        DWORD               Enable                          :  1;

        DWORD               __Reserved1                     :  1;

        // Padding makes the packet 64 bytes.This is not the same as the
        // minimum collision distance.
        // If padding of short packet is allowed, the value in TX descriptor length
        // field should be not less than 17 bytes.
        DWORD               PadShortPackets                 :  1;

        // This determines the number of attempts at re - transmission prior to
        // giving up on the packet(not including the first transmission attempt).
        // While this can be varied, it should be set to a value of 15 in order to
        // comply with the IEEE specification requiring a total of 16 attempts.
        // The Ethernet back - off algorithm is implemented and clamps to the
        // maximum number of slot times after 10 retries.This field only has
        // meaning while in half - duplex operation.
        DWORD               CollisionThreshold              :  8;

        // Specifies the minimum number of byte times that must elapse for
        // proper CSMA / CD operation.Packets are padded with special symbols,
        // not valid data bytes.Hardware checks and pads to this value plus one
        // byte even in full - duplex operation.
        DWORD               CollisionDistance               : 10;

        // When set to 1b, the device schedules the transmission of an XOFF
        // (PAUSE) frame using the current value of the pause timer.This bit self
        // clears upon transmission of the XOFF frame.
        DWORD               SoftwareXoffTransmission        :  1;

        DWORD               __Reserved2                     :  1;

        DWORD               __Ignored0                      :  1;

        DWORD               UnderRunNoRetransmit            :  1;

        DWORD               TxDescriptorMinimumThreshold    :  2;

        // This bit defines the number of read requests the 82574 issues for
        // transmit data.When set to 0b, the 82574 submits only one request at
        // a time, When set to 1b, the 82574 might submit up to four concurrent
        // requests.The software device driver must not modify this register
        // when the Tx head register is not equal to the tail register.
        // This bit is loaded from the NVM word 0x24 / 0x14.
        DWORD               MultipleRequestSupport          :  1;

        // These bits define the threshold size for the intermediate buffer to
        // determine when to send the read command to the packet buffer.
        // Threshold is defined as follows :
        //  RRTHRESH = 00b threshold = 2 lines of 16 bytes
        //  RRTHRESH = 01b threshold = 4 lines of 16 bytes
        //  RRTHRESH = 10b threshold = 8 lines of 16 bytes
        //  RRTHRESH = 11b threshold = No threshold(transfer data after all of
        //                                          the request is in the RFIFO)
        DWORD               ReadRequestThreashold           :  2;

        DWORD               __Reserved3                     :  1;
    };
    DWORD                   Raw;
} TRANSMIT_CONTROL_REGISTER, *PTRANSMIT_CONTROL_REGISTER;
STATIC_ASSERT(sizeof(TRANSMIT_CONTROL_REGISTER) == ETH_INTERNAL_REG_SIZE);

// 0x3800 - RW
typedef union _TD_BASE_ADDRESS_LOW
{
    struct
    {
        DWORD                   __Ignored0                      :  4;

        DWORD                   BaseAddressLow                  : 28;
    };
    DWORD                       Raw;
} TD_BASE_ADDRESS_LOW, *PTD_BASE_ADDRESS_LOW;
STATIC_ASSERT(sizeof(TD_BASE_ADDRESS_LOW) == ETH_INTERNAL_REG_SIZE);

// 0x3804 - RW
typedef union _TD_BASE_ADDRESS_HIGH
{
    DWORD                   BaseAddressHigh;
    DWORD                   Raw;
} TD_BASE_ADDRESS_HIGH, *PTD_BASE_ADDRESS_HIGH;
STATIC_ASSERT(sizeof(TD_BASE_ADDRESS_HIGH) == ETH_INTERNAL_REG_SIZE);

// 0x3808 - RW
typedef union _TD_LENGTH
{
    struct
    {
        // Number of bytes allocated for descriptors in the circular descriptor
        // buffer.It must be 128 - byte aligned.
        DWORD                   Length                          : 20;

        DWORD                   __Reserved0                     : 12;
    };
    DWORD                       Raw;
} TD_LENGTH, *PTD_LENGTH;
STATIC_ASSERT(sizeof(TD_LENGTH) == ETH_INTERNAL_REG_SIZE);

// 0x3810 - RW
typedef union _TD_HEAD
{
    struct
    {
        // This register contains the head pointer for the transmit descriptor ring.It points to a
        // 16 - byte datum.Hardware controls this pointer.The only time that software should
        // write to this register is after a reset(hardware reset or CTRL.RST) and before enabling
        // the transmit function(TCTL.EN).
        WORD                    Head;

        WORD                    __Reserved0;
    };
    DWORD                       Raw;
} TD_HEAD, *PTD_HEAD;
STATIC_ASSERT(sizeof(TD_HEAD) == ETH_INTERNAL_REG_SIZE);

// 0x3818 - RW
typedef union _TD_TAIL
{
    struct
    {
        // This register contains the tail pointer for the transmit descriptor ring.It points to a 16 -
        // byte datum.Software writes the tail pointer to add more descriptors to the transmit
        // ready queue.Hardware attempts to transmit all packets referenced by descriptors
        // between head and tail.
        WORD                        Tail;

        WORD                        __Reserved0;
    };
    DWORD                           Raw;
} TD_TAIL, *PTD_TAIL;
STATIC_ASSERT(sizeof(TD_TAIL) == ETH_INTERNAL_REG_SIZE);

#pragma warning(pop)
#pragma pack(pop)
