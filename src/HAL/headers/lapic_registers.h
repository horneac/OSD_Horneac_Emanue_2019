#pragma once

#define LAPIC_VERSION_REGISTER_OFFSET                   0x030
#define LAPIC_ERROR_REGISTER_OFFSET                     0x280
#define LAPIC_LVT_TIMER_REGISTER_OFFSET                 0x320
#define LAPIC_TIMER_INITIAL_COUNT_REGISTER_OFFSET       0x380
#define LAPIC_TIMER_CURRENT_COUNT_REGISTER_OFFSET       0x390
#define LAPIC_TIMER_DIVIDE_REGISTER_OFFSET              0x3E0

#define PREDEFINED_LAPIC_USABLE_REG_SIZE                4
#define PREDEFINED_LAPIC_REGISTER_SIZE                  0x10
#define PREDEFINED_LAPIC_SIZE                           0x400

#define APIC_TIMER_ONE_SHOT_MODE                        0b00
#define APIC_TIMER_PERIOD_MODE                          0b01

#define APIC_FIRST_USABLE_INTERRUPT_INDEX               0x20

#define APIC_TIMER_CONFIGURATION_SLEEP                  (10*MS_IN_US)

#define APIC_VECTORS_PER_ISR_REGISTER                   32

#define IA32_APIC_BASE_MASK(reg)                        (((QWORD)(reg))&0xFFFFFF000)
#define IA32_APIC_BASE_ENABLE_FLAG                      (1<<11)     // if set => APIC enabled
#define IA32_APIC_EXT_ENABLE_FLAG                       (1<<10)        // x2 APIC enable
#define IA32_APIC_BSP_FLAG                              (1<<8)

#pragma pack(push,1)

#pragma warning(push)

// warning C4214: nonstandard extension used: bit field types other than int
#pragma warning(disable:4214)

// warning C4201: nonstandard extension used: nameless struct/union
#pragma warning(disable:4201)
typedef union _ICR_HIGH_REGISTER
{
    struct
    {
        DWORD           Reserved                :   24;
        DWORD           Destination             :   8;
    };
    DWORD               Raw;
} ICR_HIGH_REGISTER, *PICR_HIGH_REGISTER;
STATIC_ASSERT(sizeof(ICR_HIGH_REGISTER) == PREDEFINED_LAPIC_USABLE_REG_SIZE);

typedef union _ICR_LOW_REGISTER
{
    struct
    {
        DWORD           Vector                  :   8;
        DWORD           DeliveryMode            :   3;
        DWORD           DestinationMode         :   1;
        DWORD           DeliveryStatus          :   1;
        DWORD           Reserved0               :   1;
        DWORD           Level                   :   1;
        DWORD           TriggerMode             :   1;
        DWORD           Reserved1               :   2;
        DWORD           DestinationShorthand    :   2;
        DWORD           Reserved2               :  12;
    };
    DWORD               Raw;
} ICR_LOW_REGISTER, *PICR_LOW_REGISTER;
STATIC_ASSERT(sizeof(ICR_LOW_REGISTER) == PREDEFINED_LAPIC_USABLE_REG_SIZE);

typedef union _LAPIC_VERSION_REGISTER
{
    struct
    {
        BYTE            Version;
        BYTE            __Reserved0;
        BYTE            MaxLvtEntry;
        BYTE            EoiBcastSuppressSupport :    1;
        BYTE            __Reserved1             :    7;
    };
    DWORD               Raw;
} LAPIC_VERSION_REGISTER, *PLAPIC_VERSION_REGISTER;
STATIC_ASSERT(sizeof(LAPIC_VERSION_REGISTER) == PREDEFINED_LAPIC_USABLE_REG_SIZE);

typedef union _SVR_REGISTER
{
    struct
    {
        DWORD           Vector                  :    8;
        DWORD           ApicEnable              :    1;
        DWORD           FocusProcessorChecking  :    1;
        DWORD           __Reserved0             :    2;
        DWORD           EOIBroadcastSuppresion  :    1;
        DWORD           __Reserved1             :   19;
    };
    DWORD               Raw;
} SVR_REGISTER, *PSVR_REGISTER;
STATIC_ASSERT(sizeof(SVR_REGISTER) == PREDEFINED_LAPIC_USABLE_REG_SIZE);

typedef union _LVT_REGISTER
{
    struct
    {
        DWORD           Vector                  :    8;

        // Reserved for Timer and Error
        DWORD           DeliveryMode            :    3;

        DWORD           __Reserved0             :    1;

        DWORD           DeliveryStatus          :    1;

        // Valid only for LINT0 and LINT1
        DWORD           PinPolarity             :    1;
        DWORD           RemoteIRR               :    1;
        DWORD           TriggerMode             :    1;

        // Valid for all
        DWORD           Masked                  :    1;

        // valid only for TIMER
        DWORD           TimerMode               :    2;

        DWORD           __Reserved1             :   13;
    };
    DWORD               Raw;
} LVT_REGISTER, *PLVT_REGISTER;
STATIC_ASSERT(sizeof(LVT_REGISTER) == PREDEFINED_LAPIC_USABLE_REG_SIZE);

typedef union _LDR_REGISTER
{
    struct
    {
        BYTE            __Reserved0[3];

        BYTE            LogicalApicId;
    };
    DWORD               Raw;
} LDR_REGISTER, *PLDR_REGISTER;
STATIC_ASSERT(sizeof(LDR_REGISTER) == PREDEFINED_LAPIC_USABLE_REG_SIZE);

typedef union _DFR_REGISTER
{
    struct
    {
        DWORD           __Reserved0             :  28;

        DWORD           Model                   :   4;
    };
    DWORD               Raw;
} DFR_REGISTER, *PDFR_REGISTER;
STATIC_ASSERT(sizeof(DFR_REGISTER) == PREDEFINED_LAPIC_USABLE_REG_SIZE);

typedef struct _LAPIC_REGISTER
{
    volatile DWORD          Value;
    DWORD                   __Reserved[3];
} LAPIC_REGISTER, *PLAPIC_REGISTER;
STATIC_ASSERT(sizeof(LAPIC_REGISTER) == PREDEFINED_LAPIC_REGISTER_SIZE);

typedef struct _LAPIC
{
    LAPIC_REGISTER          __Reserved0[2];

    // R/W
    LAPIC_REGISTER          ApicId;

    // Read only
    LAPIC_REGISTER          ApicVersion;
    LAPIC_REGISTER          __Reserved1[4];

    // R/W
    LAPIC_REGISTER          TPR;

    // Read only
    LAPIC_REGISTER          APR;

    // Read only
    LAPIC_REGISTER          PPR;

    // Write only
    LAPIC_REGISTER          EOI;

    // Read only
    LAPIC_REGISTER          RRD;

    // R/W
    LAPIC_REGISTER          LogicalDestination;

    // R/W
    LAPIC_REGISTER          DestinationFormat;

    // R/W
    LAPIC_REGISTER          SpuriousInterruptVector;

    // Read only
    LAPIC_REGISTER          ISR[8];

    // Read only
    LAPIC_REGISTER          TMR[8];

    // Read only
    LAPIC_REGISTER          IRR[8];

    // Read only
    LAPIC_REGISTER          ErrorStatus;

    LAPIC_REGISTER          __Reserved2[6];

    // R/W
    LAPIC_REGISTER          LvtCMCI;

    // R/W
    LAPIC_REGISTER          IcrLow;
    LAPIC_REGISTER          IcrHigh;

    // R/W
    LAPIC_REGISTER          LvtTimer;

    // R/W
    LAPIC_REGISTER          LvtThermalSensor;

    // R/W
    LAPIC_REGISTER          LvtPerformanceMonitoringCounters;

    // R/W
    LAPIC_REGISTER          LvtLINT0;
    LAPIC_REGISTER          LvtLINT1;

    // R/W
    LAPIC_REGISTER          LvtError;

    // R/W
    LAPIC_REGISTER          TimerInitialCount;

    // Read only
    LAPIC_REGISTER          TimerCurrentCount;

    LAPIC_REGISTER          __Reserved3[4];

    // R/W
    LAPIC_REGISTER          TimerDivideConfiguration;

    LAPIC_REGISTER          __Reserved4;
} LAPIC, *PLAPIC;
STATIC_ASSERT(sizeof(LAPIC) == PREDEFINED_LAPIC_SIZE );
STATIC_ASSERT(FIELD_OFFSET(LAPIC,ApicVersion) == LAPIC_VERSION_REGISTER_OFFSET);
STATIC_ASSERT(FIELD_OFFSET(LAPIC,ErrorStatus) == LAPIC_ERROR_REGISTER_OFFSET);
STATIC_ASSERT(FIELD_OFFSET(LAPIC,LvtTimer) == LAPIC_LVT_TIMER_REGISTER_OFFSET );
STATIC_ASSERT(FIELD_OFFSET(LAPIC,TimerInitialCount) == LAPIC_TIMER_INITIAL_COUNT_REGISTER_OFFSET );
STATIC_ASSERT(FIELD_OFFSET(LAPIC,TimerCurrentCount) == LAPIC_TIMER_CURRENT_COUNT_REGISTER_OFFSET );
STATIC_ASSERT(FIELD_OFFSET(LAPIC,TimerDivideConfiguration) == LAPIC_TIMER_DIVIDE_REGISTER_OFFSET );

#pragma warning(pop)
#pragma pack(pop)
