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
#include "iopin.h"

Component* FlipFlopRS::construct( QObject* parent, QString type, QString id )
{ return new FlipFlopRS( parent, type, id ); }

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

    init({         // Inputs:
            "IL01 S",
            "IL03 R",
            "IL02>",
                   // Outputs:
            "OR01Q",
            "OR03!Q",
        });

    m_setPin   = m_inPin[0];
    m_resetPin = m_inPin[1];
    m_clkPin   = m_inPin[2];

    setSrInv( true );                           // Inver Set & Reset pins
    setClockInv( false );                        //Don't Invert Clock pin
    setTrigger( Clock );

    removeProperty(tr("Main"),"UseRS");
}
FlipFlopRS::~FlipFlopRS(){}

void FlipFlopRS::voltChanged()
{
    updateClock();
    bool clkAllow = (m_clkState == Clock_Allow); // Get Clk to don't miss any clock changes
    if( !clkAllow ) return;

    bool set   = sPinState();
    bool reset = rPinState();

    if( set || reset) m_nextOutVal = (set? 1:0) + (reset? 2:0);

    sheduleOutPuts( this );
}
