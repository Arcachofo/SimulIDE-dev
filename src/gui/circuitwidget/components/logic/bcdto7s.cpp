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

#include "bcdto7s.h"
#include "itemlibrary.h"
#include "iopin.h"

#include "boolprop.h"

Component* BcdTo7S::construct( QObject* parent, QString type, QString id )
{ return new BcdTo7S( parent, type, id ); }

LibraryItem* BcdTo7S::libraryItem()
{
    return new LibraryItem(
        tr( "Bcd To 7S." ),
        tr( "Logic/Converters" ),
        "2to3g.png",
        "BcdTo7S",
        BcdTo7S::construct );
}

BcdTo7S::BcdTo7S( QObject* parent, QString type, QString id )
       : BcdBase( parent, type, id )
{
    m_width  = 4;
    m_height = 8;

    init({         // Inputs:
            "IL03 S0",
            "IL04 S1",
            "IL05 S2",
            "IL06 S3",
                    // Outputs:
            "OR01a ",
            "OR02b ",
            "OR03c ",
            "OR04d ",
            "OR05e ",
            "OR06f ",
            "OR07g "
        });

    createOePin( "IU01OE ", id+"-in4");

    addPropGroup( { tr("Main"), {
new BoolProp<BcdTo7S>( "Inverted", tr("Invert Outputs"),"", this, &BcdTo7S::invertOuts, &BcdTo7S::setInvertOuts )
    }} );
    addPropGroup( { tr("Electric"), IoComponent::inputProps()+IoComponent::outputProps() } );
    addPropGroup( { tr("Edges"), IoComponent::edgeProps() } );
}
BcdTo7S::~BcdTo7S(){}

void BcdTo7S::stamp()
{
    BcdBase::stamp();

    uint8_t value = m_values[0];
    for( int i=0; i<7; ++i ) m_outPin[i]->setOutState( value & (1<<i) );
}

void BcdTo7S::voltChanged()
{
    LogicComponent::updateOutEnabled();
    BcdBase::voltChanged();
    m_nextOutVal = m_digit;
    sheduleOutPuts( this );
}
