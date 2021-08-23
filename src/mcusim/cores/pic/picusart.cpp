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

#include "picusart.h"
#include "usarttx.h"
#include "usartrx.h"
#include "e_mcu.h"
#include "iopin.h"
#include "serialmon.h"
#include "datautils.h"
#include "regwatcher.h"

PicUsart::PicUsart( eMcu* mcu,  QString name, int number )
        : McuUsart( mcu, name, number )
{
    m_stopBits = 1;
    m_dataMask = 0xFF;
    m_parity   = parNONE;

    //QString n = m_name.right(1);

    m_PIR1  = mcu->getReg( "PIR1" );
    m_RCSTA = mcu->getReg( "RCSTA" );
    m_SPBRG = mcu->getReg( "SPBRG" );

    m_SPEN = getRegBits( "SPEN", mcu );
    m_BRGH = getRegBits( "BRGH", mcu );

    m_bit9Tx = getRegBits( "TX9D", mcu );
    m_bit9Rx = getRegBits( "RX9D", mcu );

    m_txEn = getRegBits( "TXEN", mcu );
    m_rxEn = getRegBits( "CREN:", mcu );
    m_TX9  = getRegBits( "TX9", mcu );
    m_RX9  = getRegBits( "RX9", mcu );
    m_TXIF = getRegBits( "TXIF", mcu );

    watchRegNames( "SPBRG", R_WRITE, this, &PicUsart::setBaurrate, mcu );
}
PicUsart::~PicUsart(){}

void PicUsart::configureA( uint8_t newTXSTA ) // TXSTA changed
{
    // clockPol = getRegBitsVal( val, CSRC );

    bool txEn = getRegBitsBool( newTXSTA, m_txEn );
    if( txEn != m_sender->isEnabled() )
    {
        if( txEn ) setRegBits( m_TXIF );
        m_sender->enable( txEn );
        m_sender->getPin()->controlPin( txEn, false );
    }

    m_dataBits = getRegBitsVal( newTXSTA, m_TX9 )+8;

    m_speedx2 = getRegBitsVal( newTXSTA, m_BRGH ); // Double Speed?
    setBaurrate();

}

void PicUsart::configureB( uint8_t newRCSTA ) // RCSTA changed
{
    bool rxEn = getRegBitsVal( newRCSTA, m_rxEn );
    if( rxEn != m_receiver->isEnabled() )
    {
        m_receiver->getPin()->controlPin( rxEn, false );
        m_receiver->enable( rxEn );
}   }

void PicUsart::setBaurrate( uint8_t )
{
    uint64_t mult = 16;
    if( m_speedx2 ) mult = 4;
    setPeriod( mult*(*m_SPBRG+1)*m_mcu->simCycPI() ); // period in picoseconds
}

uint8_t PicUsart::getBit9()
{
    return getRegBitsVal( *m_RCSTA, m_bit9Rx );
}

void PicUsart::setBit9( uint8_t bit )
{
    if( bit ) setRegBits( m_bit9Tx );
    else      clearRegBits( m_bit9Tx );
}

void PicUsart::sendByte(  uint8_t data )
{
    if( !m_sender->isEnabled() ) return;

    if( getRegBitsVal( *m_PIR1, m_TXIF ) )  // Buffer is empty
    {
        clearRegBits( m_TXIF ); // Transmit buffer full: Clear TXIF bit
        m_sender->processData( data );
}   }

void PicUsart::frameSent( uint8_t data )
{
    if( m_monitor ) m_monitor->printOut( data );

    if( getRegBitsVal( *m_PIR1, m_TXIF ) ) return; // Buffer is empty
    m_sender->startTransmission();                 // Buffer contains data, send it
}

/*void PicUsart::readByte( uint8_t )   // UDRn is being readed
{
    m_mcu->m_regOverride = m_receiver->getData();
    //clearRegBits( m_RXC );                 // Clear RXCn flag
}*/

/*void PicUsart::setUCSRnA( uint8_t newUCSRnA )
{
    if( getRegBitsVal( newUCSRnA, m_TXC ) )
        m_mcu->m_regOverride = newUCSRnA & ~m_TXC.mask; // Clear TXCn flag
}*/
