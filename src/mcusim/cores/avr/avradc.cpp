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

AvrAdc* AvrAdc::createAdc( eMcu* mcu, QString name ,int type )
{
    switch ( type )
    {
        case 0: return new AvrAdc0( mcu, name ); break;
        case 1: return new AvrAdc1( mcu, name ); break;
        case 2: return new AvrAdc2( mcu, name ); break;
        case 3: return new AvrAdc3( mcu, name ); break;
        default: return NULL;
    }
}

AvrAdc::AvrAdc( eMcu* mcu, QString name )
      : McuAdc( mcu, name )
{
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

    m_timer0 = (AvrTimer80*)mcu->getTimer( "TIMER0" );
    m_timer1 = NULL;

    m_t0OCA = m_timer0->getOcUnit("OCA");
    m_txOCB = NULL;
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

void AvrAdc::setChannel( uint8_t newADMUX ) // ADMUX
{
    m_channel = newADMUX & 0x0F;
    m_leftAdjust = getRegBitsBool( newADMUX, m_ADLAR );

    m_refSelect = getRegBitsVal( newADMUX, m_REFS );
}

void AvrAdc::endConversion()
{
    if( m_leftAdjust ) m_adcValue <<= 6;

    clearRegBits( m_ADSC ); // Clear ADSC bit
    if( m_autoTrigger && m_freeRunning ) startConversion();
}

//------------------------------------------------------
//-- AVR ADC Type 0 ------------------------------------

AvrAdc0::AvrAdc0( eMcu* mcu, QString name )
      : AvrAdc( mcu, name )
{
    m_timer1 = (AvrTimer16bit*)mcu->getTimer("TIMER1");
    m_txOCB = m_timer1->getOcUnit("OCB");
}
AvrAdc0::~AvrAdc0(){}

void AvrAdc0::autotriggerConf()
{
    uint8_t trigger = m_trigger;
    if( !m_autoTrigger ) trigger = 255;

    m_freeRunning = trigger == 0;
    /// TODO                                     trigger == 1 // Analog Comparator
    /// TODO                                     trigger == 2 //External Interrupt Request 0
    m_t0OCA->getInterrupt()->callBack( this, trigger == 3 );  // Timer/Counter0 Compare Match A
    m_timer0->getInterrupt()->callBack( this, trigger == 4 ); // Timer/Counter0 Overflow
    m_txOCB->getInterrupt()->callBack( this, trigger == 5 );  // Timer/Counter1 Compare Match B
    m_timer1->getInterrupt()->callBack( this, trigger == 6 ); // Timer/Counter1 Overflow
    /// TODO                                     trigger == 7 // Timer/Counter1 Capture Event
}

double AvrAdc0::getVref()
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

//------------------------------------------------------
//-- AVR ADC Type 1 ------------------------------------

AvrAdc1::AvrAdc1( eMcu* mcu, QString name )
      : AvrAdc( mcu, name )
{
    m_txOCB = m_timer0->getOcUnit("OCB");
}
AvrAdc1::~AvrAdc1(){}

void AvrAdc1::autotriggerConf()
{
    uint8_t trigger = m_trigger;
    if( !m_autoTrigger ) trigger = 255;

    m_freeRunning = trigger == 0;
    /// TODO                                     trigger == 1 // Analog Comparator
    /// TODO                                     trigger == 2 //External Interrupt Request 0
    m_t0OCA->getInterrupt()->callBack( this, trigger == 3 );  // Timer/Counter0 Compare Match A
    m_timer0->getInterrupt()->callBack( this, trigger == 4 ); // Timer/Counter0 Overflow
    m_txOCB->getInterrupt()->callBack( this, trigger == 5 );  // Timer/Counter0 Compare Match B
    /// TODO                                     trigger == 6 // Pin Change Interrupt Request
}

double AvrAdc1::getVref()
{
    double vRef = 0;

    switch( m_refSelect )
    {
        case 0:     // Vcc
            vRef = 5;
            break;
        case 1:     ///TODO // Internal Vref. = ??? 1.1 Volt
            vRef = 1.1;
            break;
    }
    return vRef;
}

//------------------------------------------------------
//-- AVR ADC Type 2 ------------------------------------

AvrAdc2::AvrAdc2( eMcu* mcu, QString name )
      : AvrAdc0( mcu, name )
{
}
AvrAdc2::~AvrAdc2(){}

double AvrAdc2::getVref()
{
    double vRef = 0;

    switch( m_refSelect )
    {
        case 0:     // Vcc
            vRef = 5;
            break;
        case 1:    // External voltage reference at PA0 (AREF)
            /// TODO
            break;
        case 2:    // Internal Vref. 1.1 Volt
            vRef = 1.1;
            break;
    }
    return vRef;
}

//------------------------------------------------------
//-- AVR ADC Type 3 ------------------------------------

AvrAdc3::AvrAdc3( eMcu* mcu, QString name )
       : AvrAdc1( mcu, name )
{
}
AvrAdc3::~AvrAdc3(){}

double AvrAdc3::getVref()
{
    double vRef = 0;

    switch( m_refSelect )
    {
        case 0:     // Vcc
            vRef = 5;
            break;
        case 1:    // External voltage reference at PB0 (AREF)
            /// TODO
            break;
        case 2:    // Internal Vref. 1.1 Volt
            vRef = 1.1;
            break;
        case 3:    // Reserved
            break;
        case 4:    // Internal 2.56V Voltage Reference without external capacitor
            // fallthrough
        case 5:    // Internal 2.56V Voltage Reference with external capacitor
            vRef = 2.56;
            break;
    }
    return vRef;
}
