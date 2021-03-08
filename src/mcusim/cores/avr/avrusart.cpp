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
#include "mcutimer.h"
#include "e_mcu.h"

#define UCSRNB *m_ucsrnb

AvrUsart::AvrUsart( eMcu* mcu,  QString name )
        : UsartM( mcu, name )
{
    m_stopBits = 1;
    m_dataMask = 0xFF;
    m_parity   = parNONE;

    //m_timerConnected = false;
    //m_timer1 = mcu->getTimer( "TIMER1" );

    QString n = m_name.right(1);
    m_ucsrnb = mcu->getReg( "SUCSR"+n+"B" );
    m_bit9Tx = mcu->getRegBits( "TXB8"+n );
    m_bit9Rx = mcu->getRegBits( "RXB8"+n );

    m_modeRB = mcu->getRegBits( "UMSEL"+n+"0,UMSEL"+n+"1" );
    m_pariRB = mcu->getRegBits( "UPM"+n+"0,UPM"+n+"1"  );
    m_stopRB = mcu->getRegBits( "USBS"+n );
    m_dataRB = mcu->getRegBits( "UCSZ"+n+"0,UCSZ"+n+"1" );
}
AvrUsart::~AvrUsart(){}

void AvrUsart::configure( uint8_t val )
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

void AvrUsart::step( uint8_t )
{
    m_sender.runEvent();
    m_receiver.runEvent();
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

