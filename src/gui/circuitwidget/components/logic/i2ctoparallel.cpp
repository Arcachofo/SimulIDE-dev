/***************************************************************************
 *   Copyright (C) 2018 by Santiago González                               *
 *                                                                         *
 ***( see copyright.txt file at root folder )*******************************/

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
         "Converters" ,
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
            "IL01SDA",//type: Input, side: Left, pos: 01, label: "SDA"
            "IL02SCL",
            "IL06A0",
            "IL07A1",
            "IL08A2",
            "IL04INT"
        });

    setNumOuts( 8,"D" );

    m_inPin[0]->setPinMode( openCo );
    TwiModule::setSdaPin( m_inPin[0] );
    
    m_inPin[1]->setPinMode( openCo );
    TwiModule::setSclPin( m_inPin[1] );

    m_int = m_inPin[5];
    m_int->setPinMode( openCo );

    for( int i=0; i<8; ++i )
    {
        m_outPin[i]->setPinMode( openCo );
        m_outPin[i]->setPullup( true );
    }
    m_cCode = 0b01010000; // 0x50, 80

    addPropGroup( { tr("Main"), {
new IntProp <I2CToParallel>("Control_Code", tr("Control_Code") ,""    , this, &I2CToParallel::cCode,   &I2CToParallel::setCcode,0,"uint" ),
new DoubProp<I2CToParallel>("Frequency"   , tr("I2C Frequency"),"_KHz", this, &I2CToParallel::freqKHz, &I2CToParallel::setFreqKHz ),
    }, groupNoCopy} );
}
I2CToParallel::~I2CToParallel(){}

void I2CToParallel::stamp()             // Called at Simulation Start
{
    TwiModule::initialize();
    IoComponent::initState();

    for( int i=0; i<8; ++i )
    {
        m_outPin[i]->setOutState( true );
        m_outPin[i]->changeCallBack( this, true );
    }
    m_int->setOutState( true );
    m_portState = 0xFF;

    TwiModule::setMode( TWI_SLAVE );
    
    for( int i=2; i<5; ++i )
    {
        m_inPin[i]->update();
        m_inPin[i]->changeCallBack( this ); // Callbacks address pins
    }
}

void I2CToParallel::voltChanged()        // Some Pin Changed State, Manage it
{
    m_address = m_cCode;

    if( m_inPin[2]->getInpState() ) m_address += 1;
    if( m_inPin[3]->getInpState() ) m_address += 2;
    if( m_inPin[4]->getInpState() ) m_address += 4;

    int value = 0;
    for( int i=0; i<8; ++i )
    { if( m_outPin[i]->getInpState() ) value += pow( 2, i ); }

    if( value != m_portState )
        m_int->setOutState( false ); // Trigger Interrupt
    else if( m_int->getInpState() == false )
        m_int->setOutState( true );                         // Reset Interrupt
    
    TwiModule::voltChanged();                               // Run I2C Engine
}

void I2CToParallel::startWrite() // Master will write
{
    for( int i=0; i<8; ++i ) m_outPin[i]->changeCallBack( this, false ); // Disable callbacks from Port while master writes
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
    m_int->setOutState( true ); // Interrupt reset
    m_portState = m_rxReg;      // Load reference value for reset

    for( int i=0; i<8; ++i ) m_outPin[i]->changeCallBack( this, true ); // Enable callbacks from Port

    TwiModule::readByte();
}

void I2CToParallel::writeByte()   // Writting to I2C from Parallel (master is reading)
{
    int value = 0;
    for( int i=0; i<8; ++i )
    { if( m_outPin[i]->getInpState() ) value += pow( 2, i ); }
    m_txReg = value;

    m_int->setOutState( true ); // Interrupt reset
    m_portState = m_txReg;      // Load reference value for reset

    TwiModule::writeByte();
}
