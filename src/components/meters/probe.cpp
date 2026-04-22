/***************************************************************************
 *   Copyright (C) 2012 by Santiago González                               *
 *                                                                         *
 ***( see copyright.txt file at root folder )*******************************/

#include "circuitwidget.h"
#include <QtMath>
#include <QPainter>
#include <QMenu>

#include "probe.h"
#include "connector.h"
#include "connectorline.h"
#include "circuit.h"
#include "simulator.h"
#include "itemlibrary.h"
#include "label.h"
#include "iopin.h"

#include "doubleprop.h"
#include "boolprop.h"

#define tr(str) simulideTr("Probe",str)

Component* Probe::construct( QString type, QString id )
{ return new Probe( type, id ); }

LibraryItem* Probe::libraryItem()
{
    return new LibraryItem(
        tr("Probe"),
        "Meters",
        "probe.png",
        "Probe",
        Probe::construct );
}

Probe::Probe( QString type, QString id )
     : Component( type, id )
     , eElement( id )
{
    setZValue( 200 );
    m_area = QRect(-12,-8, 20, 16 );
    m_graphical = true;
    m_pauseState = false;

    m_voltTrig = 2.5;
    m_voltIn = 0;

    m_pin.resize(1); // Create Input Pin
    m_pin[0] = m_inputPin = new IoPin( 180, QPoint(-22,0), id+"-inpin", 0, this, undef_mode );
    m_inputPin->setBoundingRect( QRect(-1, -1, 2, 2) );
    m_inputPin->setImpedance( 1e9 );

    setValLabelPos( 16, 0, 45 ); // x, y, rot
    setShowVal( true );
    setLabelPos( 16,-16, 45 );
    setRotation( rotation() - 45 );
    m_voltIn = -1; // Force update
    setVolt( 0 );

    setSmall( false );

    Simulator::self()->addToUpdateList( this );

    addPropGroup( { tr("Main"), {
        new BoolProp<Probe>("ShowVolt" , tr("Show Voltage"), ""
                           , this, &Probe::showVal, &Probe::setShowVal, propNoCopy ),

        new DoubProp<Probe>("Threshold", tr("Threshold"), "V"
                           , this, &Probe::threshold, &Probe::setThreshold ),

        new BoolProp<Probe>("Small", tr("Small size"), ""
                           , this, &Probe::isSmall, &Probe::setSmall ),

        new BoolProp<Probe>("Pause", "", ""
                           , this, &Probe::pauseState, &Probe::setPauseState, propHidden )
    }, 0 } );
}
Probe::~Probe(){}

void Probe::stamp()
{
    m_state = false;
    m_inputPin->changeCallBack( this, m_pauseState );
}

void Probe::updateStep()
{
    if( !Simulator::self()->isRunning() ) { setVolt( 0.0 ); return; }

    if( m_inputPin->isConnected() )// Voltage from connected pin
    {
         setVolt( m_inputPin->getVoltage() );
         return;
    }
    QList<QGraphicsItem*> list = m_inputPin->collidingItems(); // Voltage from connector or Pin behind inputPin
    if( list.isEmpty() ) { setVolt( 0.0 ); return; }

    for( QGraphicsItem* it : list )
    {
        if( it->type() == UserType+3 )                    // Pin found
        {
            Pin* pin =  qgraphicsitem_cast<Pin*>( it );
            setVolt( pin->getVoltage() );
            break;
        }else if( it->type() == UserType+2 )        // ConnectorLine
        {
            ConnectorLine* line =  qgraphicsitem_cast<ConnectorLine*>( it );
            Connector* con = line->connector();
            setVolt( con->getVoltage() );
            break;
}   }   }

void Probe::voltChanged()
{
    if( !m_pauseState ) return;

    bool state = m_inputPin->getInpState();
    if( m_state == state ) return;
    m_state = state;
    CircuitWidget::self()->pauseCirc();
}

void Probe::setVolt( double volt )
{
    if( m_voltIn == volt ) return;
    m_voltIn = volt;
    update();       // Repaint

    if( !m_showVal ) return;
    if( qFabs(volt) < 0.01 ) volt = 0;
    
    float v = ( volt > 0 ) ? 0.5 : -0.5;
    v = float(int( v+volt*100 ))/100;
    setValLabelText( QString("%1 V").arg(v) );
}

void Probe::rotateCW()  { rotateAngle( 45 ); }
void Probe::rotateCCW() { rotateAngle(-45 );}

void Probe::rotateAngle( double a )
{
    Component::rotateAngle( a );
    m_idLabel->rotateAngle(-a );
    m_valLabel->rotateAngle(-a );
    updtPinSize();
}

void Probe::setAngle( double angle )
{
    Component::setAngle( angle );
    updtPinSize();
}

void Probe::setSmall( bool s )
{
    m_small = s;

    QFont font = m_idLabel->font();

    if( s ){
        m_area = QRect(-16, -4, 8, 8 );
        font.setPixelSize( 7 );
    }else{
        m_area = QRect(-12,-8, 20, 16 );
        font.setPixelSize( 8 );
    }
    m_idLabel->setFont( font );

    updtPinSize();
}

void Probe::updtPinSize()
{
    int length = 0;
    int an = (int)rotation()%90;

    if( m_small ) length = an ? 6 : 8;
    else          length = an ? 14 : 16;
    m_inputPin->setLength( length );

    int xPos = an? -22 : -24;
    m_inputPin->setPos( xPos, 0 );
    m_inputPin->isMoved();

    Circuit::self()->update();
}

void Probe::slotBreakpoint()
{
    m_pauseState = !m_pauseState;

    m_inputPin->changeCallBack( this, m_pauseState );

    update();
}

void Probe::contextMenu( QGraphicsSceneContextMenuEvent* event, QMenu* menu )
{
    if( m_inputPin->connector() )
    {
        QString iconStr = m_pauseState ? ":/nobreakpoint.png" : ":/breakpoint.png";
        QAction* breakAction = menu->addAction( m_theme->icon( iconStr ),tr("Pause at state change") );
        QObject::connect( breakAction, &QAction::triggered, [=](){ slotBreakpoint(); } );
    }
    menu->addSeparator();
    Component::contextMenu( event, menu );
}

QPainterPath Probe::shape() const
{
    QPainterPath path;
    path.addEllipse( m_area );
    return path;
}
void Probe::paint( QPainter* p, const QStyleOptionGraphicsItem* o, QWidget* w )
{
    Component::paint( p, o, w );

    if      ( m_voltIn > m_voltTrig)  p->setBrush( QColor( 255, 166, 0 ) );
    else if ( m_voltIn < -m_voltTrig) p->setBrush( QColor( 0, 100, 255 ) );
    else                              p->setBrush( QColor( 230, 230, 255 ) );

    if( m_pauseState )
    {
        QPen pen = p->pen();
        pen.setWidthF( 2.5 );
        pen.setColor( QColor( 255, 0, 0 ));
        p->setPen(pen);
    }
    if( m_small ) p->drawEllipse( m_area );
    else          p->drawEllipse( QRect(-8,-8, 16, 16 ) );

    Component::paintSelected( p );
}
