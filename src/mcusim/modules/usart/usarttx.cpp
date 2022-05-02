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

#include "usarttx.h"
#include "mcuinterrupts.h"
#include "iopin.h"
#include "simulator.h"

UartTx::UartTx( UsartModule* usart, eMcu* mcu, QString  name )
      : UartTR( usart, mcu, name )
{
    m_period = 0;
}
UartTx::~UartTx( ){}

void UartTx::enable( uint8_t en )
{
    bool enabled = en > 0;
    if( enabled == m_enabled ) return;
    m_enabled = enabled;
    m_runHardware = m_ioPin->isConnected();//   ->connector();

    if( enabled ){
        m_state = usartIDLE;
        m_ioPin->sheduleState( true );
    }
    else m_state = usartSTOPPED;
}

void UartTx::runEvent()
{
    if( m_state == usartSTOPPED ) return;
    if( m_state == usartTRANSMIT )
    {
        sendBit();
    }
    else if( m_state == usartTXEND )
    {
        m_state = usartIDLE;
        m_ioPin->sheduleState( true );
        m_usart->frameSent( m_data );
        /// m_interrupt->raise();
}   }

void UartTx::processData( uint8_t data )
{
    m_buffer = data;
    if( m_enabled && m_state == usartIDLE ) startTransmission();
}

void UartTx::startTransmission() // Data loaded to ShiftReg
{
    m_usart->bufferEmpty();
    m_data = m_buffer;

    m_state = usartTRANSMIT;

    uint8_t data = m_data & mDATAMASK;
    m_frame = data<<1;                   // Data + Start bit

    if( mDATABITS == 9 )
    {
        m_bit9  = m_usart->getBit9Tx();
        m_frame |= m_bit9<<9;
    }
    m_framesize = mDATABITS+1;

    if( mPARITY > parNONE )              // Parity bit
    {
        bool parity = getParity( data );

        if( parity ) m_frame |= 1<<m_framesize;
        m_framesize++;
    }
    for( int i=0; i<mSTOPBITS; ++i )    // Stop bits
    {
        m_frame |= 1<<m_framesize;
        m_framesize++;
    }
    m_currentBit = 0;
    if( m_period )
    {
        if( m_runHardware ) sendBit(); // Start transmission
        else{                          // Not running Hardwware
            m_state = usartTXEND;      // Shedule End of transmission
            Simulator::self()->addEvent( m_period*m_framesize, this );
}   }   }

void UartTx::sendBit()
{
    m_ioPin->sheduleState( m_frame & 1 );
    m_frame >>= 1;

    m_currentBit++;
    if( m_currentBit == m_framesize ) // Data transmission finished
    {
        m_state = usartTXEND;
    }
    if( m_period )
        Simulator::self()->addEvent( m_period, this ); // Shedule next bit
}

