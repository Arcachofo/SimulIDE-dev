/***************************************************************************
 *   Copyright (C) 2012 by Santiago González                               *
 *                                                                         *
 ***( see copyright.txt file at root folder )*******************************/

#include <QAbstractSlider>
#include <QPainter>
#include <QGraphicsProxyWidget>

#include "potentiometer.h"
#include "dialwidget.h"
#include "propdialog.h"
#include "simulator.h"
#include "circuit.h"
#include "itemlibrary.h"
#include "e-node.h"

#include "doubleprop.h"

#define tr(str) simulideTr("Potentiometer",str)

Component* Potentiometer::construct( QString type, QString id )
{ return new Potentiometer( type, id ); }

LibraryItem* Potentiometer::libraryItem()
{
    return new LibraryItem(
        tr("Potentiometer"),
        "Resistors",
        "potentiometer.png",
        "Potentiometer",
        Potentiometer::construct );
}

Potentiometer::Potentiometer( QString type, QString id )
             : Dialed( type, id )
             , eElement( (id+"-eElement") )
             , m_pinA( 180, QPoint(-16,0 ), id+"-PinA", 0, this )
             , m_pinM( 270, QPoint( 0,16 ), id+"-PinM", 0, this )
             , m_pinB(   0, QPoint( 16,0 ), id+"-PinB", 0, this )
             , m_ePinA( id+"-ePinA", 1 )
             , m_ePinB( id+"-ePinB", 1 )
             , m_resA(  id+"-resA" )
             , m_resB(  id+"-resB" )
{
    m_areaDial = QRectF(-11,-28 , 22, 22 );
    m_areaComp = QRectF(-12,-4.5, 24, 12.5 );
    m_area     = m_areaComp;
    m_graphical = true;

    m_midEnode = nullptr;

    m_pin.resize(3);
    m_pin[0] = &m_pinA;
    m_pin[1] = &m_pinM;
    m_pin[2] = &m_pinB;

    m_pin[0]->setLength( 5 );
    m_pin[2]->setLength( 5 );

    m_resA.setEpin( 0, &m_pinA );
    m_resA.setEpin( 1, &m_ePinA );
    m_resB.setEpin( 1, &m_pinB );
    m_resB.setEpin( 0, &m_ePinB );

    setValLabelPos( 15,-20, 0 );
    setLabelPos(-16,-40, 0);

    Potentiometer::updateProxy();

    addPropGroup( { tr("Main"), {
        new DoubProp<Potentiometer>("Resistance", tr("Resistance"),"Ω"
                                   , this, &Potentiometer::resistance, &Potentiometer::setResistance ),

        new DoubProp<Potentiometer>("Value_Ohm", tr("Current Value"),"Ω"
                                   , this, &Potentiometer::getValue, &Potentiometer::setValue ),
    },0 } );

    addPropGroup( { tr("Dial"), Dialed::dialProps(), groupNoCopy } );

    m_res1 = 0;
    setShowProp("Resistance");
    setPropStr("Resistance", "1 kΩ");
    dialChanged( 500 );
}
Potentiometer::~Potentiometer() {}

void Potentiometer::initialize()
{
    m_midEnode = m_pinM.getEnode();  // Get eNode from middle Pin
    if( !m_midEnode ) m_midEnode = new eNode( m_id+"-mideNode" ); // Not connected: Create mid eNode

    m_pinM.setEnode( m_midEnode );
    m_ePinA.setEnode( m_midEnode );  // Set eNode to internal eResistors ePins
    m_ePinB.setEnode( m_midEnode );

    m_needUpdate = true;
}

void Potentiometer::updateStep()
{
    if( !m_needUpdate ) return;
    m_needUpdate = false;

    double res1 = double( m_resistance*m_dialW.value()/1000 );
    double res2 = m_resistance-res1;

    if( res1 < 1e-6 ){
        res1 = 1e-3;
        res2 = m_resistance-res1;
    }
    if( res2 < 1e-6 ){
        res2 = 1e-6;
        res1 = m_resistance-res2;
    }
    m_resA.setResistance( res1 );
    m_resB.setResistance( res2 );

    if( m_propDialog ) m_propDialog->updtValues();
    else setValLabelText( getPropStr( showProp() ) );
}

double Potentiometer::getValue() { return m_resistance*m_dialW.value()/1000; }

void Potentiometer::setValue( double val )
{
    if( val > m_resistance ) val = m_resistance;
    else if( val < 1e-12 ) val = 1e-12;
    m_dialW.setValue( val*1000/m_resistance );
    m_res1 = val;
    m_needUpdate = true;
    if( !Simulator::self()->isRunning() ) updateStep();
}

void Potentiometer::setResistance( double res ) // Called when property resistance is changed
{
    if( res < 1e-12 ) res = 1e-12;
    m_resistance = res;
    setValue( m_res1 );
}

void Potentiometer::setLinkedValue( double v, int i )
{
    if( i ) setResistance( v );
    else Dialed::setLinkedValue( v, i );
}

void Potentiometer::updateProxy()
{
    m_proxy->setPos( QPoint(-m_dialW.width()/2,-m_dialW.height()-5) );
}

void Potentiometer::paint( QPainter* p, const QStyleOptionGraphicsItem* o, QWidget* w )
{
    if( m_hidden ) return;

    Component::paint( p, o, w );

    p->drawRect( QRectF(-11,-4.5, 22, 9 ));
    QPen pen = p->pen();
    pen.setWidth(3);
    p->setPen(pen);

    p->drawLine( 0, 6,-3, 9 );
    p->drawLine( 0, 6, 3, 9 );

    Component::paintSelected( p );
}
