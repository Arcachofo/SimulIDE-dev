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
#include "circuit.h"
#include "itemlibrary.h"

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

    pinList // Inputs:
            << "IL01 D1"
            << "IL02 D2"
            << "IL03 D3"
            << "IL04 D4"
            << "IL05 D5"
            << "IL06 D6"
            << "IL07 D7"
            << "IL08 D8"
            << "IL09 D9"
            << "IL10 D10"
            << "IL11 D11"
            << "IL12 D12"
            << "IL13 D13"
            << "IL14 D14"
            << "IL15 D15"

            << "IU03OE "

            // Outputs:
            << "OR03A "
            << "OR04B "
            << "OR05C "
            << "OR06D "
            ;
    init( pinList );

    setOePin( m_inPin[15] );    // Output Enable

    for( int i=9; i<15; ++i ) m_inPin[i]->setVisible( false );
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

    if( m_16Bits ) m_bits = 16;
    else           m_bits = 10;

    int height = set ? 16 : m_height;

    for( int i=9; i<15; ++i )
    {
        m_inPin[i]->setVisible( set );
        if( !set ) m_inPin[i]->removeConnector();
    }
    m_area = QRect( -(m_width/2)*8, -(m_height/2)*8, m_width*8, height*8 );
    Circuit::self()->update();
}

#include "moc_dectobcd.cpp"
