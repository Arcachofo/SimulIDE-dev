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

#include <QDomDocument>

#include "mcutimer.h"
#include "e_mcu.h"
#include "simulator.h"

QHash<QString, McuTimer*> McuTimer::m_timers;

McuTimer::McuTimer( eMcu* mcu )
        : eElement( "McuTimer" )
{
    m_mcu = mcu;

    m_countL = 0l;
    m_countH = 0l;
}

McuTimer::~McuTimer(){}

void McuTimer::remove() // Static
{
    for( McuTimer* timer : m_timers ) delete timer;
    m_timers.clear();
}

void McuTimer::initialize()
{
    m_running = false;
    m_compare = false;
    m_bidirec = false;
    m_reverse = false;

    m_countVal  = 0;
    m_ovfMatch  = 0;
    m_ovfPeriod = 0;
    m_ovfCycle  = 0;

    m_comMatch  = 0;
    m_comPeriod = 0;
    m_comCycle  = 0;
}

void McuTimer::runEvent()            // Overflow or Compare match
{
    if( !m_running ) return;

    uint64_t circTime = Simulator::self()->circTime();

    if( circTime == m_ovfCycle )                     // Overflow
    {
        on_tov.emitValue( 1 );

        m_countVal = 0;
        if( m_bidirec ) m_reverse = !m_reverse;

        sheduleEvents();
    }
    if( m_compare && (circTime == m_comCycle) )  // Compare match
    {
        on_comp.emitValue( 1 );
    }
}

void McuTimer::sheduleEvents()
{
    if( m_running )
    {
        uint64_t circTime = Simulator::self()->circTime();

        m_ovfCycle = circTime + m_ovfPeriod - m_countVal;

        Simulator::self()->addEvent( m_ovfPeriod, this );

        if( m_compare )
        {
            if( m_reverse )
                 m_comMatch = m_ovfPeriod - m_comPeriod;
            else m_comMatch = m_comPeriod;

            if( m_comMatch > m_countVal ) // be sure next comp match is still ahead
            {
                m_comCycle = circTime + m_comMatch - m_countVal;
                Simulator::self()->addEvent( m_comMatch- m_countVal, this );
            }
        }
    }
    else Simulator::self()->cancelEvents( this );
}

void McuTimer::enable( uint8_t en )
{
    updtCount();    // If disabling, write counter values to Ram
    m_countValL = TIM_COUNT_L;
    m_countValH = TIM_COUNT_H;

    m_running = en;
    updtCycles();  // This will shedule or cancel events
}

void McuTimer::countWriteL( uint8_t val ) // Someone wrote to counter low byte
{
    updtCount();
    m_countValL = val;
    m_countValH = TIM_COUNT_H;

    Simulator::self()->cancelEvents( this );
    updtCycles();                             // update & Reshedule
}

void McuTimer::countWriteH( uint8_t val ) // Someone wrote to counter high byte
{
    updtCount();
    m_countValL = TIM_COUNT_L;
    m_countValH = val;

    Simulator::self()->cancelEvents( this );
    updtCycles();                             // update & Reshedule
}

void McuTimer::updtCount() // Write counter values to Ram
{
    if( m_running )
    {
        uint64_t timTime = m_ovfCycle-Simulator::self()->circTime(); // Next overflow time - current time
        uint16_t countVal = timTime/m_prescaler;

        if( m_countL ) m_countL[0] = countVal & 0xFF;
        if( m_countH ) m_countH[0] = (countVal>>8) & 0xFF;
    }
}

void McuTimer::countReadL( uint8_t val ) // Someone is reading counter low byte
{
    updtCount(); // Write counter values to Ram
}

void McuTimer::countReadH( uint8_t val ) // Someone is reading counter high byte
{
    updtCount(); // Write counter values to Ram
}


/*void McuTimer::configure( uint16_t p, uint16_t cm, bool bd )
{
    m_period = p;
    m_comMatch = cm;
    m_bidir = bd;
}*/
