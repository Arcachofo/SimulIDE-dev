/***************************************************************************
 *   Copyright (C) 2012 by Santiago González                               *
 *                                                                         *
 ***( see copyright.txt file at root folder )*******************************/

#include <QPainter>

#include "capacitor.h"
#include "itemlibrary.h"

#define tr(str) simulideTr("Capacitor",str)

Component* Capacitor::construct( QString type, QString id )
{ return new Capacitor( type, id ); }

LibraryItem* Capacitor::libraryItem()
{
    return new LibraryItem(
        tr("Capacitor"),
        "Reactive",
        "capacitor.png",
        "Capacitor",
        Capacitor::construct);
}

Capacitor::Capacitor( QString type, QString id )
         : CapacitorBase( type, id )
{}
Capacitor::~Capacitor(){}

void Capacitor::paint( QPainter* p, const QStyleOptionGraphicsItem* option, QWidget* widget )
{
    Component::paint( p, option, widget );

    QPen pen = p->pen();
    pen.setWidth(3);
    p->setPen(pen);

    p->drawLine(-3,-6,-3, 6 );
    p->drawLine( 3,-6, 3, 6 );

    Component::paintSelected( p );
}
