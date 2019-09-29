#pragma once

#pragma pack(push, 1)

#pragma warning(push)

// warning C4201: nonstandard extension used : nameless struct/union
#pragma warning(disable:4201)

// warning C4214: nonstandard extension used: bit field types other than int
#pragma warning(disable:4214)

#define PREDEFINED_CPU_RST_CTRL_REG_SIZE    1

#define CPU_RST_CTRL_REG_PORT               0xCF9

typedef union _CPU_RST_CTRL_REG
{
    struct
    {
        BYTE                                        __reserved0                 : 1;

        // System Reset(SYS_RST) - R / W. This bit is used to determine a hard or soft reset to the
        // processor.
        //     0 = When RST_CPU bit goes from 0 to 1, the PCH performs a soft reset by activating INIT# for 16
        //     PCI clocks.
        //     1 = When RST_CPU bit goes from 0 to 1, the PCH performs a hard reset by activating PLTRST# and
        //     SUS_STAT# active for a minimum of about 1 milliseconds.In this case, SLP_S3#, SLP_S4#
        //     and SLP_S5# state(assertion or de-assertion) depends on FULL_RST bit setting.The PCH
        //     main power well is reset when this bit is 1. It also resets the resume well bits(except for those
        //         noted throughout this document).
        BYTE                                        SYS_RST                     : 1;

        // Reset Processor(RST_CPU) - R / W. When this bit transitions from a 0 to a 1, it initiates a hard or
        // soft reset, as determined by the SYS_RST bit(bit 1 of this register).
        BYTE                                        RST_CPU                     : 1;

        // Full Reset(FULL_RST) - R / W.This bit is used to determine the states of SLP_S3#, SLP_S4#, and
        //     SLP_S5# after a CF9 hard reset(SYS_RST = 1 and RST_CPU is set to 1), after PWROK going low
        //     (with RSMRST# high), or after two TCO timeouts.
        //     0 = PCH will keep SLP_S3#, SLP_S4# and SLP_S5# high.
        //     1 = PCH will drive SLP_S3#, SLP_S4# and SLP_S5# low for 3–5 seconds.
        //     Note : When this bit is set, it also causes the full power cycle(SLP_S3 / 4 / 5# assertion) in response
        //     to SYS_RESET#, PWROK#, and Watchdog timer reset sources.
        BYTE                                        FULL_RST                    : 1;

        BYTE                                        __reserved4_7               : 4;
    };

    BYTE                                            Raw;
} CPU_RST_CTRL_REG;
STATIC_ASSERT_INFO(sizeof(CPU_RST_CTRL_REG) == PREDEFINED_CPU_RST_CTRL_REG_SIZE,
    "See Intel 9 Series Chipset Family Platform Controller Hub(PCH) section 12.7.5 RST_CNT - Reset Control Register");

#pragma warning(pop)
#pragma pack(pop)
