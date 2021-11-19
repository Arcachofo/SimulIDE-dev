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

#include "picwdt.h"
#include "e_mcu.h"
#include "mcucore.h"
#include "simulator.h"
#include "datautils.h"

PicWdt::PicWdt( eMcu* mcu, QString name )
      : McuWdt( mcu, name )
{
    m_clkPeriod = 18*1e9; // 18 ms

    m_PS  = getRegBits( "PS0, PS1, PS2", mcu );
    m_PSA = getRegBits( "PSA", mcu );
}
PicWdt::~PicWdt(){}

void PicWdt::initialize()
{
    McuWdt::initialize();
    m_prescaler = 0;
    m_ovfPeriod = m_clkPeriod/m_prescList[ m_prescaler ];
    m_ovfReset = true;
}

/*void PicWdt::runEvent()
{
}*/

void PicWdt::configureA( uint8_t newOPTION ) // OPTION Written
{
    if( !m_enabled ) return;

    if( getRegBitsVal( newOPTION, m_PSA ) )
         m_prescaler = getRegBitsVal( newOPTION, m_PS );  // Prescaler asigned to Watchdog
    else m_prescaler = 0;                           // Prescaler asigned to TIMER0
    m_ovfPeriod = m_clkPeriod/m_prescList[ m_prescaler ];
}

void PicWdt::reset()
{
    Simulator::self()->cancelEvents( this );
    if( m_enabled ) Simulator::self()->addEvent( m_ovfPeriod, this );
}


