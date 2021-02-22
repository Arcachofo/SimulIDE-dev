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

#include "ground.h"
#include "e-source.h"
#include "itemlibrary.h"
#include "pin.h"

Component* Ground::construct( QObject* parent, QString type, QString id )
{ return new Ground( parent, type, id ); }

LibraryItem* Ground::libraryItem()
{
        return new LibraryItem(
        tr( "Ground (0 V)" ),
        tr( "Sources" ),
        "ground.png",
        "Ground",
        Ground::construct );
}

Ground::Ground( QObject* parent, QString type, QString id )
      : Component( parent, type, id )
{
    m_area = QRect( -10, -10, 20, 20 );

    m_pin.resize(1);
    QString nodid = id;
    nodid.append(QString("-Gnd"));
    QPoint nodpos = QPoint( 0, -16 );
    m_pin[0] = new Pin( 90, nodpos, nodid, 0, this );

    nodid.append( QString("-eSource") );
    m_out = new eSource( nodid, m_pin[0], source );
    
    setLabelPos(-16, 8, 0);
}
Ground::~Ground() { delete m_out; }

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

#include "moc_ground.cpp"
