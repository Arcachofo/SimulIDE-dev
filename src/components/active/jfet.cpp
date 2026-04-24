/***************************************************************************
 *   Copyright (C) 2016 by Santiago González                               *
 *                                                                         *
 ***( see copyright.txt file at root folder )*******************************/

#include <QPainter>

#include "jfet.h"
#include "simulator.h"
#include "circuit.h"
#include "itemlibrary.h"
#include "iopin.h"

#include "doubleprop.h"

#define tr(str) simulideTr("Jfet",str)

Component* Jfet::construct( QString type, QString id )
{ return new Jfet( type, id ); }

LibraryItem* Jfet::libraryItem()
{
    return new LibraryItem(
        tr("Jfet"),
        "Transistors",
        "jfet.png",
        "Jfet",
        Jfet::construct);
}

Jfet::Jfet( QString type, QString id )
    : Component( type, id )
    , eJfet( id )
{
    m_area = QRectF(-12, -14, 28, 28 );
    setLabelPos( 18, 0, 0 );

    m_pin.resize(3);
    m_ePin[0] = m_pin[0] = new Pin(  90, QPoint(8,-16), id+"-Dren", 0, this );
    m_ePin[1] = m_pin[1] = new Pin( 270, QPoint(8, 16), id+"-Sour", 1, this );
    m_ePin[2] = m_pin[2] = new IoPin( 180, QPoint(-16, 0), id+"-Gate", 0, this, input );

    // Pin for debug monitoring of Rds for voltage controlled resistor applications
    // m_ePin[3] = m_pin[3] = new IoPin( 0, QPoint(16, 0), id+"-admit", 0, this, output );
    
    Simulator::self()->addToUpdateList( this );

    addPropGroup( { tr("Electric"), {
        new DoubProp<Jfet>("Idss"    , tr("Idss"),"A"
                            , this, &Jfet::Idss, &Jfet::setIdss),

        new DoubProp<Jfet>("Vp", tr("Vp"),"V"
                            , this, &Jfet::Vp, &Jfet::setVp ),

        new DoubProp<Jfet>("1/Lambda", tr("1/Lambda"),"V"
                            , this, &Jfet::LambdaInv, &Jfet::setLambdaInv )},0} );
}
Jfet::~Jfet(){}

void Jfet::updateStep()
{
    if( Circuit::self()->animateCurr() ) update();

    if( !m_changed ) return;
    m_changed = false;

    updateValues();
    voltChanged();
}

void Jfet::paint( QPainter* p, const QStyleOptionGraphicsItem* o, QWidget* w )
{
    Component::paint( p, o, w);
    
    if( Circuit::self()->animateCurr() && m_gateV > 0 ) p->setBrush( Qt::yellow );
    else                                                p->setBrush( QColor( *m_backColor ) );

    p->drawEllipse( m_area );
    
    p->drawLine(-12, 0, 0, 0 );
    p->drawLine( 0,-9, 0, 9 );
    
    p->drawLine( 0,-7, 8,-7 );
    p->drawLine( 0, 7, 8, 7 );
    
    p->drawLine( 8,-12, 8,-7 );
    p->drawLine( 8, 12, 8, 8 );
    
    p->setBrush( QColor( *m_foreColor ) );

    QPointF points[3] = {
        QPointF( -1, 0 ),
        QPointF( -5,-2 ),
        QPointF( -5, 2 ) };
    p->drawPolygon( points, 3 );

    Component::paintSelected( p );
}
