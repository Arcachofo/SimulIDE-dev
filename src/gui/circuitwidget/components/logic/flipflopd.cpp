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

#include "flipflopd.h"
#include "itemlibrary.h"
#include "simulator.h"

Component* FlipFlopD::construct( QObject* parent, QString type, QString id )
{
    return new FlipFlopD( parent, type, id );
}

LibraryItem* FlipFlopD::libraryItem()
{
    return new LibraryItem(
        tr( "FlipFlop D" ),
        tr( "Logic/Memory" ),
        "2to2.png",
        "FlipFlopD",
        FlipFlopD::construct );
}

FlipFlopD::FlipFlopD( QObject* parent, QString type, QString id )
         : FlipFlopBase( parent, type, id )
{
    m_width  = 3;
    m_height = 3;
    
    QStringList pinList;

    pinList // Inputs:
            << "IL01D"
            << "IU01S"
            << "ID02R"
            << "IL02>"
            
            // Outputs:
            << "OR01Q"
            << "OR02!Q"
            ;
    init( pinList );

    m_setPin   = m_inPin[1];
    m_resetPin = m_inPin[2];
    m_dataPins = 1;
    
    m_clkPin = m_inPin[3];

    setSrInv( true );                           // Inver Set & Reset pins
    setClockInv( false );                       // Don't Invert Clock pin
    setTrigger( Clock );
}
FlipFlopD::~FlipFlopD(){}

void FlipFlopD::voltChanged()
{
    bool clkAllow = (getClockState() == Clock_Allow); // Get Clk to don't miss any clock changes

    bool set   = m_setPin->getInpState();
    bool reset = m_resetPin->getInpState();

    if( set || reset)   m_nextOutVal = (set? 1:0) + (reset? 2:0);
    else if( clkAllow ) m_nextOutVal = m_inPin[0]->getInpState()? 1:2; // D state

    sheduleOutPuts( this );
}

#include "moc_flipflopd.cpp"
