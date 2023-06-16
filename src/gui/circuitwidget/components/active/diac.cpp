/***************************************************************************
 *   Copyright (C) 2022 by Santiago González                               *
 *                                                                         *
 ***( see copyright.txt file at root folder )*******************************/

#include <QPainter>
#include <QtMath>
#include <QDebug>

#include "diac.h"
#include "itemlibrary.h"
#include "simulator.h"
#include "circuit.h"
#include "e-diode.h"
#include "e-node.h"
#include "pin.h"

#include "doubleprop.h"

Component* Diac::construct( QObject* parent, QString type, QString id )
{ return new Diac( parent, type, id ); }

LibraryItem* Diac::libraryItem()
{
    return new LibraryItem(
        QCoreApplication::translate( "Diac", "Diac" ),
        "Rectifiers",
        "diac.png",
        "Diac",
        Diac::construct );
}

Diac::Diac( QObject* parent, QString type, QString id )
     : Component( parent, type, id )
     , eElement( id )
{
    m_area =  QRectF( -8, -16, 16, 32 );
    setLabelPos(-12,-30, 0 );

    m_resOn    = 500;
    m_resOff   = 1e8;
    m_brkVolt  = 30;
    m_holdCurr = 0.01;

    // Pin0--  --ePin0--diode1--ePin1--midEnode--ePin4--resistor--Pin1
    //         --ePin2--diode2--ePin3--
    m_pin.resize( 2 );
    m_pin[0] = new Pin( 180, QPoint(-16, 0 ), id+"-lPin", 0, this);
    m_pin[1] = new Pin( 0,   QPoint( 16, 0 ), id+"-rPin", 1, this);

    setNumEpins( 5 );

    m_diode1 = new eDiode( id+"-dio1" );
    m_diode1->setEpin( 0, m_ePin[0] );
    m_diode1->setEpin( 1, m_ePin[1] );
    m_diode1->setModel( "Diode Default" );

    m_diode2 = new eDiode( id+"-dio2" );
    m_diode2->setEpin( 0, m_ePin[2] );
    m_diode2->setEpin( 1, m_ePin[3] );
    m_diode2->setModel( "Diode Default" );

    m_resistor = new eResistor( m_elmId+"-resistor");
    m_resistor->setEpin( 0, m_ePin[4] );
    m_resistor->setEpin( 1, m_pin[1] );

    Simulator::self()->addToUpdateList( this );

    addPropGroup( { tr("Main"), {
new DoubProp<Diac>( "ResOn"   , tr("On Resistance")    ,"Ω", this, &Diac::resOn ,   &Diac::setResOn ),
new DoubProp<Diac>( "ResOff"  , tr("Off Resistance")   ,"Ω", this, &Diac::resOff,   &Diac::setResOff ),
new DoubProp<Diac>( "BrkVolt" , tr("Breakdown Voltage"),"V", this, &Diac::brkVolt,  &Diac::setBrkVolt ),
new DoubProp<Diac>( "HoldCurr", tr("Hold Current")     ,"A", this, &Diac::holdCurr, &Diac::setHoldCurr )
    },0} );
}
Diac::~Diac()
{
    delete m_diode1;
    delete m_diode2;
    delete m_resistor;
}

void Diac::initialize()
{
    m_midEnode = new eNode( m_elmId+"-mideNode");
}

void Diac::stamp()
{
    m_state = false;

    eNode* node0 = m_pin[0]->getEnode();
    eNode* node1 = m_pin[1]->getEnode();

    m_diode1->getEpin(0)->setEnode( node0 );
    m_diode1->getEpin(1)->setEnode( m_midEnode );

    m_diode2->getEpin(0)->setEnode( m_midEnode );
    m_diode2->getEpin(1)->setEnode( node0 );

    m_resistor->getEpin(0)->setEnode( m_midEnode );

    if( node0 ) node0->addToNoLinList( this );
    if( node1 ) node1->addToNoLinList( this );

    m_resistor->setRes( m_resOff );
}

void Diac::updateStep()
{
    if( !m_changed ) return;
    m_changed = false;

    double res = m_state ? m_resOn : m_resOff;
    m_resistor->setRes( res );
    voltChanged();
}

void Diac::voltChanged()
{
    double voltage = m_pin[0]->getVoltage()-m_pin[1]->getVoltage();
    double current = m_resistor->current();// - m_diode2->getCurrent();
    bool state = m_state;

    if( qFabs(current) < m_holdCurr ) state = false; /// Mingw needs qFabs
    if( qFabs(voltage) > m_brkVolt  ) state = true;

    if( m_state == state ) return;
    Simulator::self()->notCorverged();

    //qDebug() << "Diac::voltChanged" << state;
    m_state = state;
    double res = state ? m_resOn : m_resOff;
    m_resistor->setRes( res );
}

void Diac::paint( QPainter* p, const QStyleOptionGraphicsItem* option, QWidget* widget )
{
    Component::paint( p, option, widget );

    p->setBrush( Qt::black );

 static const QPointF points1[3] = {
        QPointF( 7,-8  ),
        QPointF(-8,-15 ),
        QPointF(-8, 0  )          };
    p->drawPolygon( points1, 3 );

 static const QPointF points2[3] = {
        QPointF(-7, 8  ),
        QPointF( 8, 0 ),
        QPointF( 8, 15 )          };
    p->drawPolygon( points2, 3 );

    QPen pen = p->pen();
    pen.setWidth(3);
    p->setPen(pen);

    p->drawLine(-8,-16,-8, 16 );
    p->drawLine( 8,-16, 8, 16 );
}
