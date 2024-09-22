/***************************************************************************
 *   Copyright (C) 2012 by Santiago González                               *
 *                                                                         *
 ***( see copyright.txt file at root folder )*******************************/

#include <QPainter>

#include "line.h"
#include "itemlibrary.h"

#define tr(str) simulideTr("Line",str)

Component* Line::construct( QString type, QString id )
{ return new Line( type, id ); }

LibraryItem* Line::libraryItem()
{
    return new LibraryItem(
        tr("Line"),
        "Graphical",
        "line.png",
        "Line",
        Line::construct);
}

Line::Line( QString type, QString id )
    : Shape( type, id )
{
}
Line::~Line(){}

QPainterPath Line::shape() const
{
    QPainterPath path;
    QPolygon polygon;
    polygon << QPoint(-m_hSize/2+1, m_vSize/2   );
    polygon << QPoint( m_hSize/2,  -m_vSize/2+1 );
    polygon << QPoint( m_hSize/2-1,-m_vSize/2   );
    polygon << QPoint(-m_hSize/2,   m_vSize/2-1 );
    path.addPolygon(polygon);
    return path;
}

void Line::paint( QPainter* p, const QStyleOptionGraphicsItem* o, QWidget* w )
{
    Component::paint( p, o, w );

    QPen pen( m_color, m_border, Qt::SolidLine, Qt::RoundCap, Qt::RoundJoin);
    p->setPen( pen );

    p->drawLine( -m_hSize/2, m_vSize/2, m_hSize/2, -m_vSize/2 );

    Component::paintSelected( p );
}
