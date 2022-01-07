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

#ifndef REGWATCHER_H
#define REGWATCHER_H

#include <QDebug>

#include "mcudataspace.h"

template <class T>                // Add callback for Register changes by address
void watchRegister( uint16_t addr, int write
                  , T* inst, void (T::*func)(uint8_t)
                  , DataSpace* mcu, uint8_t mask=0xFF )
{
    if( addr == 0 ) qDebug() << "Warning: watchRegister address 0 ";

    regSignal_t* regSignal = mcu->regSignals()->value( addr );
    if( !regSignal )
    {
        regSignal = new regSignal_t;
        mcu->regSignals()->insert( addr, regSignal );
    }
    if( write ) regSignal->on_write.connect( inst, func, mask );
    else        regSignal->on_read.connect( inst, func, mask  );
}

template <class T>                // Add callback for Register changes by names
void watchRegNames( QString regNames, int write
                  , T* inst, void (T::*func)(uint8_t)
                  , DataSpace* mcu)
{
    if( regNames.isEmpty() ) return;

    QStringList regs = regNames.split(",");
    for( QString reg : regs )
    {
        if( !mcu->regInfo()->contains( reg ) )
        {
            qDebug() << "ERROR: Register not found: " << reg;
            continue;
        }
        uint16_t addr = mcu->regInfo()->value( reg ).address;
        watchRegister( addr, write, inst, func, mcu );
    }
}

template <class T>              // Add callback for Register bit changes by names
void watchBitNames( QString bitNames, int write
              , T* inst, void (T::*func)(uint8_t)
              , DataSpace* mcu )
{
    if( bitNames.isEmpty() ) return;

    uint16_t regAddr = 0;
    QStringList bitList = bitNames.split(",");
    uint8_t     bitMask = getBitMask( bitList, mcu );

    regAddr = mcu->bitRegs()->value( bitList.first() );

    if( regAddr )
        watchRegister( regAddr, write, inst, func, mcu, bitMask );
}
#endif
