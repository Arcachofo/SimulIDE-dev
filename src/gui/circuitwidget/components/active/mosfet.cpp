/***************************************************************************
 *   Copyright (C) 2016 by Santiago González                               *
 *                                                                         *
 ***( see copyright.txt file at root folder )*******************************/

#include <QPainter>

#include "mosfet.h"
#include "simulator.h"
#include "circuit.h"
#include "itemlibrary.h"
#include "iopin.h"

#include "doubleprop.h"
#include "boolprop.h"

Component* Mosfet::construct( QObject* parent, QString type, QString id )
{ return new Mosfet( parent, type, id ); }

LibraryItem* Mosfet::libraryItem()
{
    return new LibraryItem(
        QCoreApplication::translate("Mosfet", "Mosfet"),
        "Transistors",
        "mosfet.png",
        "Mosfet",
        Mosfet::construct);
}

Mosfet::Mosfet( QObject* parent, QString type, QString id )
      : Component( parent, type, id )
      , eMosfet( id )
{
    m_area = QRectF(-12, -14, 28, 28 );
    setLabelPos( 18, 0, 0 );

    m_pin.resize(3);
    m_ePin[0] = m_pin[0] = new Pin(  90, QPoint(8,-16), id+"-Dren", 0, this );
    m_ePin[1] = m_pin[1] = new Pin( 270, QPoint(8, 16), id+"-Sour", 1, this );
    m_ePin[2] = m_pin[2] = new IoPin( 180, QPoint(-16, 0), id+"-Gate", 0, this, input );
    
    Simulator::self()->addToUpdateList( this );

    addPropGroup( { tr("Main"), {
new BoolProp<Mosfet>("P_Channel", tr("P Channel"),"", this, &Mosfet::pChannel,  &Mosfet::setPchannel ),
new BoolProp<Mosfet>("Depletion", tr("Depletion"),"", this, &Mosfet::depletion, &Mosfet::setDepletion ),
    },0} );
    addPropGroup( { tr("Electric"), {
new DoubProp<Mosfet>("RDSon"    , tr("RDSon")    ,"Ω", this, &Mosfet::rdson,     &Mosfet::setRDSon),
new DoubProp<Mosfet>("Threshold", tr("Threshold"),"V", this, &Mosfet::threshold, &Mosfet::setThreshold )
    },0} );
}
Mosfet::~Mosfet(){}

void Mosfet::updateStep()
{
    if( Circuit::self()->animate() ) update();

    if( !m_changed ) return;
    m_changed = false;

    voltChanged();
}

void Mosfet::paint( QPainter* p, const QStyleOptionGraphicsItem* option, QWidget* widget )
{
    Component::paint( p, option, widget );
    
    if( Circuit::self()->animate() && m_gateV > 0 )  p->setBrush( Qt::yellow );
    else                                             p->setBrush( Qt::white );

    p->drawEllipse( m_area );
    
    p->drawLine(-12, 0,-4, 0 );
    p->drawLine( -4,-8,-4, 8 );
    
    p->drawLine( 0,-7, 8,-7 );
    p->drawLine( 0, 0, 8, 0 );
    p->drawLine( 0, 7, 8, 7 );
    
    p->drawLine( 8,-12, 8,-7 );
    p->drawLine( 8, 12, 8, 0 );
    
    p->setBrush( Qt::black );
    if( m_Pchannel )
    {
         QPointF points[3] = {
         QPointF( 7, 0 ),
         QPointF( 3,-2 ),
         QPointF( 3, 2 ) };
         p->drawPolygon( points, 3 );
    } else {
        QPointF points[3] = {
        QPointF( 1, 0 ),
        QPointF( 5,-2 ),
        QPointF( 5, 2 )     };
        p->drawPolygon( points, 3 );
    }
    if( m_depletion )
    {
        QPen pen(Qt::black, 2.0, Qt::SolidLine, Qt::RoundCap, Qt::RoundJoin);
        p->setPen( pen );
        p->drawLine( 0,-9, 0, 9 );
    } else {
        p->drawLine( 0,-9, 0,-5 );
        p->drawLine( 0,-2, 0, 2 );
        p->drawLine( 0, 5, 0, 9 );
}   }
