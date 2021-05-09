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

#include "flipflopjk.h"
#include "itemlibrary.h"
#include "e-source.h"
#include "simulator.h"


Component* FlipFlopJK::construct( QObject* parent, QString type, QString id )
{
    return new FlipFlopJK( parent, type, id );
}

LibraryItem* FlipFlopJK::libraryItem()
{
    return new LibraryItem(
        tr( "FlipFlop JK" ),
        tr( "Logic/Memory" ),
        "3to2.png",
        "FlipFlopJK",
        FlipFlopJK::construct );
}

FlipFlopJK::FlipFlopJK( QObject* parent, QString type, QString id )
          : FlipFlopBase( parent, type, id )
{
    m_width  = 3;
    m_height = 4;
    
    QStringList pinList;

    pinList // Inputs:
            << "IL01 J"
            << "IL03 K"
            << "IU01S"
            << "ID02R"
            << "IL02>"
            
            // Outputs:
            << "OR01Q"
            << "OR03!Q"
            ;
    init( pinList );
    
    eLogicDevice::createInput( m_inPin[0] );                  // Input J
    eLogicDevice::createInput( m_inPin[1] );                  // Input K
    eLogicDevice::createInput( m_inPin[2] );                  // Input S
    eLogicDevice::createInput( m_inPin[3] );                  // Input R

    m_setPin = m_input[2];
    m_resetPin = m_input[3];
    m_dataPins = 2;
    
    m_trigPin = m_inPin[4];
    eLogicDevice::createClockPin( m_trigPin );            // Input Clock
    
    eLogicDevice::createOutput( m_outPin[0] );               // Output Q
    eLogicDevice::createOutput( m_outPin[1] );               // Output Q'

    setSrInv( true );                         // Invert Set & Reset pins
    setClockInv( false );                       //Don't Invert Clock pin
    setTrigger( Clock );
}
FlipFlopJK::~FlipFlopJK(){}

void FlipFlopJK::voltChanged()
{
    bool clkAllow = (eLogicDevice::getClockState() == Clock_Allow); // Get Clk to don't miss any clock changes

    bool set   = getInputState( 2 );
    bool reset = getInputState( 3 );

    if( set || reset) m_nextOutVal = (set? 1:0) + (reset? 2:0);
    else if( clkAllow )              // Allow operation
    {
        bool J = eLogicDevice::getInputState( 0 );
        bool K = eLogicDevice::getInputState( 1 );
        bool Q = m_output[0]->getState();

        m_Q0 = (J && !Q) || (!K && Q) ;
        m_nextOutVal = m_Q0? 1:2;
    }
    sheduleOutPuts();
}
#include "moc_flipflopjk.cpp"
