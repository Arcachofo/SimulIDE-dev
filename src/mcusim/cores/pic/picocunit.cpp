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

#include "picocunit.h"
#include "datautils.h"
#include "mcupin.h"
#include "e_mcu.h"
#include "simulator.h"

PicOcUnit::PicOcUnit( eMcu* mcu, QString name )
         : McuOcUnit( mcu, name )
{
    m_enhanced = name.endsWith("E");
    m_resetTimer = false;

    m_GODO = getRegBits( "GO/DONE", mcu );
}
PicOcUnit::~PicOcUnit( ){}

void PicOcUnit::runEvent()  // Compare match
{
    if( !m_enabled ) return;

    if( m_resetTimer ) // 1 Timer cycle after last match
    {
        m_resetTimer = false;
        m_timer->resetTimer();
        return;
    }
    m_interrupt->raise();   // Trigger interrupt
    drivePin( m_comAct );

    if( m_specEvent )
    {
         m_resetTimer = true;
         uint64_t cycles = m_timer->scale()*m_mcu->simCycPI();
         Simulator::self()->addEvent( cycles, this ); // Reset Timer next Timer cycle

         m_mcu->writeReg( m_GODO.regAddr, *m_GODO.reg | m_GODO.mask );  // Set ADC GO/DONE bit
    }
}

void PicOcUnit::configure( uint8_t val ) // CCPxM0,CCPxM1,CCPxM2,CCPxM3
{
    uint8_t mode = getRegBitsVal( val, m_configBits );
    if( mode == 0 ) m_enabled = false;
    if( mode == 2 && m_enhanced ) setOcActs( ocTOGGLE, ocNONE );

    if( (mode & 0b1100) != 0b1000 ) return; // No Compare Mode
    mode = mode & 0b11;
    m_enabled = true;

    if( mode == m_mode ) return;
    m_mode =  mode;
    m_specEvent = false;
    setOcActs( ocNONE, ocNONE );

    switch( mode ) {
        case 0: setOcActs( ocSET, ocCLEAR ); break; // Set OC Pin
        case 1: setOcActs( ocCLEAR, ocSET ); break; // Clear OC Pin
        case 3: m_specEvent = true;                 // Special event
    }
    bool controlPin = m_comAct != ocNONE;
    m_ocPin->controlPin( controlPin, false );
    if( controlPin ) m_ocPin->setOutState( false );
}
