#include "hal_base.h"
#include "serial.h"

// Serial Port Register Offsets
#define     DATA_REG_OFFSET                 0x0     // Stores Data for both I&O
#define     INT_REG_OFFSET                  0x1     // Interrupt Register
#define     FIFO_REG_OFFSET                 0x2     // FIFO Register
#define     LINE_CREG_OFFSET                0x3     // Line Control Register
#define     MODEM_CREG_OFFSET               0x4     // Modem Control Register
#define     LSR_REG_OFFSET                  0x5     // Line Status Register
#define     MODEM_SREG_OFFSET               0x6     // Modem Status Register
#define     SCRATCH_REG_OFFSET              0x7     // Scratch Register
#define     RESERVED_REG_OFFSET             0x8

// these are valid only if DLAB is set
#define     LSB_DIV_OFFSET_VALUE            0x0     // LSB of BaudRate divisor
#define     MSB_DIV_OFFSET_VALUE            0x1     // MSB of BaudRate divisor

// LSR masks
#define     LSR_THR_READY                   (1<<5)  // data is ready to be sent
#define     LSR_OVERRUN_ERROR               (1<<1)

#define     DLAB_MASK                       (1<<7)

// FIFO masks
#define     FIFO_ENABLE                     (1<<0)
#define     FIFO_RECEIVER_RESET             (1<<1)
#define     FIFO_TRANSMIT_RESET             (1<<2)
#define     FIFO_RECEIVE_TRIG_0             (1<<6)
#define     FIFO_RECEIVE_TRIG_1             (1<<7)

// we will send 7 bits of data
#define     DATA_BITS                       2

// 1 stop bit
#define     STOP_BIT                        (0<<2)

// no parity bit
#define     PARITY_BIT                      (0<<3)


//******************************************************************************
// Function:    SerialOut
// Description: Outputs a byte of data through the serial port.
// Returns:       void
// Parameter:     IN BYTE Data - byte to output.
//******************************************************************************
__forceinline
static
void 
_SerialOut(
    IN WORD Port,
    IN BYTE Data 
    )
{
    // we wait for the data register to be ready to receive new data
    while( ( __inbyte( Port + LSR_REG_OFFSET ) & LSR_THR_READY ) == 0 );

    // we output the byte
    __outbyte( Port, Data );
}

void
SerialInitialize( 
    IN  WORD    Port
    )
{
    ASSERT( 0 != Port );

    // Disable all interrupts
    __outbyte( Port + INT_REG_OFFSET, 0x00);    
    __outbyte( Port + LINE_CREG_OFFSET, DLAB_MASK );

    // set baud rate divisor to 1 => BaudRate = 115200
    __outbyte( Port + LSB_DIV_OFFSET_VALUE, 1 );
    __outbyte( Port + MSB_DIV_OFFSET_VALUE, 0 );

    // 7 data bits, no parity, one stop bit
    __outbyte( Port + LINE_CREG_OFFSET, PARITY_BIT | STOP_BIT | DATA_BITS );

    // we enable FIFO
    // we would theoretically be interrupted every 14 characters received but because we have interrupts disabled
    // this will never happen
    __outbyte( Port + FIFO_REG_OFFSET, FIFO_RECEIVE_TRIG_1 | FIFO_RECEIVE_TRIG_0 | FIFO_TRANSMIT_RESET | FIFO_RECEIVER_RESET | FIFO_ENABLE );
}

void 
SerialWriteBuffer(
    IN   WORD   Port,
    IN_Z char*  Buffer 
    )
{
    DWORD i;

    ASSERT( 0 != Port );
    ASSERT( NULL != Buffer );

    i = 0;

    while( '\0' != Buffer[i] )
    {
        _SerialOut( Port, Buffer[i] );
        ++i;
    }
}

void
SerialWriteNBuffer(
    IN                              WORD  Port,
    IN_READS_BYTES(BufferLength)    BYTE* Buffer,
    IN                              DWORD BufferLength
    )
{
    DWORD i;

    ASSERT(0 != Port);
    ASSERT(NULL != Buffer);

    for( i = 0; i < BufferLength; ++i )
    {
        _SerialOut( Port, Buffer[i] );
    }
}