/***************************************************************************
 *   Copyright (C) 2012 by Santiago González                               *
 *                                                                         *
 ***( see copyright.txt file at root folder )*******************************/

#include <QPainter>

#include "ground.h"
#include "itemlibrary.h"
#include "iopin.h"

Component* Ground::construct( QObject* parent, QString type, QString id )
{ return new Ground( parent, type, id ); }

LibraryItem* Ground::libraryItem()
{
    return new LibraryItem(
        QObject::tr( "Ground (0 V)" ),
        QObject::tr( "Sources" ),
        "ground.png",
        "Ground",
        Ground::construct );
}

Ground::Ground( QObject* parent, QString type, QString id )
      : Component( parent, type, id )
{
    m_area = QRect(-10,-10, 20, 14 );

    m_pin.resize(1);
    m_pin[0] = new IoPin( 90, QPoint( 0,-16 ), id+"-Gnd", 0, this, source );
    
    setLabelPos(-16, 8, 0);
}
Ground::~Ground() { /*delete m_out;*/ }

void Ground::paint( QPainter *p, const QStyleOptionGraphicsItem *option, QWidget *widget )
{
    Component::paint( p, option, widget );

    QPen pen = p->pen();
    pen.setWidth(3);
    p->setPen(pen);

    p->drawLine( -8,-8, 8,-8 );
    p->drawLine( -5,-3, 5,-3 );
    p->drawLine( -2, 2, 2, 2 );
}
