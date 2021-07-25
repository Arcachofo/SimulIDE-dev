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

#include "avreeprom.h"
#include "e_mcu.h"
#include "simulator.h"

AvrEeprom::AvrEeprom( eMcu* mcu, QString name )
         : McuEeprom( mcu, name )
{
    m_EECR  = mcu->getReg( "EECR" );
    m_EEPM  = mcu->getRegBits( "EEPM0, EEPM1" );
    m_EEMPE = mcu->getRegBits( "EEMPE" );
    m_EEPE  = mcu->getRegBits( "EEPE" );
    m_EERE  = mcu->getRegBits( "EERE" );
}

AvrEeprom::~AvrEeprom()
{
}

void AvrEeprom::initialize()
{
    m_mode = 0;
    McuEeprom::initialize();
}

void AvrEeprom::runEvent() // Write cycle end reached
{
    clearRegBits( m_EEPE );
}

void AvrEeprom::configureA( uint8_t newEECR ) // EECR is being written
{
    m_mode = getRegBitsVal( newEECR, m_EEPM );

    bool eempe    = getRegBitsVal( newEECR, m_EEMPE );

    if( eempe )
    {
        bool oldEepe = getRegBitsVal( *m_EECR, m_EEPE );
        bool eepe    = getRegBitsVal( newEECR, m_EEPE );
        if( !oldEepe && eepe ) // Write triggered
        {
            if( m_mcu->cycle() <= m_nextCycle ) writeEeprom();// write data
            /// else clearRegBits( m_EEPE );
        }
        return;
    }
    bool oldEempe = getRegBitsVal( *m_EECR, m_EEMPE );
    if( !oldEempe && eempe ) // Set maximun cycle to procedd to write
    {
        m_nextCycle = m_mcu->cycle()+4;
    }

    if( getRegBitsVal( newEECR, m_EERE ) ) // Read enable
    {
        m_mcu->cyclesDone += 4;
        readEeprom();
    }
}

void AvrEeprom::writeEeprom()
{
    uint8_t data = *m_dataReg;
    uint64_t time;

    switch( m_mode )
    {
        case 0:     // 3.4 ms - Erase and Write in one operation (Atomic Operation)
            time = 3400*1e12; // picoseconds
            break;
        case 1:     // 1.8 ms - Erase Only
            data = 0xFF;
            // fallthrough
        case 2:     // 1.8 ms - Write Only
            time = 1800*1e12; // picoseconds
            break;
    }
    m_mcu->setRomValue( m_address, data );
    m_mcu->cyclesDone += 2;

    Simulator::self()->addEvent( time, this ); // Shedule Write cycle end
}

