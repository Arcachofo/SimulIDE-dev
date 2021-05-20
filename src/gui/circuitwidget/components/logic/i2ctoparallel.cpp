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

#include "i2ctoparallel.h"
#include "itemlibrary.h"
#include "iopin.h"

Component* I2CToParallel::construct( QObject* parent, QString type, QString id )
{
    return new I2CToParallel( parent, type, id );
}

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
    
    QStringList pinList;                              // Create Pin List

    pinList // Inputs:
            << "IL02 SDA"//type: Input, side: Left, pos: 01, label: "SDA"
            << "IL03 SCL"
            << "IL05 A0"
            << "IL06 A1"
            << "IL07 A2"
            ;
    init( pinList );                   // Create Pins Defined in pinList

    setNumOuts( 8, "D" );

    m_inPin[0]->setPinMode( open_col );
    TwiModule::setSdaPin( m_inPin[0] );
    
    m_inPin[1]->setPinMode( open_col );
    TwiModule::setSclPin( m_inPin[1] );

    for( int i=0; i<8; ++i ) m_outPin[i]->setPinMode( open_col );

    m_cCode = 0b01010000;
}
I2CToParallel::~I2CToParallel(){}

QList<propGroup_t> I2CToParallel::propGroups()
{
    propGroup_t mainGroup { tr("Main") };
    mainGroup.propList.append( {"Control_Code", tr("Control_Code"),""} );
    mainGroup.propList.append( {"Frequency", tr("I2C Frequency"),"KHz"} );

    QList<propGroup_t> pg = IoComponent::propGroups();
    pg.prepend( mainGroup );
    return pg;
}

void I2CToParallel::initialize()
{
    TwiModule::initialize();
    IoComponent::initState();
    for( int i=0; i<8; ++i ) m_outPin[i]->setOutState( true );
}

void I2CToParallel::stamp()                     // Called at Simulation Start
{
    TwiModule::stamp();
    
    for( int i=2; i<5; ++i )                  // Initialize address pins
        m_inPin[i]->changeCallBack( this );
}

void I2CToParallel::voltChanged()             // Some Pin Changed State, Manage it
{
    bool A0 = m_inPin[1]->getInpState();
    bool A1 = m_inPin[2]->getInpState();
    bool A2 = m_inPin[3]->getInpState();
    
    int  address = m_cCode;
    if( A0 ) address += 1;
    if( A1 ) address += 2;
    if( A2 ) address += 4;
    
    m_address = address;
    
    TwiModule::voltChanged();                               // Run I2C Engine
}

void I2CToParallel::readByte()           // Reading from I2C, Writting to Parallel
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

void I2CToParallel::writeByte()         // Writting to I2C from Parallel (master is reading)
{
    int value = 0;
    for( int i=0; i<8; ++i )
    {
        if( m_outPin[i]->getInpState() ) value += pow( 2, i );
    }
    m_txReg = value;

    TwiModule::writeByte();
}

int I2CToParallel::cCode()
{
    return m_cCode;
}

void I2CToParallel::setCcode( int code )
{
    m_cCode = code;
}

#include "moc_i2ctoparallel.cpp"
