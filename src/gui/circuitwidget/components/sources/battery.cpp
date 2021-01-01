/***************************************************************************
 *   Copyright (C) 2020 by santiago González                               *
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

#include "connector.h"
#include "battery.h"
#include "itemlibrary.h"
#include "pin.h"

Component* Battery::construct( QObject* parent, QString type, QString id )
{ return new Battery( parent, type, id ); }

LibraryItem* Battery::libraryItem()
{
        return new LibraryItem(
        tr( "Battery" ),
        tr( "Sources" ),
        "battery.png",
        "Battery",
        Battery::construct );
}

Battery::Battery( QObject* parent, QString type, QString id )
      : Component( parent, type, id )
      , eBattery( id )
{
    m_area = QRect( -10, -10, 20, 20 );

    m_pin.resize( 2 );

    m_pin[0] = new Pin( 180, QPoint(-16, 0 ), id+"-Pin0", 0, this);
    m_pin[1] = new Pin(   0, QPoint( 16, 0 ), id+"-Pin1", 1, this);
    m_ePin[0] = m_pin[0];
    m_ePin[1] = m_pin[1];

    setLabelPos(-18,-22, 0);
    setValLabelPos(-10, 10 , 0 ); // x, y, rot
    setShowVal( true );
    setVolt( 5.0 );
}
Battery::~Battery() {}

void Battery::setVolt( double volt )
{
    m_valLabel->setPlainText( QString::number(volt)+" V" );

    eBattery::setVolt( volt );
}

void Battery::paint( QPainter* p, const QStyleOptionGraphicsItem* option, QWidget* widget )
{
    Component::paint( p, option, widget );

    QPen pen = p->pen();
    pen.setWidth(3);
    p->setPen(pen);

    p->drawLine(-7,-8,-7, 8 );
    p->drawLine(-2,-3,-2, 3 );
    p->drawLine( 3,-8, 3, 8 );
    p->drawLine( 8,-3, 8, 3 );
}

#include "moc_battery.cpp"
