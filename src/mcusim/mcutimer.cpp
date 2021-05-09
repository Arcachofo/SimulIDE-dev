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

#include "mcutimer.h"
#include "e_mcu.h"
#include "mcuocunit.h"
#include "simulator.h"


McuTimer::McuTimer( eMcu* mcu, QString name )
        : McuModule( mcu, name )
{
    m_number = name.right(1).toInt();

    m_countL = NULL;
    m_countH = NULL;
    initialize();
}

McuTimer::~McuTimer()
{
    for( McuOcUnit* ocUnit : m_ocUnit ) delete ocUnit;
}

void McuTimer::initialize()
{
    m_running = false;
    m_bidirec = false;
    m_reverse = false;

    m_countVal   = 0;
    m_countStart = 0;
    m_ovfMatch   = 0;
    m_ovfPeriod  = 0;
    m_ovfCycle   = 0;

    m_prescaler = 1;

    m_clkSrc  = clkMCU;
    /// m_clkEdge = Clock_Rising;
}

void McuTimer::runEvent()            // Overflow
{
    if( !m_running ) return;

    for( McuOcUnit* ocUnit : m_ocUnit ) ocUnit->tov();

    m_countVal = m_countStart;                // Reset count value
    if( m_bidirec ) m_reverse = !m_reverse;

    if( !m_reverse ) on_tov.emitValue( 1 );

    sheduleEvents();
}

void McuTimer::sheduleEvents()
{
    if( m_running )
    {
        uint64_t circTime = Simulator::self()->circTime();
        m_scale = m_prescaler*m_mcu->simCycPI();

        uint32_t ovfPeriod = m_ovfPeriod;
        if( m_countVal > m_ovfPeriod ) ovfPeriod += m_maxCount;

        uint64_t cycles = (ovfPeriod-m_countVal)*m_scale; // cycles in ps
        m_ovfCycle = circTime + cycles;// In simulation time (ps)

        Simulator::self()->addEvent( cycles, this );
        for( McuOcUnit* ocUnit : m_ocUnit ) ocUnit->sheduleEvents( m_ovfMatch, m_countVal );
    }
    else
    {
        Simulator::self()->cancelEvents( this );
        for( McuOcUnit* ocUnit : m_ocUnit ) Simulator::self()->cancelEvents( ocUnit );
    }
}

void McuTimer::enable( uint8_t en )
{
    updtCount();    // If disabling, write counter values to Ram

    m_running = en;
    updtCycles();  // This will shedule or cancel events
}

void McuTimer::countWriteL( uint8_t val ) // Someone wrote to counter low byte
{
    updtCount();
    m_countL[0] = val;
    updtCycles();                             // update & Reshedule
}

void McuTimer::countWriteH( uint8_t val ) // Someone wrote to counter high byte
{
    updtCount();
    m_countH[0] = val;
    updtCycles();                             // update & Reshedule
}

void McuTimer::updtCount( uint8_t )          // Write counter values to Ram
{
    if( m_running ) // If no running, values were already written at timer stop.
    {
        uint64_t timTime = m_ovfCycle-Simulator::self()->circTime(); // Next overflow time - current time
        uint16_t countVal = timTime/m_mcu->simCycPI()/m_prescaler;

        if( m_countL ) m_countL[0] = countVal & 0xFF;
        if( m_countH ) m_countH[0] = (countVal>>8) & 0xFF;
    }
}

void McuTimer::updtCycles() // Recalculate ovf, comps, etc
{
    if( m_countH ) m_countVal  = m_countH[0] << 8;
    m_countVal |= m_countL[0];
    m_countStart = 0;

    Simulator::self()->cancelEvents( this );
    sheduleEvents();
}

// ----------------------------------------

McuTimers::McuTimers( eMcu* mcu  )
{
    m_mcu = mcu;
}

McuTimers::~McuTimers(){}

void McuTimers::remove()
{
    for( McuTimer* timer : m_timerList ) delete timer;
    m_timerList.clear();
}
