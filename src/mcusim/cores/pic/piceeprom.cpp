/***************************************************************************
 *   Copyright (C) 2022 by santiago González                               *
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

#include "piceeprom.h"
#include "datautils.h"
#include "e_mcu.h"
#include "simulator.h"

PicEeprom::PicEeprom( eMcu* mcu, QString name )
         : McuEeprom( mcu, name )
{
    //m_EECR  = mcu->getReg( "EECR" );
    m_WRERR = getRegBits( "WRERR", mcu );
    m_WREN  = getRegBits( "WREN", mcu );
    m_WR    = getRegBits( "WR", mcu );
    m_RD    = getRegBits( "RD", mcu );
}
PicEeprom::~PicEeprom(){}

void PicEeprom::initialize()
{
    m_writeEnable = false;
    m_nextCycle = 0;
    m_wrMask = 0;
    McuEeprom::initialize();
}

void PicEeprom::runEvent() // Write cycle end reached
{
    writeEeprom();
    clearRegBits( m_WR );
    m_wrMask = 0;
}

void PicEeprom::configureA( uint8_t newEECON1 ) // EECR is being written
{
    if( m_writeEnable ) // Write enabled
    {
        m_writeEnable = false;

        if( m_mcu->cycle() == m_nextCycle ) // Must happen in next cycle
        {
            bool wren = getRegBitsBool( newEECON1, m_WREN );
            bool write = wren && getRegBitsBool( newEECON1, m_WR );
            if( write )
            {
                m_wrMask = m_WR.mask; // Don't clear WR until write finished
                Simulator::self()->addEvent( 5e9, this ); // Write time = 5 ms
            }
        }
        m_nextCycle = 0;
    }
    else if( getRegBitsBool( newEECON1, m_RD ) ) // Read enable
    {
        readEeprom();
    }
    newEECON1 &= ~(m_WR.mask);                                    // Clear WR if not in write cycle
    m_mcu->m_regOverride = (newEECON1 | m_wrMask) & ~(m_RD.mask); // Clear RD, set WR if in write cycle
}

void PicEeprom::configureB( uint8_t newEECON2 )
{
    if     ( newEECON2 == 0x55 ) m_nextCycle = m_mcu->cycle()+2;
    else if( newEECON2 == 0xAA )
    {
        if( m_mcu->cycle() == m_nextCycle )
        {
            m_nextCycle = m_mcu->cycle()+1;
            m_writeEnable = true;
        }
    }
    m_mcu->m_regOverride = 0; // Don't write value (this is not a physical register).
}

/*void PicEeprom::writeEeprom()
{
    uint8_t data = *m_dataReg;
    uint64_t time;

    switch( m_mode )
    {
        case 0:     // 3.4 ms - Erase and Write in one operation (Atomic Operation)
            time = 3400*1e6; // picoseconds
            break;
        case 1:     // 1.8 ms - Erase Only
            data = 0xFF;
            // fallthrough
        case 2:     // 1.8 ms - Write Only
            time = 1800*1e6; // picoseconds
            break;
    }
    m_mcu->setRomValue( m_address, data );
    m_mcu->cyclesDone += 2;

    Simulator::self()->addEvent( time, this ); // Shedule Write cycle end
}*/

