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

#include "avrwdt.h"
#include "e_mcu.h"
#include "simulator.h"

AvrWdt::AvrWdt( eMcu* mcu, QString name )
      : McuWdt( mcu, name )
{
    m_clkPeriod = 1e12/1048576; //

    m_WDIF = mcu->getRegBits( "WDIF" );
    m_WDIE = mcu->getRegBits( "WDIE" );
    m_WDCE = mcu->getRegBits( "WDCE" );
    m_WDE  = mcu->getRegBits( "WDE" );
    m_WDP  = mcu->getRegBits( "WDP0, WDP1, WDP2, WDP3" );
}
AvrWdt::~AvrWdt(){}

void AvrWdt::initialize()
{
    m_prescaler = 0; // 2K cycles = 16 ms
    m_ovfPeriod = m_clkPeriod/m_prescList[ m_prescaler ];

    McuWdt::initialize();
}

void AvrWdt::configureA( uint8_t newWDTCSR ) // WDTCSR Written
{
    bool clearWdif = getRegBitsVal( newWDTCSR, m_WDIF );
    if( clearWdif )                       /// Writting 1 to WDIF clears the flag
    {
        m_mcu->m_regOverride = newWDTCSR & ~m_WDIF.mask; // Clear WDIF flag
    }
    m_ovfInter = getRegBitsVal( newWDTCSR, m_WDIE );
    wdtEnable();

    if( !getRegBitsVal( newWDTCSR, m_WDCE ) ) return; // Prescaler & WDE changes not enabled

    m_ovfReset  = getRegBitsVal( newWDTCSR, m_WDE );
    m_prescaler = getRegBitsVal( newWDTCSR, m_WDP );
    m_ovfPeriod = m_clkPeriod/m_prescList[ m_prescaler ];

    wdtEnable();
}

void AvrWdt::wdtEnable()
{
    m_enabled = m_ovfInter || m_ovfReset;
    if( m_enabled )
    {
        Simulator::self()->cancelEvents( this );
        Simulator::self()->addEvent( m_ovfPeriod, this );
    }
}
