/***************************************************************************
 *   Copyright (C) 2020 by santiago González                               *
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

#include "i51usart.h"
#include "mcutimer.h"
#include "e_mcu.h"

#define SCON *m_scon

I51Usart::I51Usart( eMcu* mcu,  QString name )
        : UsartM( mcu, name )
{
    m_stopBits = 1;
    m_dataMask = 0xFF;
    m_parity   = parNONE;

    m_timerConnected = false;
    m_timer1 = mcu->getTimer( "TIMER1" );

    m_scon = mcu->getReg( "SCON" );
    m_bit9Tx = mcu->getRegBits( "TB8" );
    m_bit9Rx = mcu->getRegBits( "RB8" );
}
I51Usart::~I51Usart(){}

void I51Usart::configure( uint8_t val )
{
    m_sender.enable( true );

    uint8_t sm0 = val & 0b10000000;
    uint8_t sm1 = val & 0b01000000;

    if( sm1 ) // Modes 1 and 3
    {
        if( !m_timerConnected )
        {
            m_timerConnected = true;
            m_timer1->on_tov.connect( this, &I51Usart::step );
            setPeriod( 0 );
        }
    }
    if( sm0 )  // modes 2 and 3
    {
        m_dataBits = 9;

        if( !sm0 ) // Mode 2
        {
            /// setPeriod(  m_mcu->simCycPI() );// Fixed baudrate 32 or 64
        }
        m_timerConnected = false;
    }
    else // Modes 0 and 1
    {
        m_dataBits = 8;

        /// if( !sm1 ) // Mode 0
        ///    setPeriod(  m_mcu->simCycPI() ); // Cycles in 1 machine cycle

        m_timerConnected = false;
    }
}

void I51Usart::step( uint8_t )
{
    m_sender.runEvent();
    m_receiver.runEvent();
}

uint8_t I51Usart::getBit9()
{
    return getRegBitsVal( SCON, m_bit9Tx );
}

void I51Usart::setBit9( uint8_t bit )
{
    SCON &= ~m_bit9Rx.mask;
    if( bit ) SCON |= m_bit9Rx.mask;
}

