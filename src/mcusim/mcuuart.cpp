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

#include "mcuuart.h"
#include "simulator.h"
#include "mcupin.h"
#include "usartmodule.h"


#define mSTOPBITS m_usart->m_stopBits
#define mDATABITS m_usart->m_dataBits
#define mDATAMASK m_usart->m_dataMask
#define mPARITY   m_usart->m_parity

UartTR::UartTR( UsartM* usart, QString name )
      : eElement( name )
{
    m_usart = usart;

    m_state = usartSTOPPED;
}
UartTR::~UartTR( ){}

bool UartTR::getParity( uint8_t data )
{
    bool parity = false;
    for( int i=0; i<mDATABITS; ++i )
    {
        parity ^= data & 1;
        data >>= 1;
    }
    if( mPARITY == parODD ) parity ^= 1;
    return parity;
}

// TRANSMITTER -------------------------------------------------

UartTx::UartTx( UsartM* usart, QString  name )
       : UartTR( usart, name )
{
}
UartTx::~UartTx( ){}

void UartTx::enable( uint8_t en )
{
    if( en )
    {
        m_state = usartIDLE;
        m_ioPin->setState( 1 );
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
        on_dataEnd.emitValue( 0 );
        //m_usart->dataSent();
        m_state = usartIDLE;
        m_ioPin->setState( 1 );
    }
}

void UartTx::processData( uint8_t data )
{
    m_state = usartTRANSMIT;

    data &= mDATAMASK;
    m_frame = data<<1;    // Data + Start bit
    if( mDATABITS == 9 )
    {
        m_bit9  = m_usart->getBit9();
        m_frame |= m_bit9<<9;
    }
    m_framesize = mDATABITS+1;

    if( mPARITY > parNONE )                       // Parity bit
    {
        bool parity = getParity( data );

        if( parity ) m_frame |= 1<<m_framesize;
        m_framesize++;
    }
    for( int i=0; i<mSTOPBITS; ++i )     // Stop bits
    {
        m_frame |= 1<<m_framesize;
        m_framesize++;
    }
    m_currentBit = 0;
    //sendBit();                            // Start transmission
}

void UartTx::sendBit()
{
    m_ioPin->setState( m_frame & 1 );
    m_frame >>= 1;

    m_currentBit++;
    if( m_currentBit == m_framesize )        // Data transmission finished
    {
        m_state = usartTXEND;
    }
    ///Simulator::self()->addEvent( m_period, this ); // Shedule next bit
}


// RECEIVER -------------------------------------------------

UartRx::UartRx( UsartM* usart, QString name )
      : UartTR( usart, name )
{
}
UartRx::~UartRx( ){}

void UartRx::enable( uint8_t en )
{
    if( en ) processData( 0 );
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

            if( parity != parityBit )
            {
                m_usart->parityError();
                return;
            }
        }
        *m_register  = data; // Save data to Ram

        if( mDATABITS == 9 ) m_usart->setBit9( m_bit9 ); // Save Bit 9

        m_currentBit = 0;
        m_frame = 0;
        m_state = usartRECEIVE;
        on_dataEnd.emitValue( data );
    }
}

void UartRx::processData( uint8_t data )
{
    m_state = usartRECEIVE;

    m_framesize = 1+mDATABITS+mPARITY+mSTOPBITS;

    m_currentBit = 0;
    ///Simulator::self()->addEvent( m_period/2, this ); // Shedule reception
}

void UartRx::readBit()
{
    bool bit = m_ioPin->getState();

    if( (m_currentBit == 0) && bit ) return; // Wait for start bit

    if( bit ) m_frame += 1<<m_currentBit;    // Get bit into frame

    m_currentBit++;
    if( m_currentBit == m_framesize )        // Data reception finished
    {
        m_state = usartRXEND;
    }
    ///Simulator::self()->addEvent( m_period, this ); // Shedule next bit
}
