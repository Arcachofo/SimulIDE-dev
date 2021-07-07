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
//#include "mcutimer.h"
#include "e_mcu.h"

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
    m_u2xn   = mcu->getRegBits( "U2X"+n );

    m_bit9Tx = mcu->getRegBits( "TXB8"+n );
    m_bit9Rx = mcu->getRegBits( "RXB8"+n );

    m_modeRB = mcu->getRegBits( "UMSEL"+n+"0,UMSEL"+n+"1" );
    m_pariRB = mcu->getRegBits( "UPM"+n+"0,UPM"+n+"1"  );
    m_stopRB = mcu->getRegBits( "USBS"+n );
    m_dataRB = mcu->getRegBits( "UCSZ"+n+"0,UCSZ"+n+"1" );

    m_ubrrnL = mcu->getReg( "UBRR"+n+"L" );
    m_ubrrnH = mcu->getReg( "UBRR"+n+"H" );
    m_mcu->watchRegNames( "UBRR"+n+"L", R_WRITE, this, &AvrUsart::setUBRRnL );
    m_mcu->watchRegNames( "UBRR"+n+"H", R_WRITE, this, &AvrUsart::setUBRRnH );
}
AvrUsart::~AvrUsart(){}

void AvrUsart::configureA( uint8_t val ) // UCSRnC changed
{
    // clockPol = getRegBitsVal( val, UCPOLn );

    m_mode = getRegBitsVal( val, m_modeRB );        // UMSELn1, UMSELn0
    m_stopBits = getRegBitsVal( val, m_stopRB )+1;  // UPMn1, UPMno
    m_dataBits = getRegBitsVal( val, m_dataRB )+5;

    uint8_t par = getRegBitsVal( val, m_pariRB );
    if( par > 0 ) m_parity = (parity_t)(par-1);
    else          m_parity = parNONE;

    /*if( sm0 )  // modes 2 and 3
    {
        if( !sm0 ) // Mode 2
        {
            /// setPeriod(  m_mcu->simCycPI() );// Fixed baudrate 32 or 64
        }
    }*/
}

void AvrUsart::setUBRRnL( uint8_t val )
{
    setBaurrate( val |( (*m_ubrrnH & 0x0F)<<8 ) );
}

void AvrUsart::setUBRRnH( uint8_t val )
{
    setBaurrate( *m_ubrrnL |( (val & 0x0F)<<8 ) );
}

void AvrUsart::setBaurrate( uint16_t ubrr )
{
    uint8_t doub = getRegBitsVal( *m_ucsrna, m_u2xn ); // Double Speed?
    if( doub ) ubrr /= 2;
    setPeriod( 1e6*16*(ubrr+1) ); // period in picoseconds
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

