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
#include "datautils.h"

AvrAdc* AvrAdc::createAdc( eMcu* mcu, QString name )
{
    int type = name.right(2).toInt();
    switch ( type ){
        case 00: return new AvrAdc00( mcu, name ); break;
        case 01: return new AvrAdc01( mcu, name ); break;
        case 02: return new AvrAdc02( mcu, name ); break;
        case 10: return new AvrAdc10( mcu, name ); break;
        case 11: return new AvrAdc11( mcu, name ); break;
        default: return NULL;
}   }

AvrAdc::AvrAdc( eMcu* mcu, QString name )
      : McuAdc( mcu, name )
{
    m_ADEN  = getRegBits( "ADEN", mcu );
    m_ADSC  = getRegBits( "ADSC", mcu );
    m_ADATE = getRegBits( "ADATE", mcu );
    m_ADIF  = getRegBits( "ADIF", mcu );
    m_ADPS  = getRegBits( "ADPS0,ADPS1,ADPS2", mcu );

    m_ADTS  = getRegBits( "ADTS0,ADTS1,ADTS2", mcu );

    m_ADLAR = getRegBits( "ADLAR", mcu );
    m_REFS  = getRegBits( "REFS0,REFS1", mcu );

    m_aVccPin = mcu->getPin( "PORTV0" );
    m_aRefPin = mcu->getPin( "PORTV1" );

    m_timer0 = (AvrTimer800*)mcu->getTimer( "TIMER0" );
    m_timer1 = NULL;

    m_t0OCA = m_timer0->getOcUnit("OCA");
    m_txOCB = NULL;

    m_vRefN = 0;
}
AvrAdc::~AvrAdc(){}

void AvrAdc::initialize()
{
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
    m_refSelect  = getRegBitsVal(  newADMUX, m_REFS );
}

void AvrAdc::endConversion()
{
    clearRegBits( m_ADSC ); // Clear ADSC bit
    if( m_autoTrigger && m_freeRunning ) startConversion();
}

//------------------------------------------------------
//-- AVR ADC Type 0 ------------------------------------

AvrAdc00::AvrAdc00( eMcu* mcu, QString name )
        : AvrAdc( mcu, name )
{
    m_timer1 = (AvrTimer16bit*)mcu->getTimer("TIMER1");
    m_txOCB  = m_timer1->getOcUnit("OCB");

    m_fixedVref = 1.1;
}
AvrAdc00::~AvrAdc00(){}

void AvrAdc00::autotriggerConf()
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

void AvrAdc00::updtVref()
{
    m_vRefP = 5;
    switch( m_refSelect ){
        case 0: m_vRefP = m_aRefPin->getVolt(); break; // AREF
        case 1: m_vRefP = m_aVccPin->getVolt(); break; // AVcc
        case 3: m_vRefP = m_fixedVref;                 // Internal 1.1 Volt
}   }

//------------------------------------------------------
//-- AVR ADC Type 01 -----------------------------------

AvrAdc01::AvrAdc01( eMcu* mcu, QString name )
        : AvrAdc00( mcu, name )
{
    m_ADATE = getRegBits( "ADFR", mcu ); // Same bit, different name: Autotrigger

    m_fixedVref = 2.6;
}
AvrAdc01::~AvrAdc01(){}

void AvrAdc01::autotriggerConf()
{
    m_freeRunning = m_autoTrigger;
}


//------------------------------------------------------
//-- AVR ADC Type 10 -----------------------------------

AvrAdc10::AvrAdc10( eMcu* mcu, QString name )
        : AvrAdc( mcu, name )
{
    m_txOCB = m_timer0->getOcUnit("OCB");
}
AvrAdc10::~AvrAdc10(){}

void AvrAdc10::autotriggerConf()
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

void AvrAdc10::updtVref()
{
    if( m_refSelect == 1 ) m_vRefP = 1.1;///TODO // Internal Vref. = ??? 1.1 Volt
    else                   m_vRefP = 5;
}

//------------------------------------------------------
//-- AVR ADC Type 02 -----------------------------------

AvrAdc02::AvrAdc02( eMcu* mcu, QString name )
        : AvrAdc00( mcu, name )
{
    m_pRefPin = m_refPin.at(0);
}
AvrAdc02::~AvrAdc02(){}

void AvrAdc02::updtVref()
{
    m_vRefP = 5;
    switch( m_refSelect ){
        case 1: m_vRefP = m_pRefPin->getVolt();break; // External voltage reference at PA0 (AREF)
        case 2: m_vRefP = 1.1;                 break; // Internal Vref. 1.1 Volt
}   }

//------------------------------------------------------
//-- AVR ADC Type 11 -----------------------------------

AvrAdc11::AvrAdc11( eMcu* mcu, QString name )
        : AvrAdc10( mcu, name )
{
    m_pRefPin = m_refPin.at(0);
}
AvrAdc11::~AvrAdc11(){}

void AvrAdc11::updtVref()
{
    m_vRefP = 5;
    switch( m_refSelect ){
        case 1: m_vRefP = m_pRefPin->getVolt(); break; // External voltage reference at PB0 (AREF)
        case 2: m_vRefP = 1.1;  break;  // Internal Vref. 1.1 Volt
        case 4:                         // Internal 2.56V Voltage Reference without external capacitor
        case 5: m_vRefP = 2.56; break;  // Internal 2.56V Voltage Reference with external capacitor
}   }

