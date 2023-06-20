/***************************************************************************
 *   Copyright (C) 2012 by Santiago González                               *
 *                                                                         *
 ***( see copyright.txt file at root folder )*******************************/

#include <math.h>

#include <QApplication>
#include <QPainter>

#include "pin.h"
#include "bus.h"
#include "node.h"
#include "lachannel.h"
#include "connector.h"
#include "connectorline.h"
#include "circuit.h"
#include "simulator.h"

Pin::Pin( int angle, const QPoint pos, QString id, int index, Component* parent )
   : QGraphicsItem( parent )
   , ePin( id, index )
   , m_label( parent )
{
    m_area = QRect(-3, -3, 11, 6);

    m_component = parent;
    m_pinState = undef_state;
    m_pinType  = pinNormal;

    m_blocked = false;
    m_isBus   = false;
    m_unused  = false;
    m_animate = false;
    m_warning = false;
    
    m_dataCannel = NULL;
    my_connector = NULL;
    m_conPin     = NULL;
    m_angle      = angle;
    m_space = 0;
    m_Hflip = 1;
    m_Vflip = 1;
    m_overScore = -1;
    
    m_color[0] = Qt::black;
    m_color[1] = QColor( 100, 100, 250 );
    m_color[2] = QColor( 0, 0, 180 );;
    m_color[3] = QColor( 60, 120, 60 );
    m_color[4] = QColor( 0, 0, 180 );
    m_color[5] = QColor( 180, 0, 0 );
    m_color[6] = QColor( 100, 100, 250 );
    m_color[7] = QColor( 250, 120, 0 );

    QFont font;
    font.setFamily("Ubuntu Mono");
    font.setPixelSize(7);
    font.setStretch( 100 );
    font.setLetterSpacing( QFont::PercentageSpacing, 100 );
#ifndef Q_OS_UNIX
    font.setLetterSpacing( QFont::PercentageSpacing, 87 );
    //font.setStretch( 99 );
#endif
    m_label.setFont( font );
    m_label.setText("");
    m_label.setBrush( QColor( 250, 250, 200 ) );

    /// setObjectName( id );
    setConnector( NULL );
    setPos( pos );
    setRotation( 180-angle );
    setLength(8);
    setCursor( Qt::CrossCursor );
    setFlag( QGraphicsItem::ItemStacksBehindParent, true );
    setFlag( QGraphicsItem::ItemIsSelectable, false );

    Circuit::self()->addPin( this, id );
    animate( Circuit::self()->animate() );

    m_component->addSignalPin( this );
}
Pin::~Pin()
{
    Circuit::self()->remPin( m_id );
}

void Pin::remove()
{
    setConnector( NULL );
    m_component->inStateChanged( 1 );          // Used by node to remove
    m_component->remSignalPin( this );
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
    if( m_dataCannel ) m_dataCannel->registerEnode( enode, n );

    m_blocked = false;
}

void  Pin::setConnector( Connector* connector )
{
    my_connector = connector;
    m_warning = false;

    if( my_connector )
    {
        setCursor( Qt::ArrowCursor );
        if( m_isBus ) my_connector->setIsBus( true );
    }else{
        m_conPin = NULL;
        setCursor( Qt::CrossCursor );
    }
}

void Pin::removeConnector()
{
    if( my_connector ) my_connector->remove();
    setConnector( NULL );
}

Pin* Pin::connectPin( bool connect )      // Auto-Connect
{
    Pin* _pin = NULL;
    QList<QGraphicsItem*> list = this->collidingItems();
    while( !list.isEmpty() )
    {
        QGraphicsItem* it = list.takeLast();
        if( it->type() == 65536+3 )          // Pin found
        {
            Pin* pin =  qgraphicsitem_cast<Pin*>( it );

            if( pin->parentItem() == this->parentItem() ) continue;
            if( fabs(scenePos().x()-pin->scenePos().x()) > 3 ) continue;
            if( fabs(scenePos().y()-pin->scenePos().y()) > 3 ) continue;
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
    else if( this->isVisible() && !my_connector )
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

void Pin::setLabelText( QString label, bool over )
{
    label = label.simplified();
    m_labelText = label;
    m_overScore = -1;
    if( over && label.startsWith("!")) // Draw overscore
    {
        if( !m_inverted ) m_overScore = 1; // label.indexOf("!");
        label.remove( 0, 1 );// replace("!","");
    }
    /// Hack: if ItemStacksBehindParent then overscore does not paint
    setFlag( QGraphicsItem::ItemStacksBehindParent, m_overScore == -1 );
    m_label.setText( label );
    setLabelPos();
}

void Pin::setSpace( int s )
{
    m_space = s;
    setLabelPos();
}

void Pin::setLabelPos()
{
    QFontMetrics fm( m_label.font() );

    int xlabelpos = pos().x();
    int ylabelpos = pos().y();
    m_labelheight = (fm.height()+1)/2;
    m_labelWidth  = fm.width(m_label.text());
    int space = m_space ? m_space : ( m_labelheight < 4 ) ? 2 : m_labelheight/2;
    int offset = m_length + space;

    if( m_angle == 0 )         // Right side
    {
        m_label.setRotation( 0 );
        if( m_Hflip == -1 ) xlabelpos -= offset;
        else                xlabelpos -= m_labelWidth+offset;
        if( m_Vflip == -1 ) ylabelpos += m_labelheight;
        else                ylabelpos -= m_labelheight;
    }
    else if( m_angle == 90 )   // Top
    {
        m_label.setRotation(m_angle);
        if( m_Hflip == -1 ) xlabelpos -= m_labelheight;
        else                xlabelpos += m_labelheight;
        if( m_Vflip == -1 ) ylabelpos += m_labelWidth+offset;
        else                ylabelpos += offset;
    }
    else if( m_angle == 180 )  // Left
    {
        m_label.setRotation( 0 );
        if( m_Hflip == -1 ) xlabelpos += m_labelWidth+offset;
        else                xlabelpos += offset;
        if( m_Vflip == -1 ) ylabelpos += m_labelheight;
        else                ylabelpos -= m_labelheight;
    }
    else if( m_angle == 270 )  // Bottom
    {
        m_label.setRotation( m_angle );
        if( m_Hflip == -1 ) xlabelpos += m_labelheight;
        else                xlabelpos -= m_labelheight;
        if( m_Vflip == -1 ) ylabelpos -= m_labelWidth+offset;
        else                ylabelpos -= offset;
    }
    m_label.setPos( xlabelpos, ylabelpos );
    m_label.setTransform( QTransform::fromScale( m_Hflip, m_Vflip ) );
    update();
}

void Pin::setLabelColor( QColor color ) { m_label.setBrush( QBrush(color) ); }

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
    /// if( !bus ) return;          // Why?
    m_isBus = bus;
    
    if( my_connector ) my_connector->setIsBus( true );
    if( m_conPin ) m_conPin->setIsBus( true );
    
    m_component->inStateChanged( 2 );         // Propagate Is Bus (Node)

    update();
}

void Pin::setVisible( bool visible )
{
    m_label.setVisible( visible );
    QGraphicsItem::setVisible( visible );
}

void Pin::warning( bool w )
{
    m_warning = w;
    if     (  m_warning ) Simulator::self()->addToUpdateList( this );
    else if( !m_animate ) Simulator::self()->remFromUpdateList( this );
    update();
}

void Pin::animate( bool an )
{
    if( m_unused || m_isBus ) return;
    m_animate = an;

    if     (  m_animate ) Simulator::self()->addToUpdateList( this );
    else if( !m_warning ) Simulator::self()->remFromUpdateList( this );

    update();
}

void Pin::updateStep()
{
    if( m_unused ) return;
    //if( m_PinChanged )
        update();
}

void Pin::paint( QPainter* painter, const QStyleOptionGraphicsItem* option, QWidget* widget )
{
    if( !isVisible() ) return;
    //m_PinChanged = false;

    /*QPen pen0( m_color[0], 0, Qt::SolidLine, Qt::RoundCap, Qt::RoundJoin );
    painter->setPen(pen0);
    painter->setBrush( Qt::red );
    painter->drawRect( boundingRect() );*/

    if( m_warning )
    {
        static double opaci = 0.3;
        static double speed = 0.05;
        if( Simulator::self()->isRunning() ) m_opCount += speed;
        else                                 m_opCount = 0.65;

        painter->setOpacity( m_opCount+opaci );
        if( m_opCount > 0.7 ) m_opCount = 0.0;

        painter->fillRect( QRect(-4, -4, m_length+4, 8 ), QColor(200, 100, 0, 240) );
    }

    if( m_overScore > -1 )
    {
        qreal x = m_length+2.5;
        qreal y = -m_labelheight+1;
        qreal width = m_labelWidth-1;
        if( m_angle == 0 ) y = -y; // Right

        QPen pen( m_label.brush().color(), 0.5, Qt::SolidLine, Qt::RoundCap, Qt::RoundJoin );
        painter->setPen( pen );
        painter->drawLine( QPointF( x, y ), QPointF( x+width, y ) );
    }
    QPen pen( m_color[0], 3, Qt::SolidLine, Qt::RoundCap, Qt::RoundJoin );

    if     ( m_unused  ) pen.setColor( QColor( 75, 120, 170 ));
    else if( m_isBus   ) pen.setColor( Qt::darkGreen );
    else if( m_animate )
    {
        if( m_pinState == undef_state )
            pen.setColor( (getVoltage() > 2.5) ? m_color[out_high] : m_color[out_low] );
        else pen.setColor( m_color[m_pinState] );
    }

    painter->setPen(pen);
    if( m_length > 1 ) painter->drawLine( 0, 0, m_length-1, 0);
    else               painter->drawLine( QPointF(-0.01, 0 ), QPointF( 0.03, 0 ));

    if( m_inverted )
    {
        painter->setBrush( Qt::white );
        QPen pen = painter->pen();
        pen.setWidthF( 1.8 );
        painter->setPen(pen);
        QRectF rect( 3.5,-2.2, 4.4, 4.4 );
        painter->drawEllipse(rect);
    }
    if( !m_unused && m_animate )
    {
        pen.setWidthF( 1.5 );
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
