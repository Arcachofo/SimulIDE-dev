/***************************************************************************
 *   Copyright (C) 2012 by Santiago González                               *
 *                                                                         *
 ***( see copyright.txt file at root folder )*******************************/

#include <QPainter>
#include <QMenu>

#include "diode.h"
#include "itemlibrary.h"
#include "simulator.h"
#include "e-node.h"
#include "pin.h"

#include "doubleprop.h"
#include "stringprop.h"

#define tr(str) simulideTr("Diode",str)

Component* Diode::construct( QString type, QString id )
{ return new Diode( type, id ); }

LibraryItem* Diode::libraryItem()
{
    return new LibraryItem(
        tr("Diode"),
        "Rectifiers",
        "diode.png",
        "Diode",
        Diode::construct);
}

Diode::Diode( QString type, QString id, bool zener )
     : LinkerComponent( type, id )
     , eDiode( id )
{
    m_area = QRect(-10, -8, 20, 16 );

    m_pin.resize(2);
    m_pin[0] = new Pin( 180, QPoint(-16, 0 ), id+"-lPin", 0, this);
    m_pin[1] = new Pin( 0,   QPoint( 16, 0 ), id+"-rPin", 1, this);

    setValLabelPos(-16, 6, 0 );
    setLabelPos(-16,-24, 0 );

    m_enumUids = m_enumNames = m_diodes.keys();

    // Pin0--eDiode--ePin1--midEnode--ePin2--eResistor--Pin1
    m_ePin[0] = m_pin[0];
    setNumEpins( 3 );

    m_resistor = new eResistor( m_elmId+"-resistor");
    m_resistor->setEpin( 0, m_ePin[2] );
    m_resistor->setEpin( 1, m_pin[1] );

    m_isZener = zener;
    if( zener ){
        m_diodeType = "zener";
        setModel( "Zener Default" );
    }else{
        m_diodeType = "diode";
        setModel( "Diode Default" );
    }
    Simulator::self()->addToUpdateList( this );

    addPropGroup( { tr("Main"), {
new StrProp<Diode>("Model", tr("Model"),""
                  , this, &Diode::model, &Diode::setModel, 0,"enum" ),
    },0} );

    addPropGroup( { tr("Electric"), {
new DoubProp<Diode>("Threshold" , tr("Forward Voltage"),"V"
                   , this, &Diode::threshold, &Diode::setThreshold ),

new DoubProp<Diode>("MaxCurrent", tr("Max Current"),"A"
                   , this, &Diode::maxCurrent, &Diode::setMaxCurrent ),

new DoubProp<Diode>("Resistance", tr("Resistance"),"Ω"
                   , this, &Diode::res, &Diode::setResSafe ),
    },0} );

    addPropGroup( { tr("Advanced"), {
new DoubProp<Diode>("BrkDownV", tr("Breakdown Voltage"),"V"
                   , this, &Diode::brkDownV, &Diode::setBrkDownV ),

new DoubProp<Diode>("SatCurrent", tr("Saturation Current"),"nA"
                   , this, &Diode::satCur, &Diode::setSatCur ),

new DoubProp<Diode>("EmCoef", tr("Emission Coefficient"),""
                   , this, &Diode::emCoef, &Diode::setEmCoef )
    },0} );
}
Diode::~Diode()
{
    delete m_resistor;
}

bool Diode::setPropStr( QString prop, QString val )
{
    if( prop =="Zener_Volt" ) //  Old: TODELETE
    {
        double zenerV = val.toDouble();
        m_isZener = zenerV > 0;
        eDiode::setBrkDownV( zenerV );
    }
    else return Component::setPropStr( prop, val );
    return true;
}

void Diode::initialize()
{
    m_crashed = false;
    m_warning = false;

    m_midEnode = new eNode( m_elmId+"-mideNode");
    eDiode::initialize();

    update();
}

void Diode::stamp()
{
    m_ePin[1]->setEnode( m_midEnode );
    m_ePin[2]->setEnode( m_midEnode );

    eDiode::stamp();
}

void Diode::updateStep()
{
    if( m_current > m_maxCur ){
        m_warning = true;
        m_crashed = m_current > m_maxCur*2;
        update();
    }else{
        if( m_warning ) update();
        m_warning = false;
        m_crashed = false;
    }
    if( m_changed ) voltChanged(); // m_changed cleared at eDiode::voltChanged
}

void Diode::voltChanged()
{
    eDiode::voltChanged();
    if( !m_converged ) return;

    if( !m_linkedComp.isEmpty() )
    {
        double current = m_resistor->current();
        for( Component* comp : m_linkedComp ) comp->setLinkedValue( current );
    }
}

void Diode::paint( QPainter* p, const QStyleOptionGraphicsItem* option, QWidget* widget )
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
   
    if( m_isZener ){
        p->drawLine( 7,-6, 4,-6 );
        p->drawLine( 7, 6, 10, 6 );
    }
    Component::paintSelected( p );
}
