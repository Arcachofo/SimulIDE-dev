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

#include "picadc.h"
#include "mcupin.h"
#include "e_mcu.h"
#include "datautils.h"

PicAdc* PicAdc::createAdc( eMcu* mcu, QString name )
{
    /*switch ( type )
    {
        case 0: return new PicAdc0( mcu, name ); break;
        //case 1: return new PicAdc1( mcu, name ); break;
        //case 2: return new PicAdc2( mcu, name ); break;
        //case 3: return new PicAdc3( mcu, name ); break;
        default: return NULL;
    }*/
    return new PicAdc0( mcu, name );
    //return NULL;
}

PicAdc::PicAdc( eMcu* mcu, QString name )
      : McuAdc( mcu, name )
{
    m_ADON = getRegBits( "ADON", mcu );
    m_ADSC = getRegBits( "ADSC0,ADCS1", mcu );
    m_CHS  = getRegBits( "CH0,CH1,CH2", mcu );
    m_GODO = getRegBits( "GODO", mcu );

    m_PCFG = getRegBits( "PCFG0,PCFG1,PCFG2,PCFG3", mcu );
    m_ADFM = getRegBits( "ADFM", mcu );

    m_pRefPin = NULL;
    m_nRefPin = NULL;
}
PicAdc::~PicAdc(){}

void PicAdc::initialize()
{
    McuAdc::initialize();
}

void PicAdc::configureA(uint8_t newADCON0 ) // ADCON0
{
    m_enabled = getRegBitsBool( newADCON0, m_ADON );

    uint8_t prs = getRegBitsVal( newADCON0, m_ADSC );
    if( prs == 3 ) m_convTime = 4*12*1e6;
    else           m_convTime = m_mcu->simCycPI()*12*m_prescList[prs];

    m_channel = getRegBitsVal( newADCON0, m_CHS );

    bool convert = getRegBitsBool( newADCON0, m_GODO );
    if( !m_converting && convert ) startConversion();
}

void PicAdc::configureB( uint8_t newADCON1 ) // ADCON1
{
    m_leftAdjust = !getRegBitsBool( newADCON1, m_ADFM );

    uint8_t mode = getRegBitsVal( newADCON1, m_PCFG );
    if( mode != m_mode )
    {
        m_mode = mode;

        switch( mode ) {
            case 0:
                break;
            case 1:
                break;
            case 2:
                break;
            case 3:
                break;
            case 4:
                break;
            case 5:
                break;
            case 6:    // All Pins Digital
            case 7:
                break;
            case 8:
                break;
            case 9:
                break;
            case 10:
                break;
            case 11:
                break;
            case 12:
                break;
            case 13:
                break;
            case 14:
                break;
            case 15:
                break;
            default:
                break;
        }
    }
}

void PicAdc::endConversion()
{
    if( m_leftAdjust ) m_adcValue <<= 6;
    clearRegBits( m_GODO ); // Clear GO/DONE bit
}

//------------------------------------------------------
//-- AVR ADC Type 0 ------------------------------------

PicAdc0::PicAdc0( eMcu* mcu, QString name )
       : PicAdc( mcu, name )
{
    m_pRefPin = mcu->getPin( "PORTA3" );
    m_nRefPin = mcu->getPin( "PORTA2" );
}
PicAdc0::~PicAdc0(){}

double PicAdc0::getVref()
{
    double vRef = 0;

    switch( m_mode ) {
        case 1:
        case 3:
        case 5:
            vRef = m_pRefPin->getVolt();
            break;
        case 8:
            vRef = m_pRefPin->getVolt()-m_nRefPin->getVolt();
            break;
        case 10:
            vRef = m_pRefPin->getVolt();
            break;
        case 11:
        case 12:
        case 13:
            vRef = m_pRefPin->getVolt()-m_nRefPin->getVolt();
            break;
        case 15:
            vRef = m_pRefPin->getVolt();
            break;
        default: vRef = 5;
            break;
    }
    return vRef;
}

//------------------------------------------------------
//-- AVR ADC Type 1 ------------------------------------

PicAdc1::PicAdc1( eMcu* mcu, QString name )
       : PicAdc( mcu, name )
{
}
PicAdc1::~PicAdc1(){}

double PicAdc1::getVref()
{
    double vRef = 0;

    return vRef;
}

//------------------------------------------------------
//-- AVR ADC Type 2 ------------------------------------

PicAdc2::PicAdc2( eMcu* mcu, QString name )
      : PicAdc0( mcu, name )
{
}
PicAdc2::~PicAdc2(){}

double PicAdc2::getVref()
{
    double vRef = 0;

    return vRef;
}

//------------------------------------------------------
//-- AVR ADC Type 3 ------------------------------------

PicAdc3::PicAdc3( eMcu* mcu, QString name )
       : PicAdc1( mcu, name )
{
}
PicAdc3::~PicAdc3(){}

double PicAdc3::getVref()
{
    double vRef = 0;

    return vRef;
}
