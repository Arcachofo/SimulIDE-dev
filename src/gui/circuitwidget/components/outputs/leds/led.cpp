/***************************************************************************
 *   Copyright (C) 2012 by santiago González                               *
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

#include "led.h"
#include "pin.h"
#include "itemlibrary.h"

Component* Led::construct( QObject* parent, QString type, QString id )
{ return new Led( parent, type, id ); }

LibraryItem* Led::libraryItem()
{
    return new LibraryItem(
        tr( "Led" ),
        tr( "Leds" ),
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

    //createSerRes();
    //setModel( "RGY Default" );
}
Led::~Led(){}

void Led::drawBackground( QPainter* p )
{
    p->drawEllipse( -6, -8, 16, 16 );
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
