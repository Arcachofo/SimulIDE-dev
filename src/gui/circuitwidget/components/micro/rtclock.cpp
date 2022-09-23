/***************************************************************************
 *   Copyright (C) 2021 by Santiago González                               *
 *                                                                         *
 ***( see copyright.txt file at root folder )*******************************/

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

    m_freqBase = 32768;
    m_halfPeriod = 1e12/m_freqBase/2;  // Period in ps
    m_toggle = 1;
    m_tCount = 1;
    m_sCount = m_freqBase*2;

    m_date = QDate::currentDate();
    m_time = QTime::currentTime();
    //m_time.setHMS( 0, 0, 0 );
    //m_date.setDate( 0, 0, 0 );

    Simulator::self()->addEvent( m_halfPeriod, this );
}

void RtClock::runEvent()
{
    if( --m_tCount == 0 ) // Toggle Pin if enabled
    {
        m_tCount = m_toggle;
        if( m_enabled ) m_outpin->sheduleState( !m_outpin->getOutState(), 0 );
    }
    if( --m_sCount == 0 ) // Increment 1 second
    {
        m_sCount = m_freqBase*2;
        m_time = m_time.addSecs( 1 );
        if( m_time == QTime( 0, 0, 0 ) ) m_date = m_date.addDays( 1 );
    }
    Simulator::self()->addEvent( m_halfPeriod, this );
}

void RtClock::setFreq( uint64_t freq )
{
    m_toggle = m_freqBase/freq;
    m_tCount = m_toggle;
}

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
