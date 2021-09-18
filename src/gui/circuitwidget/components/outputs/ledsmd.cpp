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

#include "ledsmd.h"
#include "e-pin.h"

LedSmd::LedSmd( QObject* parent, QString type, QString id, QRectF area, ePin* pin0, ePin* pin1 )
      : LedBase( parent, type, id )
{
    m_area = area;

    if( !pin0 ) pin0 = new ePin( m_elmId+"-ePin"+QString::number(0), 0 );
    if( !pin1 ) pin1 = new ePin( m_elmId+"-ePin"+QString::number(1), 1 );

    setEpin( 0, pin0 );
    setEpin( 1, pin1 );

    createSerRes();
    setModel( "RGY Default" );

}
LedSmd::~LedSmd(){}

void LedSmd::drawBackground( QPainter *p )
{
    p->drawRoundedRect( m_area, 0, 0);
}

void LedSmd::drawForeground( QPainter *p )
{
    p->drawRoundedRect( m_area, 0, 0 );
}

#include "moc_ledsmd.cpp"
