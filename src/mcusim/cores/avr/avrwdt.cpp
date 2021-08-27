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
#include "mcucore.h"
#include "simulator.h"
#include "datautils.h"

AvrWdt::AvrWdt( eMcu* mcu, QString name )
      : McuWdt( mcu, name )
{
    m_clkPeriod = 8.192*1e12; // 1048576 cycles * 7812500 ps (128 KHz)

    m_WDTCSR = mcu->getReg( "WDTCSR" );

    m_WDIF = getRegBits( "WDIF", mcu );
    m_WDIE = getRegBits( "WDIE", mcu );
    m_WDCE = getRegBits( "WDCE", mcu );
    m_WDE  = getRegBits( "WDE", mcu );
    m_WDP02 = getRegBits( "WDP0, WDP1, WDP2", mcu );
    m_WDP3  = getRegBits( "WDP3", mcu );

    m_WDRF = getRegBits( "WDRF", mcu );
}
AvrWdt::~AvrWdt(){}

void AvrWdt::initialize()
{
    m_allowChanges = false;
    m_prescaler = 0; // 2K cycles = 16 ms
    m_ovfPeriod = m_clkPeriod/m_prescList[ m_prescaler ];

    McuWdt::initialize();
}

void AvrWdt::runEvent()
{
    if( m_allowChanges )
    {
        clearRegBits( m_WDCE );
        m_allowChanges = false;
    }
    else // McuWdt::runEvent();
    {
        if( m_ovfInter ) m_interrupt->raise();
        else if( m_ovfReset ) // No interrupt, just Reset
        {
            qDebug() << "AvrWdt::runEvent - Watchdog Reset\n";
            m_mcu->cpu->reset();
        }
        Simulator::self()->addEvent( m_ovfPeriod, this );
    }
}

void AvrWdt::callBack() // WDT Overflow Interrupt just executed
{
    if( !m_enabled ) return;
    qDebug() << "AvrWdt::callBack - Watchdog Reset\n";
    m_mcu->cpu->reset();
}

void AvrWdt::configureA( uint8_t newWDTCSR ) // WDTCSR Written
{
    bool clearWdif = getRegBitsVal( newWDTCSR, m_WDIF );
    if( clearWdif )  /// Writting 1 to WDIF clears the flag
    {
        newWDTCSR &= ~m_WDIF.mask; // Clear WDIF flag
    }
    m_ovfInter = getRegBitsVal( newWDTCSR, m_WDIE );

    bool WDE  = getRegBitsBool( newWDTCSR, m_WDE );
    bool WDCE = getRegBitsBool( newWDTCSR, m_WDCE );

    if( WDCE && WDE )     // Allow WDP & WDE changes for next 4 cycles
    {
        m_allowChanges = true;
        Simulator::self()->cancelEvents( this );
        Simulator::self()->addEvent( 4*m_mcu->simCycPI(), this );
    }
    else if( m_allowChanges && !WDCE ) // WDP & WDE changes allowed
    {
        m_ovfReset   = WDE;
        m_prescaler  = getRegBitsVal( newWDTCSR, m_WDP02 );
        m_prescaler |= getRegBitsVal( newWDTCSR, m_WDP3 ) << 3;
        m_ovfPeriod  = m_clkPeriod/m_prescList[ m_prescaler ];
        wdtEnable();
        runEvent();
        return;
    }
    // WDP & WDE changes not allowed, keep old values
    newWDTCSR = overrideBits( newWDTCSR, m_WDE );   // Keep old WDE
    newWDTCSR = overrideBits( newWDTCSR, m_WDP02 ); // Keep old WDP
    newWDTCSR = overrideBits( newWDTCSR, m_WDP3 );  // Keep old WDP
    m_mcu->m_regOverride = newWDTCSR;

    if( m_ovfInter && !m_allowChanges ) wdtEnable();
}

void AvrWdt::wdtEnable()
{
    m_enabled = m_ovfInter || m_ovfReset;
    Simulator::self()->cancelEvents( this );
    if( m_enabled )
    {
        Simulator::self()->addEvent( m_ovfPeriod, this );
        // In Iterrupt + reset, first execute Interrupt, then reset
        m_interrupt->exitCallBack( this, m_ovfInter && m_ovfReset );
    }
}

void AvrWdt::reset()
{
    setRegBits( m_WDRF ); // MCUSR.WDRF
    Simulator::self()->cancelEvents( this );
    if( m_enabled ) Simulator::self()->addEvent( m_ovfPeriod, this );
}


