/***************************************************************************
 *   Copyright (C) 2018 by santiago González                               *
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

#include <math.h>

#include "i2ctoparallel.h"
#include "itemlibrary.h"
#include "iopin.h"

#include "doubleprop.h"
#include "intprop.h"

Component* I2CToParallel::construct( QObject* parent, QString type, QString id )
{ return new I2CToParallel( parent, type, id ); }

LibraryItem* I2CToParallel::libraryItem()
{
    return new LibraryItem(
        tr( "I2C to Parallel" ),
        tr( "Logic/Converters" ),
        "2to3g.png",
        "I2CToParallel",
        I2CToParallel::construct );
}

I2CToParallel::I2CToParallel( QObject* parent, QString type, QString id )
             : IoComponent( parent, type, id )
             , TwiModule( id )
{
    m_width  = 4;
    m_height = 9;
    
    init({         // Inputs:
            "IL02SDA",//type: Input, side: Left, pos: 01, label: "SDA"
            "IL03SCL",
            "IL05A0",
            "IL06A1",
            "IL07A2",
        });

    setNumOuts( 8, "D" );

    m_inPin[0]->setPinMode( openCo );
    TwiModule::setSdaPin( m_inPin[0] );
    
    m_inPin[1]->setPinMode( openCo );
    TwiModule::setSclPin( m_inPin[1] );

    for( int i=0; i<8; ++i )
    {
        m_outPin[i]->setPinMode( openCo );
        m_outPin[i]->setPullup( true );
    }
    m_cCode = 0b01010000; // 0x50, 80

    addPropGroup( { tr("Main"), {
new IntProp <I2CToParallel>( "Control_Code", tr("Control_Code") ,""    , this, &I2CToParallel::cCode,   &I2CToParallel::setCcode, "uint" ),
new DoubProp<I2CToParallel>( "Frequency"   , tr("I2C Frequency"),"_KHz", this, &I2CToParallel::freqKHz, &I2CToParallel::setFreqKHz ),
    }} );
}
I2CToParallel::~I2CToParallel(){}

/*void I2CToParallel::initialize()
{
    TwiModule::initialize();
    IoComponent::initState();
    for( int i=0; i<8; ++i ) m_outPin[i]->setOutState( true );
}*/

void I2CToParallel::stamp()                     // Called at Simulation Start
{
    TwiModule::initialize();
    IoComponent::initState();
    for( int i=0; i<8; ++i ) m_outPin[i]->setOutState( true );

    TwiModule::setMode( TWI_SLAVE );
    
    for( int i=2; i<5; ++i ) m_inPin[i]->changeCallBack( this ); // Callbacks address pins
}

void I2CToParallel::voltChanged()             // Some Pin Changed State, Manage it
{
    m_address = m_cCode;

    if( m_inPin[2]->getInpState() ) m_address += 1;
    if( m_inPin[3]->getInpState() ) m_address += 2;
    if( m_inPin[4]->getInpState() ) m_address += 4;
    
    TwiModule::voltChanged();                               // Run I2C Engine
}

void I2CToParallel::readByte()  // Reading from I2C, Writting to Parallel
{
    int value = m_rxReg;
                              //qDebug() << "Reading " << value;
    for( int i=0; i<8; ++i )
    {
        m_outPin[i]->setOutState( value & 1 );
        value >>= 1;
    }
    TwiModule::readByte();
}

void I2CToParallel::writeByte()   // Writting to I2C from Parallel (master is reading)
{
    int value = 0;
    for( int i=0; i<8; ++i )
    { if( m_outPin[i]->getInpState() ) value += pow( 2, i ); }
    m_txReg = value;

    TwiModule::writeByte();
}
