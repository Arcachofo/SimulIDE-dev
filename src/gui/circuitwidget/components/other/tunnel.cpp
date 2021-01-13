/***************************************************************************
 *   Copyright (C) 2020 by santiago González                               *
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

#include "tunnel.h"
#include "circuitwidget.h"
#include "simulator.h"
#include "circuit.h"
#include "pin.h"

QHash<QString, QList<Tunnel*>*> Tunnel::m_tunnels;

Component* Tunnel::construct( QObject* parent, QString type, QString id )
{ return new Tunnel( parent, type, id ); }

LibraryItem* Tunnel::libraryItem()
{
    return new LibraryItem(
        tr( "Tunnel." ),
        tr( "Other" ),
        "tunnel.png",
        "Tunnel",
        Tunnel::construct );
}

Tunnel::Tunnel( QObject* parent, QString type, QString id )
      : Component( parent, type, id )
      , eElement( id )
{
    setLabelPos(-64,-24 );

    m_rotated = false;
    m_blocked = false;
    m_packed  = false;

    m_name = "";

    m_size = 20;
    m_area = QRect( -m_size-8-4, -4, m_size+4, 8 );

    QPoint pinPos = QPoint(0,0);
    m_pin.resize( 1 );
    m_pin[0] = new Pin( 0, pinPos, id+"-pin", 0, this);
    m_pin[0]->setLabelColor( Qt::black );

    setLabelPos(-16,-24, 0);
}
Tunnel::~Tunnel() { }

void Tunnel::setEnode( eNode* node )
{
    if( !m_tunnels.contains( m_name )  /// thi is only called by another tunnel
     || !m_pin[0]->isConnected() ) return;

    if( m_blocked ) return;
    m_blocked = true;

    if( node ) m_pin[0]->registerPinsW( node );
    m_blocked = false;
}

void Tunnel::registerPins( eNode* enode ) // called by m_pin[0]
{
    if( m_blocked ) return;

    QList<Tunnel*>* list = m_tunnels.value( m_name );
    if( !list ) return;

    m_blocked = true;

    for( Tunnel* tunnel: *list )
        if( tunnel != this ) tunnel->setEnode( enode );

    m_blocked = false;
}

void Tunnel::setName( QString name )
{
    if( name == m_name ) return;

    if( Simulator::self()->isRunning() )  CircuitWidget::self()->powerCircOff();

    removeTunnel(); // Remove old name

    if( m_tunnels.contains( name ) ) // There is already tunnel with this name
    {
        QList<Tunnel*>* list = m_tunnels.value( name );
        if( list ) list->append( this );
    }
    else if( name != "" )
    {
        QList<Tunnel*>* list = new QList<Tunnel*>();
        list->append( this );
        m_tunnels.insert( name, list );
    }

    m_name = name;
    m_pin[0]->setLabelText( name );

    if( name == "" ) m_size = 20;
    else  m_size = m_pin[0]->labelSizeX()+4;

    setRotated( m_rotated );
}

void Tunnel::setRotated( bool rot )
{
    m_rotated  = rot;
    if( rot )
    {
        m_area = QRect( 4, -4, m_size+4, 8 );
        m_pin[0]->setPinAngle( 180 );
        m_pin[0]->setLabelPos();
    }
    else {
        m_area = QRect( -m_size-8, -4, m_size+4, 8 );
        m_pin[0]->setPinAngle( 0 );
        m_pin[0]->setLabelPos();
    }
    Circuit::self()->update();
}

void Tunnel::removeTunnel()
{
    if( m_name.isEmpty() ) return;

    QList<Tunnel*>* list = m_tunnels.value( m_name );
    if( list )
    {
        list->removeAll( this );
        if( list->isEmpty() )
        {
            m_tunnels.remove( m_name );
            delete list;
        }
    }
}

void Tunnel::remove()
{
    removeTunnel();
    Component::remove();
    Circuit::self()->update();
}

QRectF Tunnel::boundingRect() const
{
    if( m_packed ) return QRectF( 0, 0, 0 ,0 );
    else return Component::boundingRect();
}

void Tunnel::paint( QPainter *p, const QStyleOptionGraphicsItem *option, QWidget *widget )
{
    if( m_hidden || m_packed ) return;

    if( m_tunnels.contains( m_name ) ) m_color = QColor( 255, 255, 250 );
    else                               m_color = QColor( 210, 210, 230 );

    Component::paint( p, option, widget );

    //p->setBrush( QColor( 255, 166, 0 ) );

    if( m_rotated )
    {
        QPointF points[5] =        {
            QPointF( m_size+8,-5 ),
            QPointF(  8,-5 ),
            QPointF(  4, 0 ),
            QPointF(  8, 5 ),
            QPointF( m_size+8, 5 ) };

        p->drawPolygon( points, 5 );
    }
    else
    {
        QPointF points[5] =        {
            QPointF(-m_size-8,-5 ),
            QPointF(  -8,-5 ),
            QPointF(  -4, 0 ),
            QPointF(  -8, 5 ),
            QPointF(-m_size-8, 5 ) };

        p->drawPolygon( points, 5 );
    }
}

#include "moc_tunnel.cpp"

