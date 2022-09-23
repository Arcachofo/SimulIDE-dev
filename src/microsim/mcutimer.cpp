/***************************************************************************
 *   Copyright (C) 2020 by Santiago González                               *
 *                                                                         *
 ***( see copyright.txt file at root folder )*******************************/

#include "mcutimer.h"
#include "e_mcu.h"
#include "mcupin.h"
#include "mcuocunit.h"
#include "mcuicunit.h"
#include "mcuinterrupts.h"
#include "simulator.h"

McuTimer::McuTimer( eMcu* mcu, QString name )
        : McuPrescaled( mcu, name )
        , eElement( mcu->getId()+"-"+name )
{
    m_number = name.right(1).toInt();

    m_clockPin = NULL;
    m_countL = NULL;
    m_countH = NULL;
    m_ICunit = NULL;
    initialize();
}

McuTimer::~McuTimer()
{
    for( McuOcUnit* ocUnit : m_ocUnit ) delete ocUnit;
    if( m_ICunit ) delete m_ICunit;
}

void McuTimer::initialize()
{
    m_running = false;
    m_bidirec = false;
    m_reverse = false;
    m_extClock = false;

    m_countVal   = 0;
    m_countStart = 0;
    m_ovfMatch   = 0;
    m_ovfPeriod  = 0;
    m_ovfCycle   = 0;
    m_mode       = 0;

    m_prescaler = 1;
    m_prIndex = 0;

    //m_clkSrc  = clkMCU;
    m_clkEdge = 1;

    m_scale = m_prescaler*m_mcu->psCycle();
}

void McuTimer::voltChanged()  // External Clock Pin changed voltage
{
    bool state = m_clockPin->getInpState();
    if( m_clkState == state ) return;
    if( m_clkEdge == 1 )              // Rising
    {
        if( !state && m_clkState ) clockStep();
    }
    else if( state && !m_clkState ) clockStep();
    m_clkState = state;
}

void McuTimer::clockStep()  // Timer driven by external clock
{
    m_countVal++;
    for( McuOcUnit* ocUnit : m_ocUnit ) ocUnit->clockStep( m_countVal ); ///
    if( m_countVal == m_ovfMatch ) runEvent();
}

void McuTimer::runEvent()            // Overflow
{
    //if( m_name == "TIMER1") /// DELETEME
    //    m_running = m_running;
    if( !m_running ) return;

    for( McuOcUnit* ocUnit : m_ocUnit ) ocUnit->tov();

    m_countVal = m_countStart;             // Reset count value
    if( m_bidirec ) m_reverse = !m_reverse;
    if( !m_reverse && m_interrupt ) m_interrupt->raise();

    sheduleEvents();
}

void McuTimer::resetTimer()
{
    m_countVal = m_countStart;             // Reset count value
    sheduleEvents();
}

void McuTimer::sheduleEvents()
{
    if( !m_running ) return;
    if( m_extClock )
    {
        Simulator::self()->cancelEvents( this );
        for( McuOcUnit* ocUnit : m_ocUnit ) Simulator::self()->cancelEvents( ocUnit );
    }else{
        uint64_t circTime = Simulator::self()->circTime();
        uint64_t ovfPeriod = m_ovfPeriod;
        if( m_countVal > m_ovfPeriod ) ovfPeriod += m_maxCount;

        uint64_t cycles = (ovfPeriod-m_countVal)*m_scale; // cycles in ps
        uint64_t ovfCycle = circTime + cycles;// In simulation time (ps)

        //if( m_name == "TIMER0") /// DELETEME
        //    m_running = m_running;

        if( m_ovfCycle != ovfCycle )
        {
            m_ovfCycle = ovfCycle;
            Simulator::self()->cancelEvents( this );
            Simulator::self()->addEvent( cycles, this );
        }
    }
    for( McuOcUnit* ocUnit : m_ocUnit ) ocUnit->sheduleEvents( m_ovfMatch, m_countVal );
}

void McuTimer::enable( uint8_t en )
{
    bool e = en > 0;
    if( m_running == e )
        return;
    updtCount();    // If disabling, write counter values to Ram
    m_running = e;
    updtCycles();  // This will shedule or cancel events
}

void McuTimer::countWriteL( uint8_t val ) // Someone wrote to counter low byte
{
    updtCount();
    *m_countL = val;
    updtCycles();       // update & Reshedule
}

void McuTimer::countWriteH( uint8_t val ) // Someone wrote to counter high byte
{
    updtCount();
    *m_countH = val;
    updtCycles();      // update & Reshedule
}

void McuTimer::updtCount( uint8_t )       // Write counter values to Ram
{
    if( !m_running ) return;// If no running, values were already written at timer stop.

    if( !m_extClock )
    {
        uint64_t time2Ovf = m_ovfCycle-Simulator::self()->circTime(); // Next overflow time - current time
        uint64_t cycles2Ovf = time2Ovf/m_scale;
        if( m_ovfMatch > cycles2Ovf ) m_countVal = m_ovfMatch-cycles2Ovf;
    }
    if( m_countL ) *m_countL = m_countVal & 0xFF;
    if( m_countH ) *m_countH = (m_countVal>>8) & 0xFF;
}

void McuTimer::updtCycles() // Recalculate ovf, comps, etc
{
    m_countVal = *m_countL;
    if( m_countH ) m_countVal |= *m_countH << 8;
    /// m_countStart = 0;
    sheduleEvents();
}

uint32_t McuTimer::getCount()
{
    updtCount();
    return m_countVal;
}

void McuTimer::enableExtClock( bool en )
{
    if( m_extClock == en ) return;
    updtCount();
    m_extClock = en; //m_clkSrc = en? clkEXT : clkMCU;
    updtCycles();      // update & Reshedule

    if( m_clockPin )
    {
        m_clockPin->changeCallBack( this, en );
        m_clkState = m_clockPin->getInpState();
    }
}

