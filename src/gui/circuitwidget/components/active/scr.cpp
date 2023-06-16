/***************************************************************************
 *   Copyright (C) 2022 by Santiago González                               *
 *                                                                         *
 ***( see copyright.txt file at root folder )*******************************/

#include <QPainter>

#include <QtMath>

#include "scr.h"
#include "itemlibrary.h"
#include "simulator.h"
#include "circuit.h"
#include "e-diode.h"
#include "e-node.h"
#include "pin.h"

#include "doubleprop.h"

Component* SCR::construct( QObject* parent, QString type, QString id )
{ return new SCR( parent, type, id ); }

LibraryItem* SCR::libraryItem()
{
    return new LibraryItem(
        QCoreApplication::translate("SCR", "SCR"),
        "Rectifiers",
        "scr.png",
        "SCR",
        SCR::construct );
}

SCR::SCR( QObject* parent, QString type, QString id )
     : Component( parent, type, id )
     , eElement( id )
{
    m_area =  QRectF( -8, -16, 16, 32 );
    setLabelPos(-12,-30, 0 );

    m_holdCurr = 0.0082;
    m_trigCurr = 0.01;
    m_gateRes  = 100;

    m_pin.resize( 3 );
    m_pin[0] = new Pin( 180, QPoint(-16, 0 ), id+"-lPin", 0, this);
    m_pin[1] = new Pin( 0,   QPoint( 16, 0 ), id+"-rPin", 1, this);
    m_pin[2] = new Pin( 0,   QPoint( 16, 8 ), id+"-gPin", 2, this);
    m_pin[2]->setPinAngle(-40 );
    m_pin[2]->setLength( 12 );

    setNumEpins( 4 );

    // Pin0--diode1--ePin0--midEnode--ePin1--resistor--Pin1
    // Pin2-------------------------resistGa--ePin2---|
    m_diode = new eDiode( id+"-dio1" );
    m_diode->setEpin( 0, m_pin[0] );
    m_diode->setEpin( 1, m_ePin[0] );
    m_diode->setModel( "Diode Default" );

    m_resistor = new eResistor( m_elmId+"-resistorA");
    m_resistor->setEpin( 0, m_ePin[1] );
    m_resistor->setEpin( 1, m_pin[1] );

    m_resistGa = new eResistor( m_elmId+"-resistorC");
    m_resistGa->setEpin( 0, m_pin[2] );
    m_resistGa->setEpin( 1, m_ePin[2] );

    Simulator::self()->addToUpdateList( this );

    addPropGroup( { tr("Main"), {
new DoubProp<SCR>("GateRes" , tr("Gate Resistance"),"Ω", this, &SCR::gateRes , &SCR::setGateRes ),
new DoubProp<SCR>("TrigCurr", tr("Trigger Current"),"A", this, &SCR::trigCurr, &SCR::setTrigCurr ),
new DoubProp<SCR>("HoldCurr", tr("Holding Current"),"A", this, &SCR::holdCurr, &SCR::setHoldCurr )
    },0} );
}
SCR::~SCR()
{
    delete m_diode;
    delete m_resistor;
    delete m_resistGa;
}

void SCR::initialize()
{
    m_midEnode = new eNode( m_elmId+"-mideNode");
}

void SCR::stamp()
{
    // Pin0--diode1--ePin0--midEnode--ePin1--resistor--Pin1
    // Pin2-------------------------resistGa--ePin2---|
    m_state = false;

    eNode* nodeA = m_pin[0]->getEnode();
    eNode* nodeC = m_pin[1]->getEnode();
    eNode* nodeG = m_pin[2]->getEnode();

    //m_diode->getEpin(0)->setEnode( nodeA );
    m_diode->getEpin(1)->setEnode( m_midEnode );

    m_resistor->getEpin(0)->setEnode( m_midEnode );
    m_resistGa->getEpin(1)->setEnode( nodeC );

    if( nodeA ) nodeA->addToNoLinList( this );
    if( nodeC ) nodeC->addToNoLinList( this );
    if( nodeG ) nodeG->addToNoLinList( this );

    m_resistor->setRes( 10e5 );
    m_resistGa->setRes( m_gateRes );
}

void SCR::updateStep()
{
    if( !m_changed ) return;
    m_changed = false;

    voltChanged();
}

void SCR::voltChanged()
{
    double voltAC = m_pin[0]->getVoltage() - m_pin[1]->getVoltage();
    double voltGC = m_pin[2]->getVoltage() - m_pin[1]->getVoltage();
    if( qFabs( voltAC ) < .01 && qFabs( voltGC ) < .01 ) return;  // Converged
    Simulator::self()->notCorverged();

    double anodCurr =  m_resistor->current();
    double cathCurr = -m_resistGa->current() - anodCurr;
    double cmult = 1/m_trigCurr;
    double amult = 1/m_holdCurr - cmult;

    bool state = -cathCurr*cmult + anodCurr*amult > 1;

    if( m_state != state )
    {
        m_state = state;
        double res = state ? 0.0105 : 10e5;
        m_resistor->setRes( res );
    }
}

void SCR::paint( QPainter* p, const QStyleOptionGraphicsItem* option, QWidget* widget )
{
    Component::paint( p, option, widget );

    p->setBrush( Qt::black );

 static const QPointF points[3] = {
        QPointF( 7, 0 ),
        QPointF(-8,-7 ),
        QPointF(-8, 7 )              };
    p->drawPolygon(points, 3);

    QPen pen = p->pen();
    pen.setWidth( 3 );
    p->setPen( pen );
    p->drawLine( 7, -6, 7, 6 );
}
