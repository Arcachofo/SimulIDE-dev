/***************************************************************************
 *   Copyright (C) 2018 by Santiago González                               *
 *                                                                         *
 ***( see copyright.txt file at root folder )*******************************/

#include <QPainter>

#include "elcapacitor.h"
#include "itemlibrary.h"
#include "simulator.h"
#include "e-pin.h"

Component* elCapacitor::construct( QObject* parent, QString type, QString id )
{ return new elCapacitor( parent, type, id ); }

LibraryItem* elCapacitor::libraryItem()
{
    return new LibraryItem(
        tr( "Electrolytic Capacitor" ),
        "Reactive",
        "elcapacitor.png",
        "elCapacitor",
        elCapacitor::construct);
}

elCapacitor::elCapacitor( QObject* parent, QString type, QString id )
           : CapacitorBase( parent, type, id )
{
    Simulator::self()->addToUpdateList( this );
}
elCapacitor::~elCapacitor(){}

void elCapacitor::updateStep()
{
    double volt = m_ePin[0]->getVoltage() - m_ePin[1]->getVoltage();
    bool crashed = ( volt < -1e-6 );
    if( crashed || (crashed != m_crashed) )
    {
        m_crashed = crashed;
        update();
}   }

void elCapacitor::paint( QPainter* p, const QStyleOptionGraphicsItem* option, QWidget* widget )
{
    Component::paint( p, option, widget );

    QPen pen = p->pen();
    pen.setWidth(3);
    p->setPen(pen);
    
    p->drawLine( 3,-7, 3, 7 );
    p->drawLine(-3,-7, 3,-7 );
    p->drawLine(-3, 7, 3, 7 );
    p->drawLine(-3,-3,-3, 3 );
}
