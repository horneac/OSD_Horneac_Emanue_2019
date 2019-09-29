#pragma once

typedef enum _APIC_DELIVERY_MODE
{
    ApicDeliveryModeFixed,
    ApicDeliveryModeLowest,
    ApicDeliveryModeSMI = 2,
    ApicDeliveryModeNMI = 4,
    ApicDeliveryModeINIT,
    ApicDeliveryModeSIPI,
    ApicDeliveryModeExtINT
} APIC_DELIVERY_MODE;

typedef enum _APIC_DESTINATION_MODE
{
    ApicDestinationModePhysical,
    ApicDestinationModeLogical
} APIC_DESTINATION_MODE;

typedef enum _APIC_DESTINATION_SHORTHAND
{
    ApicDestinationShorthandNone,
    ApicDestinationShorthandSelf,
    ApicDestinationShorthandAll,
    ApicDestinationShorthandAllExcludingSelf
} APIC_DESTINATION_SHORTHAND;

typedef enum _APIC_DIVIDE_VALUE
{
    ApicDivideBy2       = 0b0'0'00,
    ApicDivideBy4       = 0b0'0'01,
    ApicDivideBy8       = 0b0'0'10,
    ApicDivideBy16      = 0b0'0'11,
    ApicDivideBy32      = 0b1'0'00,
    ApicDivideBy64      = 0b1'0'01,
    ApicDivideBy128     = 0b1'0'10,
    ApicDivideBy1       = 0b1'0'11,

    ApicDivideReserved  = 0b1'1'11
} APIC_DIVIDE_VALUE;

typedef enum _APIC_PIN_POLARITY
{
    ApicPinPolarityActiveHigh,
    ApicPinPolarityActiveLow
} APIC_PIN_POLARITY;

typedef enum _APIC_TRIGGER_MODE
{
    ApicTriggerModeEdge,
    ApicTriggerModeLevel
} APIC_TRIGGER_MODE;

#define APIC_DESTINATION_FORMAT_FLAT_MODEL         0b1111
#define APIC_DESTINATION_FORMAT_CLUSTER_MODEL      0b0000