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

#include "avrusart.h"
#include "usarttx.h"
#include "usartrx.h"
#include "e_mcu.h"
#include "iopin.h"
#include "serialmon.h"
#include "datautils.h"
#include "regwatcher.h"

#define UCSRNB *m_ucsrnb

AvrUsart::AvrUsart( eMcu* mcu,  QString name, int number )
        : McuUsart( mcu, name, number )
{
    m_stopBits = 1;
    m_dataMask = 0xFF;
    m_parity   = parNONE;

    QString n = m_name.right(1);
    m_ucsrna = mcu->getReg( "UCSR"+n+"A" );
    m_ucsrnb = mcu->getReg( "UCSR"+n+"B" );
    m_u2xn   = getRegBits( "U2X"+n, mcu );

    m_bit9Tx = getRegBits( "TXB8"+n, mcu );
    m_bit9Rx = getRegBits( "RXB8"+n, mcu );

    m_txEn = getRegBits( "TXEN"+n, mcu );
    m_rxEn = getRegBits( "RXEN"+n, mcu );

    m_modeRB = getRegBits( "UMSEL"+n+"0,UMSEL"+n+"1", mcu );
    m_pariRB = getRegBits( "UPM"+n+"0,UPM"+n+"1", mcu );
    m_stopRB = getRegBits( "USBS"+n, mcu );
    m_dataRB = getRegBits( "UCSZ"+n+"0,UCSZ"+n+"1", mcu );

    m_ubrrnL = mcu->getReg( "UBRR"+n+"L" );
    m_ubrrnH = mcu->getReg( "UBRR"+n+"H" );
    watchRegNames( "UBRR"+n+"L", R_WRITE, this, &AvrUsart::setBaurrate, m_mcu );
    watchRegNames( "UBRR"+n+"H", R_WRITE, this, &AvrUsart::setBaurrate, m_mcu );

    m_UDRE = getRegBits( "UDRE"+n, mcu );
    m_TXC  = getRegBits( "TXC"+n, mcu );
    m_RXC  = getRegBits( "RXC"+n, mcu );

    watchRegNames( "UCSR"+n+"A", R_WRITE, this, &AvrUsart::setUCSRnA, mcu );
}
AvrUsart::~AvrUsart(){}

void AvrUsart::configureA( uint8_t newUCSRnC ) // UCSRnC changed
{


    // clockPol = getRegBitsVal( val, UCPOLn );

    m_mode = getRegBitsVal( newUCSRnC, m_modeRB );        // UMSELn1, UMSELn0
    m_stopBits = getRegBitsVal( newUCSRnC, m_stopRB )+1;  // UPMn1, UPMno
    m_dataBits = getRegBitsVal( newUCSRnC, m_dataRB )+5;

    uint8_t par = getRegBitsVal( newUCSRnC, m_pariRB );
    if( par > 0 ) m_parity = (parity_t)(par-1);
    else          m_parity = parNONE;

    m_speedx2 = getRegBitsVal( newUCSRnC, m_u2xn ); // Double Speed?
    setBaurrate();

    /*if( sm0 )  // modes 2 and 3
    {
        if( !sm0 ) // Mode 2
        {
            /// setPeriod(  m_mcu->simCycPI() );// Fixed baudrate 32 or 64
        }
    }*/
}

void AvrUsart::configureB( uint8_t newUCSRnB ) // UCSRnB changed
{
    uint8_t txEn = getRegBitsVal( newUCSRnB, m_txEn );
    if( txEn != m_sender->isEnabled() )
    {
        m_sender->enable( txEn );
        if( txEn )
        {
            m_sender->getPin()->controlPin( true, true );
            m_sender->getPin()->setPinMode( output );
        }
        else m_sender->getPin()->controlPin( false, false );
    }

    uint8_t rxEn = getRegBitsVal( newUCSRnB, m_rxEn );
    if( rxEn != m_receiver->isEnabled() )
    {
        m_receiver->enable( rxEn );
        if( rxEn )
        {
            m_receiver->getPin()->controlPin( true, true );
            m_receiver->getPin()->setPinMode( input );
        }
        else m_receiver->getPin()->controlPin( false, false );
}   }

void AvrUsart::setBaurrate( uint8_t )
{
    uint16_t ubrr = *m_ubrrnL | (*m_ubrrnH & 0x0F)<<8 ;
    uint64_t mult = 16;
    if( m_speedx2 ) mult = 8;
    setPeriod( mult*(ubrr+1)*m_mcu->simCycPI() ); // period in picoseconds
}

uint8_t AvrUsart::getBit9()
{
    return getRegBitsVal( UCSRNB, m_bit9Tx );
}

void AvrUsart::setBit9( uint8_t bit )
{
    UCSRNB &= ~m_bit9Rx.mask;
    if( bit ) UCSRNB |= m_bit9Rx.mask;
}

void AvrUsart::sendByte(  uint8_t data ) // Buffer is being written
{
    if( !m_sender->isEnabled() ) return;

    if( getRegBitsVal( *m_ucsrna, m_UDRE ) )  // Buffer is empty
    {
        clearRegBits( m_UDRE );         // Transmit buffer full: Clear UDREn bit
        m_sender->processData( data );
}   }

void AvrUsart::frameSent( uint8_t data )
{
    if( m_monitor ) m_monitor->printOut( data );

    if( getRegBitsVal( *m_ucsrna, m_UDRE ) ) // Buffer is empty, raise USART Transmit Complete
        m_sender->raiseInt();
    else                                     // Buffer contains data, send it
        m_sender->startTransmission();
}

void AvrUsart::readByte( uint8_t )   // UDRn is being readed
{
    m_mcu->m_regOverride = m_receiver->getData();
    clearRegBits( m_RXC );                 // Clear RXCn flag
}

void AvrUsart::setUCSRnA( uint8_t newUCSRnA )
{
    if( getRegBitsVal( newUCSRnA, m_TXC ) )
        m_mcu->m_regOverride = newUCSRnA & ~m_TXC.mask; // Clear TXCn flag
}
