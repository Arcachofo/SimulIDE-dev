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
#include "e-source.h"
#include "pin.h"

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
             : LogicComponent( parent, type, id )
             , eI2CSlave( id )
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
            // Outputs:
            << "OR01 D0"
            << "OR02 D1"
            << "OR03 D2"
            << "OR04 D3"
            << "OR05 D4"
            << "OR06 D5"
            << "OR07 D6"
            << "OR08 D7"
            ;
    init( pinList );                   // Create Pins Defined in pinList
    
    eLogicDevice::createInput( m_inPin[0] );                // Input SDA
    eLogicDevice::createClockPin( m_inPin[1] );             // Input SCL
    
    eLogicDevice::createInput( m_inPin[2] );                 // Input A0
    eLogicDevice::createInput( m_inPin[3] );                 // Input A1
    eLogicDevice::createInput( m_inPin[4] );                 // Input A2
    
    for( int i=0; i<8; ++i ) eLogicDevice::createOutput( m_outPin[i] );

    m_cCode = 0b01010000;
}
I2CToParallel::~I2CToParallel(){}

QList<propGroup_t> I2CToParallel::propGroups()
{
    propGroup_t mainGroup { tr("Main") };
    mainGroup.propList.append( {"Control_Code", tr("Control_Code"),""} );
    mainGroup.propList.append( {"Frequency", tr("I2C Frequency"),"KHz"} );

    QList<propGroup_t> pg = LogicComponent::propGroups();
    pg.prepend( mainGroup );
    return pg;
}

void I2CToParallel::stamp()                     // Called at Simulation Start
{
    eI2CSlave::stamp();
    
    for( int i=2; i<5; ++i )                  // Initialize address pins
    {
        eNode* enode =  m_inPin[i]->getEnode();
        if( enode ) enode->voltChangedCallback( this );
    }
    for( int i=0; i<8; ++i )
    {
        m_output[i]->setState( true );
        m_output[i]->setImp( 1e5 );
    }
}

void I2CToParallel::voltChanged()             // Some Pin Changed State, Manage it
{
    bool A0 = eLogicDevice::getInputState( 1 );
    bool A1 = eLogicDevice::getInputState( 2 );
    bool A2 = eLogicDevice::getInputState( 3 );
    
    int  address = m_cCode;
    if( A0 ) address += 1;
    if( A1 ) address += 2;
    if( A2 ) address += 4;
    
    m_address = address;
    
    eI2CSlave::voltChanged();                               // Run I2C Engine
}

void I2CToParallel::readByte()           // Reading from I2C, Writting to Parallel
{
    int value = m_rxReg;
                                      //qDebug() << "Reading " << value;
    for( int i=0; i<8; ++i )
    {
        bool pinState =  value & 1;
        double imp = pinState? 1e5 : 40;

        m_output[i]->setState( pinState );
        m_output[i]->setImp( imp );
                                  //qDebug() << "Bit " << i << pinState;
        value >>= 1;
    }
    eI2CSlave::readByte();
}

void I2CToParallel::writeByte()         // Writting to I2C from Parallel (master is reading)
{
    int value = 0;
    for( int i=0; i<8; ++i )
    {
        double volt = m_output[i]->getVolt();
        
        bool state = false;// = m_dataPinState[i];
        
        if     ( volt > m_inputHighV ) state = true;
        //else if( volt < m_inputLowV )  state = false;
        
        //m_dataPinState[i] = state;
        //qDebug() << "Bit " << i << state;
        if( state ) value += pow( 2, i );
    }
    m_txReg = value;
    //qDebug() << "I2CToParallel::writeByte Address:"<<" Value"<< m_txReg;

    eI2CSlave::writeByte();
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
