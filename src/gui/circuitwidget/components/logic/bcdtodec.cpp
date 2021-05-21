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

#include "bcdtodec.h"
#include "itemlibrary.h"
#include "circuit.h"

Component* BcdToDec::construct( QObject* parent, QString type, QString id )
{
    return new BcdToDec( parent, type, id );
}

LibraryItem* BcdToDec::libraryItem()
{
    return new LibraryItem(
        tr( "Decoder(4 to 10/16)" ),
        tr( "Logic/Converters" ),
        "2to3g.png",
        "BcdToDec",
        BcdToDec::construct );
}

BcdToDec::BcdToDec( QObject* parent, QString type, QString id )
        : LogicComponent( parent, type, id )
        , eElement( id )
{
    m_width  = 4;
    m_height = 11;

    m_tristate = true;
    m_16Bits = false;

    setNumInps( 4,"S" );
    setNumOuts( 10 );
    createOePin( "IU01OE ", id+"-in4"); // Output Enable
}
BcdToDec::~BcdToDec(){}

QList<propGroup_t> BcdToDec::propGroups()
{
    propGroup_t mainGroup { tr("Main") };
    mainGroup.propList.append( {"Inverted", tr("Invert Outputs"),""} );
    mainGroup.propList.append( {"_16_Bits", tr("16 Bits"),""} );

    QList<propGroup_t> pg = IoComponent::propGroups();
    pg.prepend( mainGroup );
    return pg;
}

void BcdToDec::initialize()
{
    LogicComponent::initState();
    m_outPin[0]->setOutState( true );
    m_outValue = 1;
}

void BcdToDec::stamp()
{
    for( int i=0; i<4; ++i ) m_inPin[i]->changeCallBack( this );
    LogicComponent::stamp( this );
}

void BcdToDec::voltChanged()
{
    LogicComponent::updateOutEnabled();

    int dec = 0;

    for( int i=0; i<4; ++i )
        if( m_inPin[i]->getInpState() ) dec += pow( 2, i );

    m_nextOutVal = 1<<dec;
    sheduleOutPuts( this );
}

void BcdToDec::set_16bits( bool set )
{
    m_16Bits = set;
    setNumOuts( m_16Bits ? 16 : 10 );
    m_oePin->setY( m_area.y()-8 );
}

#include "moc_bcdtodec.cpp"
