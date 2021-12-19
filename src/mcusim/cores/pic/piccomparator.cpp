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

#include "piccomparator.h"
#include "datautils.h"
#include "e_mcu.h"
#include "mcupin.h"
#include "mcuvref.h"

PicComp* PicComp::getComparator( eMcu* mcu, QString name ) // Static
{
    if     ( name.endsWith("01") ) return new PicComp01( mcu, name );
    else if( name.endsWith("02") ) return new PicComp02( mcu, name );
    else if( name.endsWith("11") ) return new PicComp11( mcu, name );
    else if( name.endsWith("12") ) return new PicComp12( mcu, name );
    else return NULL;
}

PicComp::PicComp( eMcu* mcu, QString name )
       : McuComp( mcu, name )
{
    mcu->vrefModule()->callBack( this, true ); // Vref Module will update vref if changed.
}
PicComp::~PicComp(){}

void PicComp::initialize()
{
    McuComp::initialize();
    m_enabled = false;
}

void PicComp::voltChanged()
{
    if( !m_enabled ) return;

    double vRef = m_fixVref ? m_vref : m_pinP->getVolt();
    bool compOut = vRef > m_pinN->getVolt() ;
    if( m_inv ) compOut = !compOut;

    if( m_compOut != compOut )
    {
        m_compOut = compOut;
        m_interrupt->raise();
        if( m_pinOut ) m_pinOut->setOutState( m_compOut );

        if( m_compOut ) setRegBits( m_COUT );
        else            clearRegBits( m_COUT );
    }
}

void PicComp::connect( McuPin* pinN, McuPin* pinP, McuPin* pinOut )
{
    if( pinN != m_pinN )
    {
        if( m_pinN ){
            m_pinN->setAnalog( false );
            m_pinN->changeCallBack( this, false );
        }
        if( pinN ){
            pinN->setAnalog( true );
            if( m_enabled ) pinN->changeCallBack( this, true );
        }
        m_pinN = pinN;
    }
    if( pinP != m_pinP )
    {
        if( m_pinP ){
            m_pinP->setAnalog( false );
            m_pinP->changeCallBack( this, false );
        }
        if( pinP ){
            pinP->setAnalog( true );
            if( m_enabled ) pinP->changeCallBack( this, true );
        }
        m_pinP = pinP;
    }
    if( pinOut != m_pinOut )
    {
        if( m_pinOut ) m_pinOut->controlPin( false, false );
        if( pinOut )   pinOut->controlPin( true, false );

        m_pinOut = pinOut;
    }
    if( m_pinN ) voltChanged(); // Update Comparator state
}

//-------------------------------------------------------------
// Type 0: 16f627 comparators  --------------------------------

PicComp0::PicComp0( eMcu* mcu, QString name )
        : PicComp( mcu, name )
{
    m_CM  = getRegBits( "CM0,CM1,CM2", mcu );
    m_CIS = getRegBits( "CIS", mcu );
}
PicComp0::~PicComp0(){}

void PicComp0::configureA( uint8_t newCMCON )
{
    m_cis = getRegBitsBool( newCMCON, m_CIS );
    m_inv = getRegBitsBool( newCMCON, m_CINV );

    uint8_t mode = getRegBitsVal( newCMCON, m_CM );
    if( mode != m_mode ) setMode( mode );
}

//-------------------------------------------------------------
// Type 01: 16f627 comparator 1 -------------------------------

PicComp01::PicComp01( eMcu* mcu, QString name )
         : PicComp0( mcu, name )
{
    m_CINV = getRegBits( "C1INV", mcu );
    m_COUT = getRegBits( "C1OUT", mcu );
}
PicComp01::~PicComp01(){}

void PicComp01::setMode( uint8_t mode )
{
    m_mode = mode;
    m_fixVref = false;
    m_enabled = true;

    switch ( mode ) {
        case 0: connect( m_pins[0], m_pins[1] ); m_enabled = false; break;
        case 1:{
            McuPin* pinN1 = m_cis ? m_pins[1] : m_pins[0];
            connect( pinN1,  m_pins[2] );
            } break;
        case 2:{
            m_fixVref = true;
            McuPin* pinN2 = m_cis ? m_pins[1] : m_pins[0];
            connect( pinN2, NULL );
           } break;
        case 3: connect( m_pins[0], m_pins[2] ); break;
        case 4: connect( m_pins[0], m_pins[1] ); break;
        case 5: connect( NULL, NULL ); break;
        case 6: connect( m_pins[0], m_pins[2] ); break;
        case 7: connect( NULL, NULL ); m_enabled = false; break;
    }
}

//-------------------------------------------------------------
// Type 02: 16f627 comparator 2 -------------------------------

PicComp02::PicComp02( eMcu* mcu, QString name )
         : PicComp0( mcu, name )
{
    m_CINV = getRegBits( "C2INV", mcu );
    m_COUT = getRegBits( "C2OUT", mcu );
}
PicComp02::~PicComp02(){}

void PicComp02::setMode( uint8_t mode )
{
    m_mode = mode;
    m_fixVref = false;
    m_enabled = true;

    switch ( mode ) {
        case 0: m_enabled = false;
        case 1: connect( m_pins[1], m_pins[0] ); break;
        case 2:{
            m_fixVref = true;
            McuPin* pinN = m_cis ? m_pins[1] : m_pins[0];
            connect( pinN, NULL );
            } break;
        case 3:
        case 4:
        case 5:
        case 6: connect( m_pins[1], m_pins[0] ); break;
        case 7: connect( NULL, NULL ); m_enabled = false; break;
    }
}

//-------------------------------------------------------------
// Type 1: 16f88x comparators  --------------------------------

PicComp1::PicComp1( eMcu* mcu, QString name )
        : PicComp( mcu, name )
{
    QString n = name.right(1); // name="Comp01" => n="1"

    m_CxON = getRegBits( "C"+n+"ON", mcu );
    m_COUT = getRegBits( "C"+n+"OUT", mcu );
    m_CxOE = getRegBits( "C"+n+"OE", mcu );
    m_CINV = getRegBits( "C"+n+"POL", mcu );
    m_CxR  = getRegBits( "C"+n+"R", mcu );
    m_CxCH = getRegBits( "C"+n+"CH0,C"+n+"CH1", mcu );
}
PicComp1::~PicComp1(){}

void PicComp1::configureA( uint8_t newCMxCON0 )
{
    if( newCMxCON0 == m_mode ) return;
    m_mode = newCMxCON0;

    m_enabled = getRegBitsBool( newCMxCON0, m_CxON );
    if( !m_enabled )
    {
        connect( NULL, NULL, NULL );
        return;
    }
    bool OE = getRegBitsBool( newCMxCON0, m_CxOE );
    McuPin* pinOut = OE ? m_pins[5] : NULL;

    m_inv     = getRegBitsBool( newCMxCON0, m_CINV );
    m_fixVref = getRegBitsBool( newCMxCON0, m_CxR );

    uint8_t channel = getRegBitsVal( newCMxCON0, m_CxCH );
    connect( m_pins[channel], m_pins[4], pinOut );
}

//-------------------------------------------------------------
// Type 11: 16f88x comparator 1 -------------------------------

PicComp11::PicComp11( eMcu* mcu, QString name )
         : PicComp1( mcu, name )
{

}
PicComp11::~PicComp11(){}

void PicComp11::configureC( uint8_t newSRCON )
{

}

//-------------------------------------------------------------
// Type 12: 16f88x comparator 2 -------------------------------

PicComp12::PicComp12( eMcu* mcu, QString name )
         : PicComp1( mcu, name )
{

}
PicComp12::~PicComp12(){}

void PicComp12::configureB( uint8_t newCM2CON1 )
{

}

void PicComp12::configureC( uint8_t newSRCON )
{

}
