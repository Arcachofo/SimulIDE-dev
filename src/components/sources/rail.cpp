/***************************************************************************
 *   Copyright (C) 2012 by Santiago González                               *
 *                                                                         *
 ***( see copyright.txt file at root folder )*******************************/

#include <QPainter>

#include "rail.h"
#include "simulator.h"
#include "iopin.h"
#include "itemlibrary.h"
#include "pin.h"

#include "doubleprop.h"

#define tr(str) simulideTr("Rail",str)

Component* Rail::construct( QString type, QString id )
{ return new Rail( type, id ); }

LibraryItem* Rail::libraryItem()
{
    return new LibraryItem(
        tr("Rail"),
        "Sources",
        "rail.png",
        "Rail",
        Rail::construct );
}

Rail::Rail( QString type, QString id )
    : Component( type, id )
    , eElement( id )
{
    setLabelPos(-64,-24 );

    m_area = QRect(-2, -8, 12, 16 );
    m_changed = false;

    m_pin.resize(1);
    m_pin[0] = m_out = new IoPin( 0, QPoint(16,0), id+"-outnod", 0, this, source );

    setRotation( 90 );
    setValLabelPos(-12, 6,-90 ); // x, y, rot
    setLabelPos(-5,-10,-90);

    addPropGroup( { tr("Main"), {
        new DoubProp<Rail>("Voltage", tr("Voltage"), "V"
                          , this, &Rail::volt, &Rail::setVolt )
    }, 0} );

    setShowProp("Voltage");
    setPropStr( "Voltage", "5" );
}
Rail::~Rail() { delete m_out; }

void Rail::setVolt( double v )
{
    Simulator::self()->pauseSim();
    m_volt = v;
    stamp();
    Simulator::self()->resumeSim();
}

void Rail::stamp()
{
    m_out->setOutHighV( m_volt );
    m_out->setOutState( true );
    update();
}

QPainterPath Rail::shape() const
{
    QPainterPath path;

    QVector<QPointF> points;

    points << QPointF(-4,-8 )
           << QPointF(-4, 8 )
           << QPointF( 8,  1 )
           << QPointF( 8, -1 );

    path.addPolygon( QPolygonF(points) );
    path.closeSubpath();
    return path;
}

void Rail::paint( QPainter* p, const QStyleOptionGraphicsItem* o, QWidget* w )
{
    Component::paint( p, o, w );
    p->setBrush( QColor( 255, 166, 0 ) );
    //p->drawRoundedRect( QRectF( -8, -8, 16, 16 ), 2, 2);
    static const QPointF points[4] = {
        QPointF(-1.5,-6.5 ),
        QPointF(-1.5, 6.5 ),
        QPointF( 9,  1 ),
        QPointF( 9, -1 )            };

    p->drawPolygon(points, 4);

    Component::paintSelected( p );
}
