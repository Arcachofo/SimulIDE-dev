/***************************************************************************
 *   Copyright (C) 2021 by Santiago González                               *
 *                                                                         *
 ***( see copyright.txt file at root folder )*******************************/

#ifndef DATAUTILS_H
#define DATAUTILS_H

#include <QStringList>

#include "mcutypes.h"

class DataSpace;

uint8_t getBitMask( QStringList bitList, DataSpace* mcu ); // Get mask for a group of bits in a Register

regBits_t getRegBits( QString bitNames, DataSpace* mcu ); // Get a set of consecutive bits in a Register

static inline uint8_t overrideBits( uint8_t val, regBits_t bits ) // Replace bits in val with current value in register bits.reg
{
    return (val & ~bits.mask) | (*(bits.reg) | bits.mask);
}

static inline uint8_t getRegBits( uint8_t val, regBits_t rb )
{
    return (val & rb.mask);
}

static inline uint8_t getRegBitsVal( regBits_t rb )
{
    return (*rb.reg & rb.mask)>>rb.bit0;
}

static inline uint8_t getRegBitsVal( uint8_t val, regBits_t rb )
{
    return (val & rb.mask)>>rb.bit0;
}

static inline uint8_t getRegBitsBool( regBits_t rb )
{
    return (*rb.reg & rb.mask) > 0;
}

static inline uint8_t getRegBitsBool( uint8_t val, regBits_t rb )
{
    return (val & rb.mask) > 0;
}

static inline void setRegBits( regBits_t bits )
{
    *(bits.reg) |= bits.mask;
}

static inline void clearRegBits( regBits_t bits ) // Clear bits in a Register
{
    *(bits.reg) &= ~bits.mask;
}

static inline void writeRegBits( regBits_t bits, bool value ) // Clear bits in a Register
{
    if( value ) setRegBits( bits );
    else        clearRegBits( bits );
}
#endif
