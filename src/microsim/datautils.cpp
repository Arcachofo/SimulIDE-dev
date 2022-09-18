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

#include "datautils.h"
#include "mcudataspace.h"

uint8_t getBitMask( QStringList bitList, DataSpace* mcu ) // Get mask for a group of bits in a Register
{
    uint8_t bitMask = 0;
    for( QString bitName : bitList ) bitMask |= mcu->bitMasks()->value( bitName.remove(" ") );
    return bitMask;
}

regBits_t getRegBits( QString bitNames, DataSpace* mcu ) // Get a set of bits in a Register
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
    regBits.regAddr = mcu->bitRegs()->value( bitList.first() );
    uint8_t* ram = mcu->getRam();
    regBits.reg = ram + regBits.regAddr;

    return regBits;
}

