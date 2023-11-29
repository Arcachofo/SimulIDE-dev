/***************************************************************************
 *   Copyright (C) 2021 by Santiago González                               *
 *                                                                         *
 ***( see copyright.txt file at root folder )*******************************/

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

    if( write )
    {
        McuSignal* regSignal = mcu->writeSignals()->value( addr );
        if( !regSignal )
        {
            regSignal = new McuSignal;
            mcu->writeSignals()->insert( addr, regSignal );
        }
        regSignal->connect( inst, func, mask );
    }else{
        McuSignal* regSignal = mcu->readSignals()->value( addr );
        if( !regSignal )
        {
            regSignal = new McuSignal;
            mcu->readSignals()->insert( addr, regSignal );
        }
        regSignal->connect( inst, func, mask  );
    }
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
