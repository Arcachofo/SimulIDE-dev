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
#include "connector.h"

UartRx::UartRx( UsartModule* usart, eMcu* mcu, QString name )
      : UartTR( usart, mcu, name )
{
    m_period = 0;
}
UartRx::~UartRx( ){}

void UartRx::enable( uint8_t en )
{
    bool enabled = en > 0;
    if( enabled == m_enabled ) return;
    m_enabled = enabled;
    m_runHardware = m_ioPin->connector();
    while( !m_inBuffer.empty() ) m_inBuffer.pop();

    if( enabled ) processData( 0 ); // Start reading
    else m_state = usartSTOPPED;
    m_frame = 0;
}

void UartRx::runEvent()
{
    if( m_state == usartSTOPPED ) return;

    if( m_state == usartRECEIVE )
    {
        if( m_runHardware ) readBit();
        else{
            if( !m_inBuffer.empty() )
            {
                m_usart->byteReceived( m_inBuffer.front() );
                m_inBuffer.pop();
            }
            Simulator::self()->addEvent( m_period*(m_framesize ), this );
            return;
        }
    }
    else if( m_state == usartRXEND )
    {
        m_frame >>= 1;  // Start bit
        uint8_t data = m_frame & mDATAMASK;

        if( mDATABITS == 9 ) m_bit9 = (m_frame>>8) & 1;

        if( mPARITY > parNONE )        // Check Parity bit
        {
            bool parity = getParity( m_data );
            bool parityBit = m_frame |= 1<<m_framesize;

            if( parity != parityBit ) { m_usart->parityError(); return; }
        }
        if( mDATABITS == 9 ) m_usart->setBit9( m_bit9 ); // Save Bit 9

        m_state = usartRECEIVE;
        m_currentBit = 0;
        m_startHigh = false;
        m_frame = 0;

        m_usart->byteReceived( data );
        /// m_interrupt->raise( data ); implemented in bytereceived
    }
    if( m_period )
        Simulator::self()->addEvent( m_period, this ); // Shedule next sample
}

void UartRx::processData( uint8_t )
{
    m_state = usartRECEIVE;
    m_framesize = 1+mDATABITS+mPARITY+mSTOPBITS;
    m_currentBit = 0;
    m_startHigh = false;

    if( m_period )
    {
         if( m_runHardware )
            Simulator::self()->addEvent( m_period/2, this ); // Shedule reception
         else Simulator::self()->addEvent( m_period*m_framesize, this ); // Shedule Byte received
}   }

void UartRx::readBit()
{
    bool bit = m_ioPin->getInpState();

    if( (m_currentBit == 0) && bit )
    {
        m_startHigh = true;
        return; // Wait for start bit
    }
    if( !m_startHigh ) return;

    if( bit ) m_frame += 1<<m_currentBit;    // Get bit into frame

    if( ++m_currentBit == m_framesize ) m_state = usartRXEND;  // Data reception finished
}

void UartRx::queueData( uint8_t data )
{
    if( !m_enabled ) return;
    if( m_runHardware )
    {
        m_runHardware = false;
        Simulator::self()->cancelEvents( this );
        Simulator::self()->addEvent( m_period*(m_framesize+2), this );
    }
    if( m_inBuffer.size() > 1000 ) return;

    m_inBuffer.push( data );
}
