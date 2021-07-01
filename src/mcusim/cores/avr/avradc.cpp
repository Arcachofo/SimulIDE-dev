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

#include "avradc.h"
#include "mcupin.h"
#include "e_mcu.h"

AvrAdc::AvrAdc( eMcu* mcu, QString name )
      : McuAdc( mcu, name )
{
    m_ADIF = mcu->getRegBits( "ADIF" );
    m_ADSC = mcu->getRegBits( "ADSC" );
    m_ADCSRA = mcu->getReg( "ADCSRA" );

    m_aVccPin = mcu->getPin( "PORTV0" );
    m_aRefPin = mcu->getPin( "PORTV1" );
}

AvrAdc::~AvrAdc()
{
}

void AvrAdc::initialize()
{
    m_leftAdjust = false;
    m_autoTrigger = false;
    McuAdc::initialize();
}

void AvrAdc::configureA( uint8_t newADCSRA ) // ADCSRA
{
    if( newADCSRA & m_ADIF.mask )
          m_mcu->m_regOverride = newADCSRA & ~(m_ADIF.mask); // Clear ADIF by writting it to 1

    m_enabled = (( newADCSRA & 0b10000000 )>0);

    uint8_t prs = newADCSRA & 0b00000111;
    m_convTime = m_mcu->simCycPI()*14.5*m_prescList[prs];

    /// TODO Auto Trigger
    m_autoTrigger = (( newADCSRA & 0b00100000 )>0);

    bool convert = (( newADCSRA & 0b01000000 )>0);

    if( !m_converting && convert ) startConversion();
}

void AvrAdc::configureB( uint8_t val ) // ADCSRB
{
    uint8_t trigger = val & 0b00000111;

    switch( trigger ) /// TODO
    {
        case 0:     // Free Running mode
            break;
        case 1:     // Analog Comparator
            break;
        case 2:     // External Interrupt Request 0
            break;
        case 3:     // Timer/Counter0 Compare Match A
            break;
        case 4:     // Timer/Counter0 Overflow
            break;
        case 5:     // Timer/Counter1 Compare Match B
            break;
        case 6:     // Timer/Counter1 Overflow
            break;
        case 7:     // Timer/Counter1 Capture Event
            break;
    }
    /// if( m_autoTrigger )
}

void AvrAdc::setChannel( uint8_t val ) // ADMUX
{
    m_channel = val & 0x0F;
    m_leftAdjust = (( val & 0b00100000 )>0);

    m_refSelect = (val & 0b11000000) >> 6;
}

double AvrAdc::getVref()
{
    double vRef = 0;

    switch( m_refSelect )
    {
        case 0:     // AREF
            vRef = m_aRefPin->getVolt();
            break;
        case 1:     // AVcc
            vRef = m_aVccPin->getVolt();
            break;
        case 2:     // Reserved
            break;
        case 3:     // Internal 1.1 Volt
            vRef = 1.1;
            break;
    }
    return vRef;
}

void AvrAdc::endConversion() // Clear ADSC bit
{
    *m_ADCSRA &= ~(m_ADSC.mask);
}
