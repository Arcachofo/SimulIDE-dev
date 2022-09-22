/***************************************************************************
 *   Copyright (C) 2018 by santiago González                               *
 *   santigoro@gmail.com                                                   *
 *                                                                         *
 *   This program is free software; you can redistribute it and/or modify  *
 *   it under the terms of the GNU General Public License as published by  *
 *   the Free Software Foundation; either version 3 of the License, or     *
 *   (at your option) any later version.                                   *
 *                                                                         *
 *   This program is distributed in the hope that it will be useful,       *
 *   but WITHOUT ANY WARRANTY; without even the implied warranty of        *
 *   MERCHANTABILITY or FITNESS FOR A PARTICULAR PURPOSE.  See the         *
 *   GNU General Public License for more details.                          *
 *                                                                         *
 *   You should have received a copy of the GNU General Public License     *
 *   along with this program; if not, see <http://www.gnu.org/licenses/>.  *
 *                                                                         *
 ***************************************************************************/

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
        QObject::tr( "Electrolytic Capacitor" ),
        QObject::tr( "Reactive" ),
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
