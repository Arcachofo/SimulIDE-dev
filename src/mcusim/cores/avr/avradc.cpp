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
#include "avrtimer.h"
#include "mcuocunit.h"
#include "mcupin.h"
#include "e_mcu.h"

AvrAdc::AvrAdc( eMcu* mcu, QString name, int type )
      : McuAdc( mcu, name )
{
    m_type = type;

    //m_ADCSRA = mcu->getReg( "ADCSRA" );
    m_ADEN  = mcu->getRegBits( "ADEN" );
    m_ADSC  = mcu->getRegBits( "ADSC" );
    m_ADATE = mcu->getRegBits( "ADATE" );
    m_ADIF  = mcu->getRegBits( "ADIF" );
    m_ADPS  = mcu->getRegBits( "ADPS0,ADPS1.ADPS2" );

    m_ADTS  = mcu->getRegBits( "ADTS0,ADTS1.ADTS2" );

    m_ADLAR = mcu->getRegBits( "ADLAR" );
    m_REFS  = mcu->getRegBits( "REFS0,REFS1" );

    m_aVccPin = mcu->getPin( "PORTV0" );
    m_aRefPin = mcu->getPin( "PORTV1" );

    m_timer0 = (AvrTimer0*)mcu->getTimer( "TIMER0" );
    m_timer1 = (AvrTimer16bit*)mcu->getTimer( "TIMER1" );

    m_t0OCA = m_timer0->getOcUnit("OCA");
    if     ( type == 0 ) m_tOCB = m_timer1->getOcUnit("OCB");
    else if( type == 1 ) m_tOCB = m_timer0->getOcUnit("OCB");

}
AvrAdc::~AvrAdc(){}

void AvrAdc::initialize()
{
    m_leftAdjust  = false;
    m_autoTrigger = false;
    m_freeRunning = false;

    m_trigger = 0;
    m_refSelect = 0;

    McuAdc::initialize();
}

void AvrAdc::configureA( uint8_t newADCSRA ) // ADCSRA
{
    if( newADCSRA & m_ADIF.mask )
          m_mcu->m_regOverride = newADCSRA & ~(m_ADIF.mask); // Clear ADIF by writting it to 1

    m_enabled = getRegBitsBool( newADCSRA, m_ADEN );

    uint8_t prs = getRegBitsVal( newADCSRA, m_ADPS );
    m_convTime = m_mcu->simCycPI()*14.5*m_prescList[prs];

    /// TODO Auto Trigger
    m_autoTrigger = getRegBitsBool( newADCSRA, m_ADATE );
    if( m_autoTrigger ) autotriggerConf();

    bool convert = getRegBitsBool( newADCSRA, m_ADSC );

    if( !m_converting && convert ) startConversion();
}

void AvrAdc::configureB( uint8_t newADCSRB ) // ADCSRB
{
    m_trigger = getRegBitsVal( newADCSRB, m_ADTS );
    if( m_autoTrigger ) autotriggerConf();

    /// TODO: ACME ???
}

void AvrAdc::autotriggerConf()
{
    uint8_t trigger = m_trigger;
    if( !m_autoTrigger ) trigger = 255;

    m_freeRunning = trigger == 0;
    /// TODO Analog Comparator
    m_t0OCA->getInterrupt()->callBack( this, m_trigger == 3 );
    m_timer0->getInterrupt()->callBack( this, m_trigger == 4 );
    m_tOCB->getInterrupt()->callBack( this, m_trigger == 5 );
    if( m_type == 0 )
    {
        m_timer1->getInterrupt()->callBack( this, m_trigger == 6 );
        /// TODO Timer/Counter1 Capture Event
    }


    /*switch( m_trigger ) /// TODO
    {
        case 0:     // Free Running mode
            m_freeRunning = true;
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
    }*/
}

void AvrAdc::setChannel( uint8_t newADMUX ) // ADMUX
{
    m_channel = newADMUX & 0x0F;
    m_leftAdjust = getRegBitsBool( newADMUX, m_ADLAR );

    m_refSelect = getRegBitsVal( newADMUX, m_REFS );
    if( m_type == 1 ) m_refSelect = (m_refSelect << 1) + 1; // tiny
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
            if( m_type == 1 ) vRef = 5; // tiny
            else              vRef = m_aVccPin->getVolt();
            break;
        case 2:     // Reserved
            break;
        case 3:     // Internal 1.1 Volt
            vRef = 1.1;
            break;
    }
    return vRef;
}

void AvrAdc::endConversion()
{
    if( m_leftAdjust ) m_adcValue <<= 6;

    clearRegBits( m_ADSC ); // Clear ADSC bit
    if( m_autoTrigger && m_freeRunning ) startConversion();
}

