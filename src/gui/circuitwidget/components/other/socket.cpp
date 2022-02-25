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

#include <QPainter>
#include <QPushButton>
#include <QGraphicsProxyWidget>

#include "socket.h"
#include "itemlibrary.h"
#include "circuitwidget.h"
#include "simulator.h"
#include "circuit.h"
#include "e-node.h"
#include "pin.h"

#include "intprop.h"

Component* Socket::construct( QObject* parent, QString type, QString id )
{ return new Socket( parent, type, id ); }

LibraryItem* Socket::libraryItem()
{
    return new LibraryItem(
        tr( "Socket" ),
        tr( "Other" ),
        "socket.png",
        "Socket",
        Socket::construct);
}

Socket::Socket( QObject* parent, QString type, QString id )
       : Component( parent, type, id )
       , eElement( id )
{
    m_graphical = true;

    this->setZValue(-1 );

    m_color = QColor( 50, 50, 70 );
    m_size = 0;
    setSize( 8 );
    setLabelPos(-16,-44, 0);
    
    Simulator::self()->addToUpdateList( this );

    addPropGroup( { tr("Main"), {
new IntProp<Socket>( "Size", tr("Size"),"_Pins", this, &Socket::size, &Socket::setSize, "uint" )
    }} );
}
Socket::~Socket()
{
    m_connPins.clear();
}

void Socket::stamp()
{
    updatePins( true );
    update();
}

void Socket::updateStep()
{
    m_changed = false;
    updatePins( false );
    if( m_changed ) CircuitWidget::self()->powerCircOff();
}

void Socket::updatePins( bool connect )
{
    QList<QGraphicsItem*> list = this->collidingItems();
    std::vector<Pin*> connPins = m_connPins;
    m_connPins.clear();
    m_connPins.resize( m_size, NULL );
    for( QGraphicsItem* it : list )
    {
        if( it->type() == 65536+3 )        // Pin found
        {
            Pin* pin =  qgraphicsitem_cast<Pin*>( it );
            if( pin->parentItem() == this ) continue;
            if( pin->connector() ) continue; // Pin is already connected
            if( pin->isBus() )     continue; // Don't connect to Bus

            QPointF pinPos = this->mapFromScene( pin->scenePos() );
            int y = pinPos.y()+24;
            int x = abs( pinPos.x() );
            //qDebug() <<pin->pinId()<<pinPos<<y;

            if( (x < 1) && (y%8 < 2) )
            {
                int i = y/8;
                if( connect ){
                    eNode* node = m_pin[i]->getEnode();
                    pin->setEnode( node );
                }
                m_connPins[i] = pin;
            }
        }
    }
    if( !connect )
    {
        for( int i=0; i<m_size; i++ ){
            if( connPins[i] != m_connPins[i] ) m_changed = true;
        }
    }
}


void Socket::createSwitches( int c )
{
    int start = m_size;
    m_size = m_size+c;
    m_pin.resize( m_size*2 );

    for( int i=start; i<m_size; i++ )
        m_pin[i] = new Pin( 180, QPoint(-8,-32+8+i*8 ), m_id+"-pin"+QString::number(i), 0, this );
}

void Socket::deleteSwitches( int d )
{
    if( d > m_size ) d = m_size;
    int start = m_size-d;

    for( int i=start; i<m_size; i++ )
    {
        m_pin[i]->removeConnector();
        delete m_pin[i];
    }
    m_size = m_size-d;
    m_pin.resize( m_size*2, NULL );
    
    Circuit::self()->update();
}

void Socket::setSize( int size )
{
    if( Simulator::self()->isRunning() )  CircuitWidget::self()->powerCircOff();
    
    if( size == 0 ) size = 8;
    
    if     ( size < m_size ) deleteSwitches( m_size-size );
    else if( size > m_size ) createSwitches( size-m_size );

    m_connPins.resize( size );
    
    m_area = QRectF(-4, -28, 7, m_size*8 );

    Circuit::self()->update();
}


void Socket::paint( QPainter* p, const QStyleOptionGraphicsItem* option, QWidget* widget )
{
    Component::paint( p, option, widget );
    p->drawRoundRect( m_area, 4, 4 );

    QPen pen( Qt::black, 1, Qt::SolidLine, Qt::RoundCap, Qt::RoundJoin );
    p->setPen( pen );

    for( int i=0; i<m_size; i++ )
    {
        if( m_connPins[i] ) p->setBrush( QColor( 170, 170, 200 ) );
        else                p->setBrush( Qt::black );
        p->drawRoundRect(-2,-28+2+i*8, 3, 4, 1, 1 );
    }
}

//#include "moc_socket.cpp"
