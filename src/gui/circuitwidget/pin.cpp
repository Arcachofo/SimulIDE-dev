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

#include <QApplication>
#include <QPainter>
#include <math.h>

#include "pin.h"
#include "bus.h"
#include "node.h"
#include "tunnel.h"
#include "connector.h"
#include "connectorline.h"
#include "circuit.h"
#include "simulator.h"

Pin::Pin( int angle, const QPoint pos, QString id, int index, Component* parent )
   : QObject()
   , QGraphicsItem( parent )
   , ePin( id, index )
   , m_label( parent )
{
    m_component = parent;
    m_area = QRect(-3, -3, 11, 6);
    m_pinState = undef_state;
    m_pinType = pinNormal;
    m_packageType = "";

    m_blocked = false;
    m_isBus   = false;
    m_unused  = false;
    m_PinChanged = false;
    
    my_connector = NULL;
    m_conPin     = NULL;
    m_angle      = angle;
    m_Hflip = 1;
    m_Vflip = 1;
    
    m_color[0] = Qt::black;
    m_color[1] = QColor( 100, 100, 250 );
    m_color[2] = QColor( 0, 0, 180 );;
    m_color[3] = QColor( 60, 120, 60 );
    m_color[4] = QColor( 0, 0, 180 );
    m_color[5] = QColor( 180, 0, 0 );
    m_color[6] = QColor( 100, 100, 250 );
    m_color[7] = QColor( 250, 120, 0 );

    QFont font;
    #ifdef _WIN32
    font.setFamily("Consolas");
    #else
    font.setFamily("Ubuntu Mono");
    #endif

    font.setPixelSize(7);
    //font.setStretch( 110 );
    //font.setLetterSpacing( QFont::PercentageSpacing, 105 );
    m_label.setFont( font );
    m_label.setText("");
    m_label.setBrush( QColor( 250, 250, 200 ) );

    setObjectName( id );
    setConnector( NULL );
    setPos( pos );
    setRotation( 180-angle );
    setLength(8);
    setCursor( Qt::CrossCursor );
    setFlag( QGraphicsItem::ItemStacksBehindParent, true );
    setFlag( QGraphicsItem::ItemIsSelectable, false );

    Circuit::self()->addPin( this, id );

    connect( parent, SIGNAL( moved() ),
               this, SLOT( isMoved() ), Qt::UniqueConnection );

    connect( parent, SIGNAL( flip( int, int ) ),
               this, SLOT( flip( int, int ) ), Qt::UniqueConnection );
}
Pin::~Pin()
{
    Circuit::self()->remPin( m_id );
}

void Pin::remove()
{
    setConnector( NULL );
    m_component->inStateChanged( 1 );          // Used by node to remove
}

void Pin::setUnused( bool unused )
{
    m_unused = unused;
    if( unused ) setCursor( Qt::ArrowCursor );
    else         setCursor( Qt::CrossCursor );
    update();
}

void Pin::registerPinsW( eNode* enode, int n )     // Called by component, calls conPin
{
    if( m_blocked ) return;
    m_blocked = true;

    if( !m_isBus ) ePin::setEnode( enode );
    if( m_conPin ) m_conPin->registerEnode( enode, n ); // Call pin at other side of Connector

    m_blocked = false;
}

void Pin::registerEnode( eNode* enode, int n )     // Called by m_conPin
{
    if( m_blocked ) return;
    m_blocked = true;

    if( !m_isBus )
    {
        ePin::setEnode( enode );
        n = m_index;
    }
    m_component->registerEnode( enode, n );

    m_blocked = false;
}

void  Pin::setConnector( Connector* connector )
{
    if( connector )
    {
        setCursor( Qt::ArrowCursor );
        if( m_isBus ) connector->setIsBus( true );
    }else{
        m_conPin = NULL;
        setCursor( Qt::CrossCursor );
    }
    my_connector = connector;
}

void Pin::removeConnector()
{
    if( my_connector ) my_connector->remove();
    setConnector( NULL );
}

Pin* Pin::connectPin( bool connect )      // Auto-Connect
{
    //if( !connect && m_pinType != pinSocket ) return NULL; // Dont connect Socket to Socket

    Pin* _pin = NULL;
    QList<QGraphicsItem*> list = this->collidingItems();
    while( !list.isEmpty() )
    {
        QGraphicsItem* it = list.takeLast();
        if( it->type() == 65536+3 )          // Pin found
        {
            Pin* pin =  qgraphicsitem_cast<Pin*>( it );

            if( pin->parentItem() == this->parentItem() ) continue;
            if( abs(scenePos().x()-pin->scenePos().x()) > 3 ) continue;
            if( abs(scenePos().y()-pin->scenePos().y()) > 3 ) continue;
            if( m_isBus != pin->isBus() ) continue; // Only connect Bus to Bus
            if( pin->connector() ) continue;
            if( pin->unused() ) continue;
            if( !connect && pin->pinType() < pinSocket
              && pin->isVisible() && !pin->isObscuredBy( m_component ) ) _pin = pin;
            if( connect )
            {
                Circuit::self()->newconnector( this );
                Circuit::self()->closeconnector( pin );
            }
            break;
        }
        else if( connect && (it->type() == UserType+2) ) // ConnectorLine
        {
            ConnectorLine* line =  qgraphicsitem_cast<ConnectorLine*>( it );
            if( m_isBus != line->connector()->isBus() ) continue;
            Circuit::self()->newconnector( this );
            line->connectToWire( QPoint( scenePos().x(), scenePos().y()) );
            break;
        }
    }
    return _pin;
}

void Pin::isMoved()
{
    if( my_connector ) my_connector->updateConRoute( this, scenePos() );
    else if( !isConnected() )
    {                                       // Auto-Connect
        if( !Circuit::self()->isBusy()
         && (QApplication::queryKeyboardModifiers() & Qt::ShiftModifier) )
            connectPin( true );
    }
    setLabelPos();
}

void Pin::flip( int h, int v )
{
    m_Hflip = h;
    m_Vflip = v;
    setLabelPos();
}

void Pin::mousePressEvent( QGraphicsSceneMouseEvent* event )
{
    if( m_unused ) return;
    
    if( event->button() == Qt::LeftButton ) // Start/Close Connector
    {
        if( my_connector ) event->ignore();
        else{
            if( Circuit::self()->is_constarted() )
            {
                Connector* con = Circuit::self()->getNewConnector();
                if( con->isBus() != m_isBus ) // Avoid connect Bus with no-Bus
                { event->ignore(); return; }
            }
            event->accept();
            if( Circuit::self()->is_constarted() ) Circuit::self()->closeconnector( this );
            else                                   Circuit::self()->newconnector( this );
}   }   }

void Pin::setLabelText( QString label )
{
    m_labelText = label;

    if( label.contains("!"))
    {
        QString text;
        for( int i=0; i<label.size(); i++ )
        {
            QString e = "!";
            QChar ch = label[i];
            if( ch == e[0] ) text.append("\u0305");
            else             text.append( ch );
        }
        label = text;
    }
    m_label.setText( label );
    setLabelPos();
}

void Pin::setLabelPos()
{
    QFontMetrics fm( m_label.font() );

    int xlabelpos = pos().x();
    int ylabelpos = pos().y();
    int height = (fm.height()+1)/2;
    int offset = m_length + height/2;

    if( m_angle == 0 )         // Right side
    {
        m_label.setRotation( 0 );
        if( m_Hflip == -1 ) xlabelpos -= offset;
        else                xlabelpos -= fm.width(m_label.text())+offset;
        if( m_Vflip == -1 ) ylabelpos += height;
        else                ylabelpos -= height;
    }
    else if( m_angle == 90 )   // Top
    {
        m_label.setRotation(m_angle);
        if( m_Hflip == -1 ) xlabelpos -= height;
        else                xlabelpos += height;
        if( m_Vflip == -1 ) ylabelpos += fm.width(m_label.text())+offset;
        else                ylabelpos += offset;
    }
    else if( m_angle == 180 )  // Left
    {
        m_label.setRotation( 0 );
        if( m_Hflip == -1 ) xlabelpos += fm.width(m_label.text())+offset;
        else                xlabelpos += offset;
        if( m_Vflip == -1 ) ylabelpos += height;
        else                ylabelpos -= height;
    }
    else if( m_angle == 270 )  // Bottom
    {
        m_label.setRotation( m_angle );
        if( m_Hflip == -1 ) xlabelpos += height;
        else                xlabelpos -= height;
        if( m_Vflip == -1 ) ylabelpos -= fm.width(m_label.text())+offset;
        else                ylabelpos -= offset;
    }
    m_label.setPos( xlabelpos, ylabelpos );
    m_label.setTransform( QTransform::fromScale( m_Hflip, m_Vflip ) );
}

void Pin::setLabelColor( QColor color ) { m_label.setBrush( QBrush(color) ); }

int Pin::labelSizeX()
{
    QFontMetrics fm( m_label.font() );
    return fm.width( m_label.text() );
}

void Pin::setFontSize( int size )
{
    QFont font = m_label.font();
    font.setPixelSize( size );
    m_label.setFont( font );
}

void Pin::setPinAngle( int angle )
{
    m_angle= angle;
    setRotation( 180-angle );
}

void Pin::setY( qreal y )
{
    QGraphicsItem::setY( y );
    isMoved();
}

void Pin::moveBy( int dx, int dy )
{
    m_label.moveBy( dx, dy );
    QGraphicsItem::moveBy( dx, dy );
}

void Pin::setLength( int length )
{
    if( length < 1 ) length = 1;
    m_length = length;
    int aLength = 11;
    if( length == 1 ) aLength = 6;
    m_area = QRect(-3, -3, aLength, 6);
    setLabelPos();
}

void Pin::setIsBus( bool bus )
{
    if( m_isBus == bus ) return;
    if( !bus ) return;
    m_isBus = bus;
    
    if( my_connector ) my_connector->setIsBus( true );
    if( m_conPin ) m_conPin->setIsBus( true );
    
    m_component->inStateChanged( 2 );         // Propagate Is Bus (Node)
}

void Pin::setVisible( bool visible )
{
    m_label.setVisible( visible );
    QGraphicsItem::setVisible( visible );
}

void Pin::updateStep()
{
    if( m_unused ) return;
    if( m_PinChanged ) update();
}

void Pin::paint( QPainter* painter, const QStyleOptionGraphicsItem* option, QWidget* widget )
{
    if( !isVisible() ) return;
    m_PinChanged = false;

    QPen pen( m_color[0], 3, Qt::SolidLine, Qt::RoundCap, Qt::RoundJoin);

    //painter->setBrush( Qt::red );
    //painter->drawRect( boundingRect() );

    if( isSelected() ) pen.setColor( Qt::darkGray);
    else if( m_unused ) pen.setColor( QColor( 75, 120, 170 ));
    else if( Circuit::self()->animate() ) pen.setColor( m_color[m_pinState] );

    painter->setPen(pen);
    if( m_length > 1 ) painter->drawLine( 0, 0, m_length-1, 0);
    else painter->drawLine( QPointF(-0.01, 0 ), QPointF( 0.03, 0 ));

    if( m_inverted )
    {
        painter->setBrush( Qt::white );
        QPen pen = painter->pen();
        pen.setWidthF( 1.8 );
        //if( isSelected() ) pen.setColor( Qt::darkGray);
        painter->setPen(pen);
        QRectF rect( 3.5,-2.2, 4.4, 4.4 );
        painter->drawEllipse(rect);
    }

    if( !m_unused && Circuit::self()->animate() )
    {
        pen.setWidth( 2 );
        painter->setPen(pen);
        if( m_pinState >= input_low ) // Draw Input arrow
        {
            painter->drawLine( 2, 0, 0, 2);
            painter->drawLine( 0,-2, 2, 0);
        }else{
            if( m_pinState >= out_low ) // Draw lower half Output arrow
            painter->drawLine( 0, 0, 2, 2);
            if( m_pinState >= driven_low )
            painter->drawLine( 2,-2, 0, 0);// Draw upper half Output arrow
        }
}   }

#include "moc_pin.cpp"
