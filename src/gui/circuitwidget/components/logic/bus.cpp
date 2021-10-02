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

#include <QPainter>

#include "bus.h"
#include "connector.h"
#include "simulator.h"
#include "circuit.h"
#include "itemlibrary.h"
#include "e-node.h"

#include "intprop.h"

Component* Bus::construct( QObject* parent, QString type, QString id )
{ return new Bus( parent, type, id ); }

LibraryItem* Bus::libraryItem()
{
    return new LibraryItem(
        tr( "Bus" ),
        tr( "Logic/Other Logic" ),
        "outbus.png",
        "Bus",
        Bus::construct );
}

Bus::Bus( QObject* parent, QString type, QString id )
   : Component( parent, type, id )
   , eElement( id )
{
    m_busPin1 = new Pin( 270, QPoint( 0, 0 ), m_id+"-busPinI", 1, this );
    m_busPin1->setFlag( QGraphicsItem::ItemStacksBehindParent, false );
    m_busPin1->setLength( 1 );
    m_busPin1->setIsBus( true );

    m_numLines = 0;
    m_startBit = 0;
    setNumLines( 8 );       // Create Input Pins

    m_ePin[0] = m_pin[0] = m_busPin0 = new Pin( 90, QPoint( 0, 0 ), m_id+"-ePin0", 1, this );
    m_busPin0->setFlag( QGraphicsItem::ItemStacksBehindParent, false );
    m_busPin0->setLength( 1 );
    m_busPin0->setIsBus( true );

    addPropGroup( { tr("Main"), {
new IntProp<Bus>( "Num_Bits" , tr("Size")     ,"_Bits", this, &Bus::numLines, &Bus::setNumLines, "uint" ),
new IntProp<Bus>( "Start_Bit", tr("Start Bit"),""     , this, &Bus::startBit, &Bus::setStartBit, "uint" )
    }} );
}
Bus::~Bus(){}

void Bus::initialize()
{
    eNode* busEnode = m_busPin0->getEnode();
    if( !busEnode ) busEnode = m_busPin1->getEnode();
    if( !busEnode ) return;

    for( int i=1; i<=m_numLines; i++ )
    {
        if( !m_pin[i]->isConnected() ) continue;
        eNode* enode = new eNode( m_id+"eNode"+QString::number( i ) );
        m_pin[i]->conPin()->registerEnode( enode );

        QList<ePin*> epins = enode->getEpins();
        busEnode->addBusPinList( epins, m_startBit+i-1 );
}   }

void Bus::inStateChanged( int msg )
{
    if( msg == 3 ) // Called by m_busPin When disconnected
    {
        for( int i=1; i<=m_numLines; i++ )
        {
            if( !m_pin[i]->isConnected() ) continue;

            eNode* enode = new eNode( m_id+"eNode"+QString::number( i ) );
            Pin* pin = m_pin[i];
            pin->conPin()->registerEnode( enode );
}   }   }

void Bus::registerEnode( eNode* enode )
{
    if( !enode->isBus() ) return;

    if( m_busPin0->isConnected() ) m_busPin0->registerPinsW( enode );
    if( m_busPin1->isConnected() ) m_busPin1->registerPinsW( enode );
}

void Bus::setNumLines( int lines )
{
    if( lines == m_numLines ) return;
    if( lines < 1 ) return;

    for( int i=1; i<=m_numLines; i++ )
    {
        if( m_pin[i]->isConnected() ) m_pin[i]->connector()->remove();
        if( m_pin[i]->scene() ) Circuit::self()->removeItem( m_pin[i] );
        delete m_pin[i];
    }
    m_numLines = lines;

    m_pin.resize( lines+2 );
    m_ePin.resize( lines+2 );
    
    for( int i=1; i<=lines; i++ )
    {
        QString pinId = m_id+"-ePin"+QString::number(i);
        Pin* pin = new Pin( 180, QPoint(-8, -8*lines+i*8 ), pinId, i, this );

        pin->setFontSize( 4 );
        pin->setLabelColor( QColor( 0, 0, 0 ) );
        pin->setLabelText( " "+QString::number( m_startBit+i-1 ) );
        m_pin[i]  = pin;
        m_ePin[i] = pin;
    }
    m_busPin1->setPos( QPoint( 0 ,-lines*8+8 ) );
    m_busPin1->isMoved();
    m_pin[ lines+1 ]  = m_busPin1;
    m_ePin[ lines+1 ] = m_busPin1;

    m_height = lines-1;
    m_area = QRect( -3,-m_height*8-2, 5, m_height*8+4 );
    Circuit::self()->update();
}

void Bus::setStartBit( int bit )
{
    if( bit < 0 ) bit = 0;
    m_startBit = bit;

    for( int i=1; i<=m_numLines; i++ )
        m_pin[i]->setLabelText( " "+QString::number( m_startBit+i-1 ) );
}

void Bus::paint( QPainter* p, const QStyleOptionGraphicsItem* option, QWidget* widget )
{
    Component::paint( p, option, widget );

    QPen pen = p->pen();
    pen.setWidth(3);
    p->setPen(pen);

    p->drawRect( QRect( 0, -m_height*8, 0, m_height*8 ) );
              //QRect( -2, -m_height*8-4, 2, m_height*8+8 );
}
