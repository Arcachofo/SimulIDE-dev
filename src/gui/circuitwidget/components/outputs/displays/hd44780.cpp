/***************************************************************************
 *   Copyright (C) 2016 by santiago González                               *
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
        tr( "Displays" ),
        "hd44780.png",
        "Hd44780",
        Hd44780::construct );
}

Hd44780::Hd44780( QObject* parent, QString type, QString id )
       : Hd44780_Base( parent, type, id )
       , eElement( id+"-eElement" )
{
    m_pin.resize( 11 );
    
    int pinY = 8;//8*((33+m_imgHeight)/8);

    m_pinRS = new IoPin( 270, QPoint( 16, pinY ), id+"-PinRS", 0, this, input );
    m_pinRW = new IoPin( 270, QPoint( 24, pinY ), id+"-PinRW", 0, this, input );
    m_pinEn = new IoPin( 270, QPoint( 32, pinY ), id+"-PinEn", 0, this, input );
    m_pinRS->setLabelText(" RS");
    m_pinRW->setLabelText(" RW");
    m_pinEn->setLabelText(" En");

    m_pin[0] = m_pinRS;
    m_pin[1] = m_pinRW;
    m_pin[2] = m_pinEn;
    
    m_dataPin.resize( 8 );
    
    for( int i=0; i<8; i++ )
    {
        m_dataPin[i] = new IoPin( 270, QPoint( 40+i*8, pinY), id+"-dataPin"+QString::number(i), 0, this, input );
        m_dataPin[i]->setLabelText( " D"+QString::number(i) );
        m_pin[i+3] = m_dataPin[i];
    }
    Simulator::self()->addToUpdateList( this );
    
    initialize();

    addPropGroup( { tr("Main"), {
new IntProp <Hd44780>( "Rows" ,tr("Rows")   ,"_Lines"  ,this,&Hd44780::rows, &Hd44780::setRows ,"uint" ),
new IntProp <Hd44780>( "Cols" ,tr("Columns"),"_Chars." ,this,&Hd44780::cols, &Hd44780::setCols ,"uint"  ),
    }} );
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
    }else{                                              // Clk Pin is Low
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
    //Get RS state: data or command
    if( m_pinRS->getInpState() ) writeData( m_input );
    else                         proccessCommand( m_input );
}

void Hd44780::showPins( bool show )
{
    m_pinRS->setVisible( show );
    m_pinRW->setVisible( show );
    m_pinEn->setVisible( show );
    
    for( int i=0; i<8; i++ ) m_dataPin[i]->setVisible( show );
}
