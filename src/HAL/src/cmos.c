#include "hal_base.h"
#include "cmos.h"

#define CMOS_ADDRESS_PORT               0x70
#define CMOS_DATA_PORT                  0x71

#define CMOS_DISABLE_NMI_BIT            7

BYTE
CmosGetValue(
    IN  CMOS_REGISTER   RegisterIndex,
    IN  BOOLEAN         DisableNMI
    )
{
    ASSERT(RegisterIndex <= MAX_BYTE);

    __outbyte(CMOS_ADDRESS_PORT, (BYTE) RegisterIndex | (DisableNMI << CMOS_DISABLE_NMI_BIT ));
    return __inbyte(CMOS_DATA_PORT);
}

void
CmosWriteValue(
    IN  CMOS_REGISTER   RegisterIndex,
    IN  BOOLEAN         DisableNMI,
    IN  BYTE            Value
    )
{
    ASSERT(RegisterIndex <= MAX_BYTE);

    __outbyte(CMOS_ADDRESS_PORT, (BYTE)RegisterIndex | (DisableNMI << CMOS_DISABLE_NMI_BIT));
    __outbyte(CMOS_DATA_PORT,Value);
}

static
__forceinline
BOOLEAN
_CmosIsUpdateInProgress(
    void
    )
{
    return IsBooleanFlagOn(CmosGetValueNMI(CmosRegisterStatusA), CMOS_UPDATE_IN_PROGRESS);
}

void
CmosReadData(
    OUT     PCMOS_DATA      CmosData
    )
{
    BYTE second;
    BYTE minute;
    BYTE hour;
    BYTE day;
    BYTE month;
    BYTE year;
    BYTE century;
    BYTE registerB;

    ASSERT(NULL != CmosData);

    // wait for the CMOS update to finish
    while (_CmosIsUpdateInProgress());

    second  = CmosGetValueNMI(CmosRegisterSeconds);
    minute  = CmosGetValueNMI(CmosRegisterMinutes);
    hour    = CmosGetValueNMI(CmosRegisterHours);
    day     = CmosGetValueNMI(CmosRegisterDay);
    month   = CmosGetValueNMI(CmosRegisterMonth);
    year    = CmosGetValueNMI(CmosRegisterYear);
    century = CmosGetValueNMI(CmosRegisterCentury);

    registerB = CmosGetValueNMI(CmosRegisterStatusB);

    // Convert BCD to binary values if necessary

    if (!(IsBooleanFlagOn(registerB, CMOS_BINARY_MODE)))
    {
        second = (second & MAX_NIBBLE) + ((second / 16) * 10);
        minute = (minute & MAX_NIBBLE) + ((minute / 16) * 10);
        hour = ((hour & MAX_NIBBLE) + (((hour & 0x70) / 16) * 10)) | (hour & 0x80);
        day = (day & MAX_NIBBLE) + ((day / 16) * 10);
        month = (month & MAX_NIBBLE) + ((month / 16) * 10);
        year = (year & MAX_NIBBLE) + ((year / 16) * 10);
        century = (century & MAX_NIBBLE) + ((century / 16) * 10);
    }

    // Convert 12 hour clock to 24 hour clock if necessary
    if (!(IsBooleanFlagOn(registerB,CMOS_24H_FORMAT)) && (hour & 0x80))
    {
        hour = ((hour & 0x7F) + 12) % 24;
    }

    CmosData->Second = second;
    CmosData->Minute = minute;
    CmosData->Hour = hour;
    CmosData->Day = day;
    CmosData->Month = month;
    CmosData->Year = year + century * 100;
}