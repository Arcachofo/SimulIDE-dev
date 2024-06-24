/***************************************************************************
 *   Copyright (C) 2024 by Santiago González                               *
 *                                                                         *
 ***( see copyright.txt file at root folder )*******************************/

#include <QPainter>

#include <QtMath>

#include "comparator.h"
#include "itemlibrary.h"
#include "propdialog.h"
#include "connector.h"
#include "simulator.h"
#include "e-node.h"
#include "iopin.h"

#include "doubleprop.h"
#include "boolprop.h"

#define tr(str) simulideTr("Comparator",str)

Component* Comparator::construct( QString type, QString id )
{ return new Comparator( type, id ); }

LibraryItem* Comparator::libraryItem()
{
    return new LibraryItem(
        tr("Comparator"),
        "Other Active",
        "opamp.png",
        "Comparator",
        Comparator::construct );
}

Comparator::Comparator( QString type, QString id )
          : IoComponent( type, id )
          , eElement( id )
{
    m_width = 4;
    setNumInps( 2, "", 0 );
    setNumOuts( 1, "", 0, false );

    m_area = QRect(-18,-8*2, 36, 8*2*2 );
    setLabelPos(-16,-32, 0);

    m_inPin[0]->setPos( QPoint(-16-8,-8) );
    m_inPin[0]->setFontSize( 9 );
    m_inPin[0]->setSpace( 1.7 );
    m_inPin[0]->setLabelText("+");
    m_inPin[0]->setLabelColor( Qt::red );

    m_inPin[1]->setPos( QPoint(-16-8, 8) );
    m_inPin[1]->setFontSize( 9 );
    m_inPin[1]->setSpace( 1.7 );
    m_inPin[1]->setLabelText("–");  // U+2013
    m_inPin[1]->setLabelColor( QColor( 0, 0, 0 ) );

    m_outPin[0]->setPos( QPoint( 16+8, 0) );

    addPropGroup( { tr("Electric"),
        IoComponent::outputProps()
       +IoComponent::outputType()
        , 0 } );

    addPropGroup( { tr("Timing"), IoComponent::edgeProps(),0 } );
}
Comparator::~Comparator(){}

void Comparator::stamp()
{
    IoComponent::initState();
    for( uint i=0; i<m_inPin.size(); ++i ) m_inPin[i]->changeCallBack( this );
}

void Comparator::voltChanged() // Called when any pin node change volt
{
    double vd = m_inPin[0]->getVoltage()-m_inPin[1]->getVoltage();
    m_nextOutVal = (vd > 0) ? 1:0;
    scheduleOutPuts( this );
}

QPainterPath Comparator::shape() const
{
    QPainterPath path;
    
    QVector<QPointF> points;
    
    points << QPointF(-16,-16 )
           << QPointF(-16, 16 )
           << QPointF( 16,  1 )
           << QPointF( 16, -1 );
        
    path.addPolygon( QPolygonF(points) );
    path.closeSubpath();
    return path;
}

void Comparator::paint( QPainter* p, const QStyleOptionGraphicsItem* o, QWidget* w )
{
    Component::paint( p, o, w );

    QPen pen = p->pen();
    pen.setWidth(2);
    p->setPen(pen);

 static const QPointF points[4] = {
        QPointF(-16,-16 ),
        QPointF(-16, 16 ),
        QPointF( 16,  1 ),
        QPointF( 16, -1 )            };

    p->drawPolygon(points, 4);

    Component::paintSelected( p );
}
