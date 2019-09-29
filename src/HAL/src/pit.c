#include "hal_base.h"
#include "pit.h"

#define PIT_FREQUENCY_HZ                                    (1'193'182ULL)

#define PIT_PERIODIC_CHANNEL                                0
#define PIT_ONETIME_CHANNEL                                 2

#define PIT_NO_OF_CHANNELS                                  3

// PIT ports
#define PIT_CHANNEL_DATA_PORT_BASE                          PIT_CHANNEL0_DATA_PORT
#define PIT_CHANNEL0_DATA_PORT                              0x40
#define PIT_CHANNEL1_DATA_PORT                              0x41
#define PIT_CHANNEL2_DATA_PORT                              0x42
#define PIT_COMMAND_REG_PORT                                0x43

// this controls the PIT gates
#define PIT_CONTROL_REG_PORT                                0x61

#define PIT_CONTROL_CH2_INPUT                               (1<<0)
#define PIT_CONTROL_SPEAKER_OUTPUT                          (1<<1)
#define PIT_CONTROL_CH2_OUTPUT                              (1<<5)

#define STATUS_GATE2_MASK                                   (1<<0)
#define STATUS_SPEAKER_MASK                                 (1<<1)
#define STATUS_OUT1_MASK                                    (1<<4)
#define STATUS_OUT2_MASK                                    (1<<5)

#define PIT_COMM_OPERATE_INTERRUPT_ON_TERMINAL_COUNT        0
#define PIT_COMM_OPERATE_RATE_GENERATOR                     2

#define PIT_COMM_ACCESS_LATCH                               0x00
#define PIT_COMM_ACCESS_LO_ONLY                             0x01
#define PIT_COMM_ACCESS_HI_ONLY                             0x02
#define PIT_COMM_ACCESS_LO_HI                               0x03

#pragma pack(push,1)

#pragma warning(push)

// warning C4214: nonstandard extension used: bit field types other than int
#pragma warning(disable:4214)

// warning C4201: nonstandard extension used: nameless struct/union
#pragma warning(disable:4201)

typedef union _PIT_COMMAND_REGISTER
{
    struct
    {
        BYTE            BcdMode             :   1;
        BYTE            OperatingMode       :   3;
        BYTE            AccessMode          :   2;
        BYTE            Channel             :   2;
    };
    BYTE                Raw;
} PIT_COMMAND_REGISTER, *PPIT_COMMAND_REGISTER;
STATIC_ASSERT(sizeof(PIT_COMMAND_REGISTER) == sizeof(BYTE));

#pragma warning(pop)
#pragma pack(pop)

static
void
_PitWriteData(
    IN      BYTE        Channel,
    IN      BYTE        OperatingMode,
    IN      WORD        Data
    );

static
WORD
_PitReadData(
    IN      BYTE        Channel
    );

WORD
PitSetTimer(
    IN      DWORD       Microseconds,
    IN      BOOLEAN     Periodic
    )
{
    DWORD initialCount;

    ASSERT( 0 != Microseconds && Microseconds <= SEC_IN_US);

    initialCount = PIT_FREQUENCY_HZ / (SEC_IN_US / Microseconds);
    ASSERT( initialCount <= MAX_WORD);

    if (!Periodic)
    {
        BYTE value;

        // get current gate controls
        value = __inbyte(PIT_CONTROL_REG_PORT);

        // 0xFD is used to disable speaker output (bit #1)
        // bit0 enables channel 2
        __outbyte(PIT_CONTROL_REG_PORT, (value & (~PIT_CONTROL_SPEAKER_OUTPUT)) | PIT_CONTROL_CH2_INPUT);
    }

    // write frequency to data buffer
    _PitWriteData( Periodic ? PIT_PERIODIC_CHANNEL : PIT_ONETIME_CHANNEL,
                   Periodic ? PIT_COMM_OPERATE_RATE_GENERATOR : PIT_COMM_OPERATE_INTERRUPT_ON_TERMINAL_COUNT,
                   (WORD) initialCount
                   );

    return (WORD) initialCount;
}

void
PitStartTimer(
    void
    )
{
    BYTE value;

    // stop timer countdown
    value = __inbyte(PIT_CONTROL_REG_PORT) & (~PIT_CONTROL_CH2_INPUT);
    __outbyte(PIT_CONTROL_REG_PORT, value);

    // start timer countdown
    __outbyte(PIT_CONTROL_REG_PORT, value | PIT_CONTROL_CH2_INPUT);
}

void
PitWaitTimer(
    void
    )
{
    // when bit 5 is set => Timer fired
    while (!(__inbyte(PIT_CONTROL_REG_PORT) & PIT_CONTROL_CH2_OUTPUT));
}

void
PitSleep(
    IN      DWORD       Microseconds
    )
{
    PitSetTimer(Microseconds, FALSE);
    PitStartTimer();
    PitWaitTimer();
}

WORD
PitGetTimerCount(
    IN      BOOLEAN     Periodic
    )
{
    return _PitReadData(Periodic ? PIT_PERIODIC_CHANNEL : PIT_ONETIME_CHANNEL);
}

static
void
_PitWriteData(
    IN      BYTE        Channel,
    IN      BYTE        OperatingMode,
    IN      WORD        Data
    )
{
    PIT_COMMAND_REGISTER cmdRegister = { 0 };

    ASSERT( Channel < PIT_NO_OF_CHANNELS );

    // we switch to mode 0
    cmdRegister.OperatingMode = OperatingMode;
    cmdRegister.AccessMode = PIT_COMM_ACCESS_LO_HI;
    cmdRegister.Channel = Channel;
    __outbyte(PIT_COMMAND_REG_PORT, cmdRegister.Raw);

    // write low frequency
    __outbyte(PIT_CHANNEL_DATA_PORT_BASE + Channel, WORD_LOW(Data));

    // short delay
    __inbyte(0x60);

    // write high frequency
    __outbyte(PIT_CHANNEL_DATA_PORT_BASE + Channel, WORD_HIGH(Data));
}

static
WORD
_PitReadData(
    IN      BYTE        Channel
    )
{
    PIT_COMMAND_REGISTER cmdRegister = { 0 };
    BYTE lo, hi;

    ASSERT( Channel < PIT_NO_OF_CHANNELS );

    // bits 3 - 0 must be zero for future compatibility
    cmdRegister.OperatingMode = 0;
    cmdRegister.AccessMode = PIT_COMM_ACCESS_LATCH;
    cmdRegister.Channel = Channel;
    __outbyte(PIT_COMMAND_REG_PORT, cmdRegister.Raw);

    // write low frequency
    lo = __inbyte(PIT_CHANNEL_DATA_PORT_BASE + Channel);

    // short delay
    __inbyte(0x60);

    hi = __inbyte(PIT_CHANNEL_DATA_PORT_BASE + Channel);

    return BYTES_TO_WORD(hi,lo);
}