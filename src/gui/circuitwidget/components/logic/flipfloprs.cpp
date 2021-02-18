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

#include "flipfloprs.h"
#include "itemlibrary.h"
#include "simulator.h"

Component* FlipFlopRS::construct( QObject* parent, QString type, QString id )
{
    return new FlipFlopRS( parent, type, id );
}

LibraryItem* FlipFlopRS::libraryItem()
{
    return new LibraryItem(
        tr( "FlipFlop RS" ),
        tr( "Logic/Memory" ),
        "2to2.png",
        "FlipFlopRS",
        FlipFlopRS::construct );
}

FlipFlopRS::FlipFlopRS( QObject* parent, QString type, QString id )
          : FlipFlopBase( parent, type, id )
{
    m_width  = 3;
    m_height = 4;
    
    QStringList pinList;

    pinList // Inputs:
            << "IL01 S"
            << "IL03 R"
            << "IL02>"
            
            // Outputs:
            << "OR01Q"
            << "OR03!Q"
            ;
    init( pinList );

    eLogicDevice::createInput( m_inPin[0] );                  // Input S
    eLogicDevice::createInput( m_inPin[1] );                  // Input R

    m_setPin = m_input[0];
    m_resetPin = m_input[1];
    
    m_trigPin = m_inPin[2];
    eLogicDevice::createClockPin( m_trigPin );             // Input Clock
    
    eLogicDevice::createOutput( m_outPin[0] );               // Output Q
    eLogicDevice::createOutput( m_outPin[1] );               // Output Q'

    setSrInv( true );                           // Inver Set & Reset pins
    setClockInv( false );                        //Don't Invert Clock pin
    setTrigger( Clock );
}
FlipFlopRS::~FlipFlopRS(){}

void FlipFlopRS::voltChanged()
{
    // Get Clk to don't miss any clock changes
    bool clkAllow = (getClockState() == Clock_Allow);
    if( !clkAllow ) return;

    bool set   = getInputState( 0 );
    bool reset = getInputState( 1 );

    if( set || reset)   m_Q0 = set;

    m_nextOutVal = m_Q0? 1:2;
    sheduleOutPuts();
}
