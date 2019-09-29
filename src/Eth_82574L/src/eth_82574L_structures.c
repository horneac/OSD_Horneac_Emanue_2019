#include "eth_82574L_base.h"

__forceinline
WORD
_EthTransformMicrosecondsTo1Dot024(
    IN      WORD                Microseconds
    )
{
    DWORD result;

    result = ( (DWORD) Microseconds * 1000 + 1023 ) / 1024;

    ASSERT( result <= MAX_WORD );

    return (WORD) result;
}

__forceinline
WORD
_EthTransform1Dot024ToMicroseconds(
    IN      WORD                DotResult
    )
{
    DWORD result;

    /// Unfortunately, I can't come up with a formula which will give
    /// the correct result for 0 :( => special case
    if (0 == DotResult)
    {
        return 0;
    }

    result = ((DWORD) DotResult * 1024 - 24 ) / 1000;

    ASSERT( result <= MAX_WORD );

    return (WORD) result;
}

DEVICE_CONTROL_REGISTER
EthGetDeviceControlRegister(
    IN      PETH_DEVICE         Device
    )
{
    DEVICE_CONTROL_REGISTER result;

    ASSERT( NULL != Device );

    result.Raw = Device->InternalRegisters->DeviceControlRegister[0];

    return result;
}

void
EthSetDeviceControlRegister(
    IN      PETH_DEVICE                 Device,
    IN      DEVICE_CONTROL_REGISTER     ControlRegister
    )
{
    ASSERT( NULL != Device );

    Device->InternalRegisters->DeviceControlRegister[0] = ControlRegister.Raw;
}

DEVICE_STATUS_REGISTER
EthGetDeviceStatusRegister(
    IN      PETH_DEVICE         Device
    )
{
    DEVICE_STATUS_REGISTER result;

    ASSERT( NULL != Device );

    result.Raw = Device->InternalRegisters->DeviceStatusRegister;

    return result;
}

INT_CAUSE_READ_REGISTER
EthGetInterruptReason(
    IN      PETH_DEVICE         Device
    )
{
    INT_CAUSE_READ_REGISTER result;

    ASSERT(NULL != Device);

    result.Raw = Device->InternalRegisters->InterruptCauseReadRegister;

    return result;
}

INT_MASK_SET_REGISTER
EthGetInterruptMaskRegister(
    IN      PETH_DEVICE         Device
    )
{
    INT_MASK_SET_REGISTER result;

    ASSERT( NULL != Device );

    result.Raw = Device->InternalRegisters->InterruptMaskSetRegister;

    return result;
}

void
EthSetInterruptMaskSetRegister(
    IN      PETH_DEVICE             Device,
    IN      INT_MASK_SET_REGISTER   Mask
    )
{
    ASSERT( NULL != Device );

    Device->InternalRegisters->InterruptMaskSetRegister = Mask.Raw;
}

void
EthSetInterruptMaskClearRegister(
    IN      PETH_DEVICE                 Device,
    IN      INT_MASK_CLEAR_REGISTER     Mask
    )
{
    ASSERT(NULL != Device);

    Device->InternalRegisters->InterruptMaskClearRegister = Mask.Raw;
}

DWORD
EthGetRxControlRegister(
    IN      PETH_DEVICE                 Device
    )
{
    ASSERT(NULL != Device);

    return Device->InternalRegisters->ReceiveControlRegister;
}

void
EthSetRxControlRegister(
    IN      PETH_DEVICE                 Device,
    IN      RECEIVE_CONTROL_REGISTER    ControlRegister
    )
{
    ASSERT(NULL != Device);

    Device->InternalRegisters->ReceiveControlRegister = ControlRegister.Raw;
}

void
EthSetRxFilterControlRegister(
    IN      PETH_DEVICE                         Device,
    IN      RECEIVE_FILTER_CONTROL_REGISTER     FilterRegister
    )
{
    ASSERT(NULL != Device);

    Device->InternalRegisters->ReceiveFilterControlRegister = FilterRegister.Raw;
}

void
EthSetRxRingBufferAddress(
    IN      PETH_DEVICE         Device,
    IN      PHYSICAL_ADDRESS    Address
    )
{
    DWORD ringBufferLow;
    DWORD ringBufferHigh;
    RD_BASE_ADDRESS_LOW rdBal;
    RD_BASE_ADDRESS_HIGH rdBah;

    ASSERT(NULL != Device);
    ASSERT(NULL != Address);

    rdBal.Raw = 0;
    rdBah.Raw = 0;
    ringBufferHigh = QWORD_HIGH(Address);
    ringBufferLow = QWORD_LOW(Address);

    // Configure ring buffer address
    ASSERT(IsAddressAligned(ringBufferLow, ETH_DESCRIPTOR_BUFFER_ALIGNMENT));
    rdBal.Raw = ringBufferLow;
    rdBah.BaseAddressHigh = ringBufferHigh;

    Device->InternalRegisters->ReceiveDescriptorAddressLow = rdBal.Raw;
    Device->InternalRegisters->ReceiveDescriptorAddressHigh = rdBah.Raw;
}

void
EthSetRxRingBufferSize(
    IN      PETH_DEVICE         Device,
    IN      DWORD               Size
    )
{
    RD_LENGTH rdLen;

    ASSERT( NULL != Device );

    rdLen.Raw = 0;

    // Set descriptor length
    rdLen.Length = Size;

    Device->InternalRegisters->ReceiveDescriptorLength = rdLen.Raw;
}

WORD
EthGetRxHead(
    IN      PETH_DEVICE         Device
    )
{
    RD_HEAD rdHead;

    ASSERT(NULL != Device);

    rdHead.Raw = Device->InternalRegisters->ReceiveDescriptorHead;

    return rdHead.Head;
}

void
EthSetRxHead(
    IN      PETH_DEVICE         Device,
    IN      WORD                Index
    )
{
    RD_HEAD rdHead;

    ASSERT(NULL != Device);

    rdHead.Raw = 0;
    rdHead.Head = Index;

    Device->InternalRegisters->ReceiveDescriptorHead = rdHead.Raw;
}

WORD
EthGetRxTail(
    IN      PETH_DEVICE         Device
    )
{
    RD_TAIL rdTail;

    ASSERT(NULL != Device);

    rdTail.Raw = Device->InternalRegisters->ReceiveDescriptorTail;

    return rdTail.Tail;
}

void
EthSetRxTail(
    IN      PETH_DEVICE         Device,
    IN      WORD                Index
    )
{
    RD_TAIL rdTail;

    ASSERT(NULL != Device);

    rdTail.Raw = 0;
    rdTail.Tail = Index;

    Device->InternalRegisters->ReceiveDescriptorTail = rdTail.Raw;
}

WORD
EthGetRxInterruptRelativeDelay(
    IN      PETH_DEVICE         Device
    )
{
    RECEIVE_INTERRUPT_RELATIVE_DELAY_TIMER timer;

    ASSERT(NULL != Device);

    timer.Raw = Device->InternalRegisters->ReceiveInterruptRelativeDelayTimer;

    return _EthTransform1Dot024ToMicroseconds( timer.Delay );
}

void
EthSetRxInterruptRelativeDelay(
    IN      PETH_DEVICE         Device,
    IN      WORD                Microseconds
    )
{
    RECEIVE_INTERRUPT_RELATIVE_DELAY_TIMER timer;

    ASSERT( NULL != Device );

    timer.Raw = 0;
    timer.Delay = _EthTransformMicrosecondsTo1Dot024(Microseconds);

    Device->InternalRegisters->ReceiveInterruptRelativeDelayTimer = timer.Raw;
}

WORD
EthGetRxInterruptAbsoluteDelay(
    IN      PETH_DEVICE         Device
    )
{
    RECEIVE_INTERRUPT_ABSOLUTE_DELAY_TIMER timer;

    ASSERT(NULL != Device);

    timer.Raw = Device->InternalRegisters->ReceiveInterruptAbsoluteDelayTimer;

    return _EthTransform1Dot024ToMicroseconds( timer.Delay );
}

void
EthSetRxInterruptAbsoluteDelay(
    IN      PETH_DEVICE         Device,
    IN      WORD                Microseconds
    )
{
    RECEIVE_INTERRUPT_ABSOLUTE_DELAY_TIMER timer;

    ASSERT(NULL != Device);

    timer.Raw = 0;
    timer.Delay = _EthTransformMicrosecondsTo1Dot024(Microseconds);

    Device->InternalRegisters->ReceiveInterruptAbsoluteDelayTimer = timer.Raw;
}

DWORD
EthGetTxControlRegister(
    IN      PETH_DEVICE                 Device
    )
{
    ASSERT(NULL != Device);

    return Device->InternalRegisters->TransmitControlRegister;
}

void
EthSetTxControlRegister(
    IN      PETH_DEVICE                 Device,
    IN      TRANSMIT_CONTROL_REGISTER   ControlRegister
    )
{
    ASSERT(NULL != Device);

    Device->InternalRegisters->TransmitControlRegister = ControlRegister.Raw;
}

void
EthSetTxRingBufferAddress(
    IN      PETH_DEVICE         Device,
    IN      PHYSICAL_ADDRESS    Address
    )
{
    DWORD ringBufferLow;
    DWORD ringBufferHigh;
    TD_BASE_ADDRESS_LOW tdBal;
    TD_BASE_ADDRESS_HIGH tdBah;

    ASSERT(NULL != Device);
    ASSERT(NULL != Address);

    tdBal.Raw = 0;
    tdBah.Raw = 0;
    ringBufferHigh = QWORD_HIGH(Address);
    ringBufferLow = QWORD_LOW(Address);

    // Configure ring buffer address
    ASSERT(IsAddressAligned(ringBufferLow, ETH_DESCRIPTOR_BUFFER_ALIGNMENT));
    tdBal.Raw = ringBufferLow;
    tdBah.BaseAddressHigh = ringBufferHigh;

    Device->InternalRegisters->TransmitDescriptorAddressLow = tdBal.Raw;
    Device->InternalRegisters->TransmitDescriptorAddressHigh = tdBah.Raw;
}

void
EthSetTxRingBufferSize(
    IN      PETH_DEVICE         Device,
    IN      DWORD               Size
    )
{
    TD_LENGTH tdLen;

    ASSERT(NULL != Device);

    tdLen.Raw = 0;

    // Set descriptor length
    tdLen.Length = Size;

    Device->InternalRegisters->TransmitDescriptorLength = tdLen.Raw;
}

WORD
EthGetTxHead(
    IN      PETH_DEVICE         Device
    )
{
    TD_HEAD tdHead;

    ASSERT(NULL != Device);

    tdHead.Raw = Device->InternalRegisters->TransmitDescriptorHead;

    return tdHead.Head;
}

void
EthSetTxHead(
    IN      PETH_DEVICE         Device,
    IN      WORD                Index
    )
{
    TD_HEAD tdHead;

    ASSERT(NULL != Device);

    tdHead.Raw = 0;
    tdHead.Head = Index;

    Device->InternalRegisters->TransmitDescriptorHead = tdHead.Raw;
}

WORD
EthGetTxTail(
    IN      PETH_DEVICE         Device
    )
{
    TD_TAIL tdTail;

    ASSERT(NULL != Device);

    tdTail.Raw = Device->InternalRegisters->TransmitDescriptorTail;

    return tdTail.Tail;
}

void
EthSetTxTail(
    IN      PETH_DEVICE         Device,
    IN      WORD                Index
    )
{
    TD_TAIL tdTail;

    ASSERT(NULL != Device);

    tdTail.Raw = 0;
    tdTail.Tail = Index;

    Device->InternalRegisters->TransmitDescriptorTail = tdTail.Raw;
}

WORD
EthGetTxInterruptRelativeDelay(
    IN      PETH_DEVICE         Device
    )
{
    TRANSMIT_INTERRUPT_RELATIVE_DELAY_TIMER timer;

    ASSERT(NULL != Device);

    timer.Raw = Device->InternalRegisters->TransmitInterruptRelativeDelayTimer;

    return _EthTransform1Dot024ToMicroseconds(timer.Delay);
}

void
EthSetTxInterruptRelativeDelay(
    IN      PETH_DEVICE         Device,
    IN      WORD                Microseconds
    )
{
    TRANSMIT_INTERRUPT_RELATIVE_DELAY_TIMER timer;

    ASSERT(NULL != Device);

    timer.Raw = 0;
    timer.Delay = _EthTransformMicrosecondsTo1Dot024(Microseconds);

    Device->InternalRegisters->TransmitInterruptRelativeDelayTimer = timer.Raw;
}

WORD
EthGetTxInterruptAbsoluteDelay(
    IN      PETH_DEVICE         Device
    )
{
    TRANSMIT_INTERRUPT_ABSOLUTE_DELAY_TIMER timer;

    ASSERT(NULL != Device);

    timer.Raw = Device->InternalRegisters->TransmitInterruptAbsoluteDelayTimer;

    return _EthTransform1Dot024ToMicroseconds(timer.Delay);
}

void
EthSetTxInterruptAbsoluteDelay(
    IN      PETH_DEVICE         Device,
    IN      WORD                Microseconds
    )
{
    TRANSMIT_INTERRUPT_ABSOLUTE_DELAY_TIMER timer;

    ASSERT(NULL != Device);

    timer.Raw = 0;
    timer.Delay = _EthTransformMicrosecondsTo1Dot024(Microseconds);

    Device->InternalRegisters->TransmitInterruptAbsoluteDelayTimer = timer.Raw;
}