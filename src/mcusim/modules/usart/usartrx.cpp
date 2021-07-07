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

#include "usartrx.h"
#include "mcuinterrupts.h"
#include "iopin.h"
#include "simulator.h"

UartRx::UartRx( UsartModule* usart, QString name )
      : UartTR( usart, name )
{
    m_period = 0;
}
UartRx::~UartRx( ){}

void UartRx::enable( uint8_t en )
{
    bool enabled = en > 0;
    if( enabled == m_enabled ) return;
    m_enabled = enabled;

    if( enabled ) processData( 0 );
    else m_state = usartSTOPPED;
    m_frame = 0;
}

void UartRx::runEvent()
{
    if( m_state == usartSTOPPED ) return;

    if( m_state == usartRECEIVE )
    {
        readBit();
    }
    else if( m_state == usartRXEND )
    {
        m_frame >>= 1;  // Start bit
        uint8_t data = m_frame & mDATAMASK;

        if( mDATABITS == 9 ) m_bit9 = (m_frame>>8) & 1;

        if( mPARITY > parNONE )        // Check Parity bit
        {
            bool parity = getParity( data );
            bool parityBit = m_frame |= 1<<m_framesize;

            if( parity != parityBit ) { m_usart->parityError(); return; }
        }
        m_usart->byteReceived( data );

        if( mDATABITS == 9 ) m_usart->setBit9( m_bit9 ); // Save Bit 9

        m_currentBit = 0;
        m_frame = 0;
        m_state = usartRECEIVE;
        //on_dataEnd.emitValue( data );
        m_interrupt->raise( data );
    }
    if( m_period )
        Simulator::self()->addEvent( m_period, this ); // Shedule next sample
}

void UartRx::processData( uint8_t data )
{
    m_state = usartRECEIVE;

    m_framesize = 1+mDATABITS+mPARITY+mSTOPBITS;

    m_currentBit = 0;
    if( m_period )
        Simulator::self()->addEvent( m_period/2, this ); // Shedule reception
}

void UartRx::readBit()
{
    bool bit = m_ioPin->getInpState();

    if( (m_currentBit == 0) && bit ) return; // Wait for start bit
    if( bit ) m_frame += 1<<m_currentBit;    // Get bit into frame

    if( ++m_currentBit == m_framesize ) m_state = usartRXEND;  // Data reception finished
}


