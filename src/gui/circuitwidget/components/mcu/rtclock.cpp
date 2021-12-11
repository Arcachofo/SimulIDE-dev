/***************************************************************************
 *   Copyright (C) 2021 by santiago González                               *
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

#include "rtclock.h"
#include "iopin.h"
#include "simulator.h"

RtClock::RtClock( QString id )
       : eElement ( id )
{
}
RtClock::~RtClock(){}

void RtClock::initialize()
{
    m_enabled = false;
    m_disOut  = false;

    m_halfCycles = 32768*2;
    m_period = 1e12/m_halfCycles;  // Period in ps
    m_toggle = 1;
    m_tCount = 1;
    m_sCount = m_halfCycles;

    m_date = QDate::currentDate();
    m_time = QTime::currentTime();
    //m_time.setHMS( 0, 0, 0 );
    //m_date.setDate( 0, 0, 0 );

    Simulator::self()->addEvent( m_period, this );
}

void RtClock::runEvent()
{
    if( --m_tCount == 0 ) // Toggle Pin if enabled
    {
        m_tCount = m_toggle;
        if( m_enabled ) m_outpin->setOutState( !m_outpin->getOutState() );
    }
    if( --m_sCount == 0 ) // Increment 1 second
    {
        m_sCount = m_halfCycles;
        m_time = m_time.addSecs( 1 );
        if( m_time == QTime( 0, 0, 0 ) ) m_date = m_date.addDays( 1 );
    }
    Simulator::self()->addEvent( m_period, this );
}

void RtClock::setFreq( uint64_t freq ) { m_toggle = m_halfCycles/freq; }

void RtClock::enable( bool en )
{
    if( m_enabled == en ) return;
    m_enabled = en;
    if( !en ) m_outpin->setOutState( m_disOut );
}

void RtClock::setDisOut( bool d )
{
    if( m_disOut == d ) return;
    m_disOut = d;
    if( !m_enabled ) m_outpin->setOutState( m_disOut );
}