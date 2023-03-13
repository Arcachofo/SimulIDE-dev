/***************************************************************************
 *   Copyright (C) 2012 by Santiago González                               *
 *                                                                         *
 ***( see copyright.txt file at root folder )*******************************/

#include <QPainter>

#include "capacitor.h"
#include "itemlibrary.h"

Component* Capacitor::construct( QObject* parent, QString type, QString id )
{ return new Capacitor( parent, type, id ); }

LibraryItem* Capacitor::libraryItem()
{
    return new LibraryItem(
        tr( "Capacitor" ),
        "Reactive",
        "capacitor.png",
        "Capacitor",
        Capacitor::construct);
}

Capacitor::Capacitor( QObject* parent, QString type, QString id )
         : CapacitorBase( parent, type, id )
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
}
