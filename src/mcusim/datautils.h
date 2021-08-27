/***************************************************************************
 *   Copyright (C) 2021 by santiago González                               *
 *   santigoro@gmail.com                                                   *
 *                                                                         *
 *   This program is free software; you can redistribute it and/or modify  *
 *   it under the terms of the GNU General Public License as published by  *
 *   the Free Software Foundation; either version 3 of the License, or     *
 *   (at your option) any later version.                                   *
 *                                                                         *
 *   This program is distributed in the hope that it will be useful,       *
 *   but WITHOUT ANY WARRANTY; without even the implied warranty of        *
 *   MERCHANTABILITY or FITNESS FOR A PARTICULAR PURPOSE.  See the         *
 *   GNU General Public License for more details.                          *
 *                                                                         *
 *   You should have received a copy of the GNU General Public License     *
 *   along with this program; if not, see <http://www.gnu.org/licenses/>.  *
 *                                                                         *
 ***************************************************************************/

#ifndef DATAUTILS_H
#define DATAUTILS_H

#include <QStringList>

#include "mcudataspace.h"
//#include "mcutypes.h"

static inline uint8_t overrideBits( uint8_t val, regBits_t bits ) // Replace bits in val with current value in register bits.reg
{
    return (val & ~bits.mask) | (*(bits.reg) | bits.mask);
}

static inline uint8_t getRegBits( uint8_t val, regBits_t rb )
{
    return (val & rb.mask);
}

static inline uint8_t getRegBitsVal( uint8_t val, regBits_t rb )
{
    return (val & rb.mask)>>rb.bit0;
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

uint8_t getBitMask( QStringList bitList, DataSpace* mcu ); // Get mask for a group of bits in a Register

regBits_t getRegBits( QString bitNames, DataSpace* mcu ); // Get a set of consecutive bits in a Register

#endif
