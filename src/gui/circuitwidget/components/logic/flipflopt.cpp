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

#include "flipflopt.h"
#include "itemlibrary.h"
#include "iopin.h"

Component* FlipFlopT::construct( QObject* parent, QString type, QString id )
{ return new FlipFlopT( parent, type, id ); }

LibraryItem* FlipFlopT::libraryItem()
{
    return new LibraryItem(
        tr( "FlipFlop T" ),
        tr( "Logic/Memory" ),
        "2to2.png",
        "FlipFlopT",
        FlipFlopT::construct );
}

FlipFlopT::FlipFlopT( QObject* parent, QString type, QString id )
         : FlipFlopBase( parent, type, id )
{
    m_width  = 3;
    m_height = 3;
    m_dataPins = 1;

    init({         // Inputs:
            "IL01T",
            "IU01S",
            "ID02R",
            "IL02>",
                   // Outputs:
            "OR01Q",
            "OR02!Q"
        });

    m_setPin   = m_inPin[1];
    m_resetPin = m_inPin[2];
    m_clkPin   = m_inPin[3];

    setSrInv( true );       // Inver Set & Reset pins
    setClockInv( false );   // Don't Invert Clock pin
    setTrigger( Clock );
}
FlipFlopT::~FlipFlopT(){}

void FlipFlopT::calcOutput()
{
    bool T = m_inPin[0]->getInpState();
    if( T ) m_nextOutVal = m_outPin[1]->getOutState()? 1:2; // !Q state = Toggle
}
