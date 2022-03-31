/***************************************************************************
 *   Copyright (C) 2022 by santiago González                               *
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

#include "connbase.h"
#include "circuitwidget.h"
#include "simulator.h"
#include "circuit.h"
//#include "pin.h"

#include "intprop.h"

ConnBase::ConnBase( QObject* parent, QString type, QString id )
       : Component( parent, type, id )
       , eElement( id )
{
    m_graphical = true;
    //this->setZValue(-1 );

    m_size = 0;
    setSize( 8 );
    setLabelPos(-16,-44, 0);

    addPropGroup( { tr("Main"), {
new IntProp<ConnBase>( "Size", tr("Size"),"_Pins", this, &ConnBase::size, &ConnBase::setSize, "uint" )
    }} );
}
ConnBase::~ConnBase()
{
    m_connPins.clear();
}

void ConnBase::registerEnode( eNode* enode, int n )
{
    for( int i=0; i<m_size; i++ )
    {
        if( m_pin[i]->conPin() ) m_pin[i]->registerPinsW( enode, n );
        if( m_sockPins[i]->conPin() ) m_sockPins[i]->registerPinsW( enode, n );
    }
}

void ConnBase::createPins( int c )
{
    int start = m_size;
    m_size = m_size+c;
    m_pin.resize( m_size );
    m_sockPins.resize( m_size );

    for( int i=start; i<m_size; i++ )
    {
        m_pin[i] = new Pin( 180, QPoint(-8,-32+8+i*8 ), m_id+"-pin"+QString::number(i), 0, this );
        m_sockPins[i] = new Pin( 90, QPoint( 0,-32+8+i*8 ), m_id+"-pin"+QString::number(i+m_size), 0, this );
        m_sockPins[i]->setFlag( QGraphicsItem::ItemStacksBehindParent, false );
        m_sockPins[i]->setLength( 1 );
        m_sockPins[i]->setPinType( m_pinType );
    }
}

void ConnBase::deletePins( int d )
{
    if( d > m_size ) d = m_size;
    int start = m_size-d;

    for( int i=start; i<m_size; i++ )
    {
        m_pin[i]->removeConnector();
        delete m_pin[i];

        m_sockPins[i]->removeConnector();
        delete m_sockPins[i];
    }
    m_size = m_size-d;
    m_pin.resize( m_size );
    m_sockPins.resize( m_size );
    
    Circuit::self()->update();
}

void ConnBase::setSize( int size )
{
    if( Simulator::self()->isRunning() )  CircuitWidget::self()->powerCircOff();
    
    if( size == 0 ) size = 8;
    
    if     ( size < m_size ) deletePins( m_size-size );
    else if( size > m_size ) createPins( size-m_size );

    m_connPins.resize( size );
    
    m_area = QRectF(-4, -28, 8, m_size*8 );

    Circuit::self()->update();
}

void ConnBase::remove()
{
    for( int i=0; i<m_size; i++ ) m_sockPins[i]->removeConnector();
    Component::remove();
}

void ConnBase::paint( QPainter* p, const QStyleOptionGraphicsItem* option, QWidget* widget )
{
    Component::paint( p, option, widget );
    //p->drawRoundRect( m_area, 4, 4 );

    updatePixmap();

    QPen pen( Qt::black, 1, Qt::SolidLine, Qt::RoundCap, Qt::RoundJoin );
    p->setPen( pen );
    if( !this->m_hidden ) p->drawRoundRect( m_area, 4, 4 );

    for( int i=0; i<m_size; i++ )
    {
        p->drawPixmap( m_area.x(), m_area.y()+i*8, 8, 8, m_pinPixmap );
        //if( m_connPins[i] ) p->setBrush( QColor( 170, 170, 200 ) );
        //else                p->setBrush( Qt::black );
        //p->drawRoundRect(-2,-28+2+i*8, 3, 4, 1, 1 );
    }
}

//#include "moc_socket.cpp"
