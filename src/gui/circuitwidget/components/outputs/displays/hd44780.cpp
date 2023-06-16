/***************************************************************************
 *   Copyright (C) 2016 by Santiago González                               *
 *                                                                         *
 ***( see copyright.txt file at root folder )*******************************/

#include <math.h>

#include "hd44780.h"
#include "itemlibrary.h"
#include "connector.h"
#include "simulator.h"
#include "iopin.h"
#include "utils.h"

#include "intprop.h"

Component* Hd44780::construct( QObject* parent, QString type, QString id )
{ return new Hd44780( parent, type, id ); }

LibraryItem* Hd44780::libraryItem()
{
    return new LibraryItem(
        tr( "Hd44780" ),
        "Displays",
        "hd44780.png",
        "Hd44780",
        Hd44780::construct );
}

Hd44780::Hd44780( QObject* parent, QString type, QString id )
       : Hd44780_Base( parent, type, id )
       , eElement( id+"-eElement" )
{
    m_pin.resize( 11 );

    m_pinRS = new IoPin( 270, QPoint( 16, 8 ), id+"-PinRS", 0, this, input );
    initPuPin( 0, "RS", m_pinRS );

    m_pinRW = new IoPin( 270, QPoint( 24, 8 ), id+"-PinRW", 0, this, input );
    initPuPin( 1, "RW", m_pinRW );

    m_pinEn = new IoPin( 270, QPoint( 32, 8 ), id+"-PinEn", 0, this, input );
    m_pinEn->setLabelText("En");
    m_pin[2] = m_pinEn;
    
    m_dataPin.resize( 8 );
    
    for( int i=0; i<8; i++ )
    {
        m_dataPin[i] = new IoPin( 270, QPoint( 40+i*8, 8), id+"-dataPin"+QString::number(i), 0, this, input );
        initPuPin( i+3, "D"+QString::number(i), m_dataPin[i] );
    }
    Simulator::self()->addToUpdateList( this );
    
    Hd44780::initialize();

    addPropGroup( { tr("Main"), {
new IntProp <Hd44780>( "Rows" ,tr("Rows")   ,"_Rows"    ,this, &Hd44780::rows, &Hd44780::setRows ,0,"uint" ),
new IntProp <Hd44780>( "Cols" ,tr("Columns"),"_Columns" ,this, &Hd44780::cols, &Hd44780::setCols ,0,"uint"  ),
    }, groupNoCopy} );
}
Hd44780::~Hd44780(){}

void Hd44780::updateStep() { update(); }

void Hd44780::initialize() { Hd44780_Base::init(); }

void Hd44780::stamp()
{
    m_pinEn->changeCallBack( this );// Register for clk changes callback
}

void Hd44780::voltChanged()             // Called when clock Pin changes 
{
    if( m_pinEn->getInpState() )                      // Clk Pin is High
    {
        m_lastClock = true;
        return; 
    }else{                                             // Clk Pin is Low
        if( m_lastClock == false ) return;         // Not a Falling edge
        m_lastClock = false;
    }
                                   // We Had  a Falling Edge: Read input
    if( m_dataLength == 8 )                                // 8 bit mode
    {
        m_input = 0;
        
        for( int pin=0; pin<8; pin++ )
            if( m_dataPin[pin]->getInpState() )
                m_input += pow( 2, pin );
    }
    else{                                                  // 4 bit mode
        if( m_nibble == 0 )                          // Read high nibble
        {
            m_input = 0;
            
            for( int pin=4; pin<8; pin++ )
                if( m_dataPin[pin]->getInpState() )
                    m_input += pow( 2, pin );
                    
            m_nibble = 1;
            return;
        }else{                                         // Read low nibble
            for( int pin=4; pin<8; pin++ )
                if( m_dataPin[pin]->getInpState() )
                    m_input += pow( 2, (pin-4) );
                    
            m_nibble = 0;
    }   }

    if( m_pinRW->getInpState() ) // Read
    {
        if( m_pinRS->getInpState() ) readData();
        else                         readBusy();
    }
    else // Write: Get RS state: data or command
    {
        if( m_pinRS->getInpState() ) writeData( m_input );
        else                         proccessCommand( m_input );
    }
}

void Hd44780::readData()
{

}

void Hd44780::readBusy()
{

}

void Hd44780::initPuPin( int n, QString l, IoPin* pin )
{
    pin->setLabelText( l );
    pin->setOutHighV( 5 );
    pin->setPullup( true );
    m_pin[n] = pin;
}

void Hd44780::showPins( bool show )
{
    m_pinRS->setVisible( show );
    m_pinRW->setVisible( show );
    m_pinEn->setVisible( show );
    
    for( int i=0; i<8; i++ ) m_dataPin[i]->setVisible( show );
}
