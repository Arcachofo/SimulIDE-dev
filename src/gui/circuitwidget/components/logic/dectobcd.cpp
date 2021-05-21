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

#include "dectobcd.h"
#include "itemlibrary.h"
#include "circuit.h"

Component* DecToBcd::construct( QObject* parent, QString type, QString id )
{
    return new DecToBcd( parent, type, id );
}

LibraryItem* DecToBcd::libraryItem()
{
    return new LibraryItem(
        tr( "Encoder(10/16 to 4)" ),
        tr( "Logic/Converters" ),
        "3to2g.png",
        "DecToBcd",
        DecToBcd::construct );
}

DecToBcd::DecToBcd( QObject* parent, QString type, QString id )
        : LogicComponent( parent, type, id )
        , eElement( id )
{
    m_width  = 4;
    m_height = 10;

    m_tristate = true;
    m_16Bits = false;
    m_bits = 10;

    QStringList pinList;

    pinList// Outputs:
            << "OR03A "
            << "OR04B "
            << "OR05C "
            << "OR06D "
            ;
    init( pinList );

    setNumInps( 10,"D", 1 );

    createOePin( "IU03OE ", id+"-in15"); // Output Enable
}
DecToBcd::~DecToBcd(){}

QList<propGroup_t> DecToBcd::propGroups()
{
    propGroup_t mainGroup { tr("Main") };
    mainGroup.propList.append( {"Invert_Inputs", tr("Invert Inputs"),""} );
    mainGroup.propList.append( {"_16_Bits", tr("16 Bits"),""} );

    QList<propGroup_t> pg = LogicComponent::propGroups();
    pg.prepend( mainGroup );
    return pg;
}

void DecToBcd::initialize()
{
    LogicComponent::initState();
}

void DecToBcd::stamp()
{
    for( int i=0; i<15; ++i ) m_inPin[i]->changeCallBack( this );
    LogicComponent::stamp( this);
}

void DecToBcd::voltChanged()
{
    LogicComponent::updateOutEnabled();

    int i;
    for( i=m_bits-2; i>=0; --i )
    {
        if( m_inPin[i]->getInpState() ) break;
    }
    m_nextOutVal = i+1;
    sheduleOutPuts( this );
}

void DecToBcd::set_16bits( bool set )
{
    m_16Bits = set;

    setNumInps( m_16Bits ? 16 : 10 );
    m_oePin->setY( m_area.y()-8 );
}

#include "moc_dectobcd.cpp"
