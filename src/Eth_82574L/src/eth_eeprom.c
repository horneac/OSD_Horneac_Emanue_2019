#include "eth_82574L_base.h"
#include "eth_eeprom.h"

WORD
EthEepromReadWord(
    IN      PETH_INTERNAL_REGS      EthRegs,
    IN      WORD                    Address
    )
{
    EERD_REGISTER reg;

    ASSERT( NULL != EthRegs );

    reg.Raw = 0;
    reg.Address = Address;
    reg.Start = 1;

    EthRegs->EepromReadRegister = reg.Raw;

    reg.Raw = EthRegs->EepromReadRegister;
    ASSERT( !reg.Start && reg.Done );

    return reg.Data;
}