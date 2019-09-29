#pragma once

typedef struct _CMOS_DATA
{
    BYTE            Second;
    BYTE            Minute;
    BYTE            Hour;
    BYTE            Day;
    BYTE            Month;
    WORD            Year;
} CMOS_DATA, *PCMOS_DATA;

typedef enum _CMOS_REGISTER
{
    CmosRegisterSeconds = 0x0,
    CmosRegisterMinutes = 0x2,
    CmosRegisterHours   = 0x4,
    CmosRegisterDay     = 0x7,
    CmosRegisterMonth   = 0x8,
    CmosRegisterYear    = 0x9,
    CmosRegisterCentury = 0x32,
    CmosRegisterStatusA = 0xA,
    CmosRegisterStatusB = 0xB,
    CmosRegisterStatusC = 0xC
} CMOS_REGISTER;

// Status Register A flags
#define CMOS_UPDATE_IN_PROGRESS         (1<<7)

// Status Register B flags
#define CMOS_PERIODIC_INTERRUPT         (1<<6)
#define CMOS_ALARM_INTERRUPT            (1<<5)
#define CMOS_UPDATE_INTERRUPT           (1<<4)
#define CMOS_BINARY_MODE                (1<<2)
#define CMOS_24H_FORMAT                 (1<<1)

void
CmosReadData(
    OUT     PCMOS_DATA      CmosData
    );

BYTE
CmosGetValue(
    IN  CMOS_REGISTER   RegisterIndex,
    IN  BOOLEAN         DisableNMI
    );

#define CmosGetValueDisableNMI(Idx)     CmosGetValue((Idx),TRUE)
#define CmosGetValueNMI(Idx)            CmosGetValue((Idx),FALSE)

void
CmosWriteValue(
    IN  CMOS_REGISTER   RegisterIndex,
    IN  BOOLEAN         DisableNMI,
    IN  BYTE            Value
    );

#define CmosWriteValueDisableNMI(Idx,Data)      CmosWriteValue((Idx),TRUE,(Data))
#define CmosWriteValueNMI(Idx,Data)             CmosWriteValue((Idx),FALSE,(Data))