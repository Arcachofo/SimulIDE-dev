/***************************************************************************
 *   Copyright (C) 2012 by santiago González                               *
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

#include "node.h"
#include "connector.h"
#include "circuit.h"

#include "doubleprop.h"
#include "pointprop.h"

Node::Node( QObject* parent, QString type, QString id )
    : Component( parent, type, id )
{
    setZValue(2);
    
    m_color = QColor( Qt::black );
    m_isBus = false;

    m_pin.resize( 3 );
    for ( int i=0; i<3; i++ )
    {
        m_pin[i] = new Pin( 90*i, QPoint(0,0), id+"-"+uchar(48+i), i, this );
        m_pin[i]->setLength( 0 );
    }
    remPropGroup( "CompGraphic" );
    addPropGroup( { "CompGraphic", {
new PointProp <Component>( "Pos"     ,"","",this, &Component::position,   &Component::setPosition )
    }} );

}
Node::~Node(){}

void Node::inStateChanged( int rem ) // Called by pin
{
    if     ( rem == 1 ) remove();
    else if( rem == 2 ) // Propagate Is Bus
    {
        for( int i=0; i< 3; i++) m_pin[i]->setIsBus( true );
        m_isBus = true;
}   }

void Node::registerEnode( eNode* enode )
{
    for( int i=0; i<3; i++ )
        if( m_pin[i]->isConnected() ) m_pin[i]->registerPinsW( enode );
}

void Node::remove() // Only remove if there are less than 3 connectors
{
    int con[2] = { 0, 0 };
    int conectors = 0;
    int conecteds = 0;

    for( int i=0; i< 3; i++)
    {
        if( m_pin[i]->connector() )
        {
            if( conecteds == 0 ) { conecteds++; con[0] = i; }
            else con[1] = i;
            conectors++;
    }   }
    if( conectors < 3 ) 
    {
        if( conectors == 2 ) joinConns( con[0], con[1] );  // 2 Conn
        else                 m_pin[con[0]]->removeConnector();

        Circuit::self()->nodeList()->removeOne( this );
        if( this->scene() ) Circuit::self()->removeItem( this );
}   }

void Node::joinConns( int c0, int c1 )
{
    Pin* pin0 = m_pin[c0];
    Pin* pin1 = m_pin[c1];

    Connector* con0 = pin0->connector();
    Connector* con1 = pin1->connector();
    Connector* con = new Connector( Circuit::self(), "Connector", con0->getUid(), pin0->conPin() );
    Circuit::self()->conList()->append( con );

    QStringList list0 = con0->pointList();
    QStringList list1 = con1->pointList();
    QStringList plist;

    if( pin0 == con0->startPin() )
        while( !list0.isEmpty() )
        {
            QString p2 = list0.takeLast();
            plist.append(list0.takeLast());
            plist.append(p2);
        }
    else while( !list0.isEmpty() ) plist.append( list0.takeFirst() );

    if( pin1 == con1->endPin() )
        while( !list1.isEmpty() )
        {
            QString p2 = list1.takeLast();
            plist.append(list1.takeLast());
            plist.append(p2);
        }
    else while( !list1.isEmpty() ) plist.append( list1.takeFirst() );

    con->setPointList( plist );

    pin0->setEnode( NULL );
    con0->setStartPin( NULL );
    con0->setEndPin( NULL );
    con0->remove();
    
    pin1->setEnode( NULL );
    con1->setStartPin( NULL );
    con1->setEndPin( NULL );
    con1->remove();

    con->closeCon( pin1->conPin(), true );
    if( this->isSelected() ) con->setSelected( true );
}

void Node::setHidden( bool hid, bool )
{
    m_hidden = hid;

    for( Pin* pin : m_pin ) pin->setVisible( !hid );
    this->setVisible( !hid );
}

void Node::paint( QPainter* p, const QStyleOptionGraphicsItem* option, QWidget* widget )
{
    if( m_hidden ) return;

    //p->setBrush( Qt::blue );
    //p->drawRect( boundingRect() );

    Component::paint( p, option, widget );
    
    if( m_isBus ) p->drawEllipse( QPointF(0,0), 1.8, 1.8  );
    else          p->drawEllipse( QPointF(0,0), 1.4, 1.4 );
}
