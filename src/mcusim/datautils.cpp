#include "datautils.h"

uint8_t getBitMask( QStringList bitList, DataSpace* mcu ) // Get mask for a group of bits in a Register
{
    uint8_t bitMask = 0;
    for( QString bitName : bitList ) bitMask |= mcu->bitMasks()->value( bitName );
    return bitMask;
}

regBits_t getRegBits( QString bitNames, DataSpace* mcu ) // Get a set of consecutive bits in a Register
{
    regBits_t regBits;
    QStringList bitList = bitNames.split(",");

    uint8_t mask = getBitMask( bitList, mcu );
    regBits.mask = mask;

    for( regBits.bit0=0; regBits.bit0<8; ++regBits.bit0 ) // Rotate mask to get initial bit
    {
        if( mask & 1 ) break;
        mask >>= 1;
    }
    uint16_t regAddr = mcu->bitRegs()->value( bitList.first() );
    uint8_t* ram = mcu->getRam();
    regBits.reg = ram+regAddr;

    return regBits;
}

