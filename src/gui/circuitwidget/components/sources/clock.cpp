/***************************************************************************
 *   Copyright (C) 2017 by santiago González                               *
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

#include "clock.h"
#include "iopin.h"
#include "simulator.h"
#include "itemlibrary.h"

#include "doubleprop.h"
#include "boolprop.h"

Component* Clock::construct( QObject* parent, QString type, QString id )
{ return new Clock( parent, type, id ); }

LibraryItem* Clock::libraryItem()
{
    return new LibraryItem(
        tr( "Clock" ),
        tr( "Sources" ),
        "clock.png",
        "Clock",
        Clock::construct );
}

Clock::Clock( QObject* parent, QString type, QString id )
     : ClockBase( parent, type, id )
{
    remPropGroup( tr("Main") );
    addPropGroup( { tr("Main"), {
new DoubProp<Clock>( "Voltage"  , tr("Voltage")  ,"V" , this, &Clock::volt,     &Clock::setVolt ),
new DoubProp<Clock>( "Freq"     , tr("Frequency"),"Hz", this, &Clock::freq,     &Clock::setFreq ),
new BoolProp<Clock>( "Always_On", tr("Always On"),""  , this, &Clock::alwaysOn, &Clock::setAlwaysOn ),
    }} );
}
Clock::~Clock(){}

void Clock::runEvent()
{
    m_state = !m_state;
    m_outpin->sheduleState( m_state );

    m_remainder += m_fstepsPC-(double)m_stepsPC;
    uint64_t remainerInt = m_remainder;
    m_remainder -= remainerInt;

    if( m_isRunning ) Simulator::self()->addEvent( m_stepsPC/2+remainerInt, this );
}

void Clock::paint( QPainter* p, const QStyleOptionGraphicsItem* option, QWidget* widget )
{
    if( m_hidden ) return;

    Component::paint( p, option, widget );

    if (  m_isRunning ) p->setBrush( QColor( 250, 200, 50 ) );
    else                p->setBrush( QColor( 230, 230, 255 ) );

    p->drawRoundedRect( m_area,2 ,2 );

    QPen pen;
    pen.setWidth(1);
    p->setPen( pen );
    
    p->drawLine(-11, 3,-11,-3 );
    p->drawLine(-11,-3,-5, -3 );
    p->drawLine( -5,-3,-5,  3 );
    p->drawLine( -5, 3, 1,  3 );
    p->drawLine(  1, 3, 1, -3 );
    p->drawLine(  1,-3, 4, -3 );
}
