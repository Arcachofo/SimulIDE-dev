/***************************************************************************
 *   Copyright (C) 2012 by Santiago González                               *
 *                                                                         *
 ***( see copyright.txt file at root folder )*******************************/

#include <QPainter>

#include "led.h"
#include "pin.h"
#include "itemlibrary.h"

Component* Led::construct( QObject* parent, QString type, QString id )
{ return new Led( parent, type, id ); }

LibraryItem* Led::libraryItem()
{
    return new LibraryItem(
        tr( "Led" ),
        "Leds",
        "led.png",
        "Led",
        Led::construct);
}

Led::Led( QObject* parent, QString type, QString id )
   : LedBase( parent, type, id )
{
    m_area = QRect(-8, -10, 20, 20 );

    m_pin.resize( 2 );
    m_pin[0] = new Pin( 180, QPoint(-16, 0 ), m_id+"-lPin", 0, this);
    m_pin[1] = new Pin(   0, QPoint( 16, 0 ), m_id+"-rPin", 1, this);
    m_pin[0]->setLength( 10 );

    setEpin( 0, m_pin[0] );
    setEpin( 1, m_pin[1] );
}
Led::~Led(){}

void Led::drawBackground( QPainter* p )
{
    p->drawEllipse(-6,-8, 16, 16 );
}

void Led::drawForeground( QPainter* p )
{
    static const QPointF points[3] = {
        QPointF( 8, 0 ),
        QPointF(-3,-6 ),
        QPointF(-3, 6 ) };

    p->drawPolygon( points, 3 );
    p->drawLine( 8,-4, 8, 4 );
    p->drawLine(-6, 0, 10, 0 );
}
