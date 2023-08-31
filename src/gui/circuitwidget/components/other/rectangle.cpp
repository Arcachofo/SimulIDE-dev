/***************************************************************************
 *   Copyright (C) 2012 by Santiago González                               *
 *                                                                         *
 ***( see copyright.txt file at root folder )*******************************/

#include <QPainter>

#include "rectangle.h"

#define tr(str) simulideTr("Rectangle",str)

Component* Rectangle::construct( QString type, QString id )
{ return new Rectangle( type, id ); }

LibraryItem* Rectangle::libraryItem()
{
    return new LibraryItem(
        tr("Rectangle"),
        "Graphical",
        "rectangle.png",
        "Rectangle",
        Rectangle::construct);
}

Rectangle::Rectangle( QString type, QString id )
         : Shape( type, id )
{
}
Rectangle::~Rectangle(){}

void Rectangle::paint( QPainter* p, const QStyleOptionGraphicsItem* option, QWidget* widget )
{
    Component::paint( p, option, widget );

    QPen pen(Qt::black, m_border, Qt::SolidLine, Qt::RoundCap, Qt::RoundJoin);
    p->setPen( pen );
    
    double opacity = p->opacity();
    p->setOpacity( opacity*m_opac );
    p->fillRect( m_area, m_color );
    p->setOpacity( opacity );

    if( m_border > 0 )
    {
        p->setBrush( Qt::transparent );
        p->drawRect( m_area );
    }
}
