/***************************************************************************
 *   Copyright (C) 2022 by Santiago González                               *
 *                                                                         *
 ***( see copyright.txt file at root folder )*******************************/

#include <math.h>

#include <QGraphicsProxyWidget>
#include <QPainter>

#include "ky040.h"
#include "customdial.h"
#include "custombutton.h"
#include "iopin.h"
#include "simulator.h"
#include "circuit.h"
#include "circuitwidget.h"
#include "itemlibrary.h"

#include "intprop.h"

#define WIDTH 40
#define HEIGHT 56
#define GAP 0
#define DIAL_SIZE 36

#define RESOLUTION 10

#define VIN 5

Component* KY040::construct( QObject* parent, QString type, QString id )
{ return new KY040( parent, type, id ); }

LibraryItem* KY040::libraryItem()
{
    return new LibraryItem(
        tr("Rotary Encoder (relative)"),
        "Perifericals",
        "ky-040.png",
        "KY040",
        KY040::construct);
}

KY040::KY040( QObject* parent, QString type, QString id )
     : Component( parent, type, id )
     , eElement( id )
{
    m_changed = false;
    m_area = QRect( -WIDTH/2, -HEIGHT/2 + GAP, WIDTH, HEIGHT );

    m_dial = new CustomDial();
    m_dial->setFixedSize( DIAL_SIZE, DIAL_SIZE );
    m_dial->setWrapping( true );
    m_dial->setMinimum( 1 );
    m_dial->setValue( 1 );
    m_dial->setSingleStep( 1 );
    m_dial->setNotchTarget( 10 );
    setSteps( 20 );
    
    m_proxy = Circuit::self()->addWidget( m_dial );
    m_proxy->setParentItem( this );
    m_proxy->setPos( QPoint(-WIDTH/2+(WIDTH-DIAL_SIZE)/2, -HEIGHT/2+(WIDTH-DIAL_SIZE)/2+GAP) );
    
    m_button = new CustomButton();
    m_button->setMaximumSize( 10, 10 );
    m_button->setGeometry(-10,-10, 10, 10 );
    
    m_proxy_button = Circuit::self()->addWidget( m_button );
    m_proxy_button->setParentItem( this );
    m_proxy_button->setPos( QPoint(8, HEIGHT/2-13) );

    m_pin.resize(3);

    m_pin[0] = m_pinA = new IoPin( 270, QPoint(4,36), id+"-clk", 0, this, output );
    m_pinA->setOutHighV( VIN );
    m_pinA->setLabelText( "CLK" );

    m_pin[1] = m_pinB = new IoPin( 270, QPoint(-4,36), id+"-dt", 0, this, output );
    m_pinB->setOutHighV( VIN );
    m_pinB->setLabelText( "DT" );

    m_pin[2] = m_sw = new IoPin( 270, QPoint(-12,36), id+"-sw", 0, this, output );
    m_sw->setOutHighV( VIN );
    m_sw->setLabelText( "SW" );

    setAngle( 90 );
    setLabelPos(-34, 20,-90 );

    Simulator::self()->addToUpdateList( this );

    connect( m_button, &QToolButton::pressed,
             this,     &KY040::onbuttonchanged );
    
    connect( m_button, &QToolButton::released,
             this,     &KY040::onbuttonchanged );

    addPropGroup( { tr("Main"), {
new IntProp<KY040>( "Steps", tr("Steps per Rotation"),tr("_Steps"), this, &KY040::steps, &KY040::setSteps,0,"uint" )
    }, groupNoCopy} );
}
KY040::~KY040(){}

void KY040::stamp()
{
    m_dial->setValue( 1 );
    m_prevDialVal = 1;

    m_posA = 5;
    m_posB = 0;

    m_stateA = false;
    m_stateB = false;

    m_pinA->setOutState( false );
    m_pinB->setOutState( false );
    m_sw->setOutState( true );
}

void KY040::updateStep()
{
    Simulator::self()->cancelEvents( this );

    int val = m_dial->value();
    if( m_prevDialVal != val )
    {
        int max = m_dial->maximum() ;
        int bot = max/4;
        int top = max-bot;

        if     ( m_prevDialVal > top && val < bot ) m_prevDialVal -= max;
        else if( m_prevDialVal < bot && val > top ) m_prevDialVal += max;
        int dialDelta = val - m_prevDialVal;
        m_prevDialVal = val;

        if( dialDelta > 0 ) m_delta = 1;// Clockwise
        else                m_delta = -1;

        uint64_t spf = Simulator::self()->psPerFrame();
        m_stepDelta = spf/fabs(dialDelta);

        //qDebug()<<"\n" << dialDelta << m_stepDelta/1e6;

        Simulator::self()->addEvent( m_stepDelta, this );
    }

    if( !m_changed ) return;
    m_sw->setOutState( !m_button->isDown() );
    m_changed = false;
}

void KY040::runEvent()
{
    m_posA += m_delta;
    if     ( m_posA > 19 ) m_posA = 0;
    else if( m_posA < 0  ) m_posA = 19;
    m_posB += m_delta;
    if     ( m_posB > 19 ) m_posB = 0;
    else if( m_posB < 0  ) m_posB = 19;

    bool stateA = m_posA > 9;
    bool stateB = m_posB > 9;

    if( m_stateA != stateA )
    {
        m_stateA = stateA;
        m_pinA->setOutState( stateA );
    }
    if( m_stateB != stateB )
    {
        m_stateB = stateB;
        m_pinB->setOutState( stateB );
    }
    Simulator::self()->addEvent( m_stepDelta, this );
}

void KY040::onbuttonchanged()
{
    m_changed = true;
    update();
}

void KY040::setSteps( int s )
{
    if( Simulator::self()->isRunning() ) CircuitWidget::self()->powerCircOff();
    m_steps = s;
    //m_deltaS = 5;
    m_dial->setMaximum( s*2*RESOLUTION );
}

void KY040::paint( QPainter* p, const QStyleOptionGraphicsItem* option, QWidget* widget )
{
    Component::paint( p, option, widget );
    
    p->setBrush(QColor( 50, 50, 70 ));
    p->drawRoundedRect( m_area, 2, 2 );
}
