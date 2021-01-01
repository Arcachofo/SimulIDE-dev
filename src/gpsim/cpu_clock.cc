/*
   Copyright (C) 2006 T. Scott Dattalo

This file is part of the libgpsim library of gpsim

This library is free software; you can redistribute it and/or
modify it under the terms of the GNU Lesser General Public
License as published by the Free Software Foundation; either
version 2.1 of the License, or (at your option) any later version.

This library is distributed in the hope that it will be useful,
but WITHOUT ANY WARRANTY; without even the implied warranty of
MERCHANTABILITY or FITNESS FOR A PARTICULAR PURPOSE.  See the GNU
Lesser General Public License for more details.

You should have received a copy of the GNU Lesser General Public
License along with this library; if not, see 
<http://www.gnu.org/licenses/lgpl-2.1.html>.
*/


#include "cpu_clock.h"
#include "pic-processor.h"
#include "processor.h"

// Cpu Clock is indeed Instruction cycle

CpuClock::CpuClock()
{
    m_halted = false;
    m_freq = 20e6;
    m_RCfreq = 0;
    m_useRCfreq = false;
    set_ClockPerInstr( 4 );
    update_cps();

    m_BrkOnBadRegRead = true;
    m_BrkOnBadRegWrite = true;
    m_havePmWrite = false;
}

CpuClock::~CpuClock()
{
}

void CpuClock::halt()
{
    cout << " ERROR!!! PIC HALTED " <<endl;
    m_halted = true;
}

void CpuClock::incrementCycle() // Increment the current cycle, check for breakpoints
{
    //if( m_halted )
    if( m_cycle == m_nextBreak )
    {
        QList<TriggerObject*> values = m_activeCB.values( m_cycle ); // Get Triggers for this cycle
        for( TriggerObject* f : values )
        {
            if( f ) f->callback();
            else qDebug() << "Cycle_Counter::increment  No TRiggerObject" << '\n';

            m_activeCB.remove( m_cycle, f );
        }
        if( !m_activeCB.isEmpty() ) m_nextBreak = m_activeCB.firstKey();
        else                        m_nextBreak = 0xFFFFFFFF;
    }
    m_cycle++;
}

bool CpuClock::setBreakRel( uint64_t cycles, TriggerObject* f ) // Set Break at "cycles" after vurrent Cycle
{
    return setBreakAbs( m_cycle+cycles, f );
}

bool CpuClock::setBreakAbs( uint64_t future_cycle, TriggerObject* f )
{
    if( future_cycle <= m_cycle )
    {
        if( future_cycle == 0 ) return 0;
        qDebug() << "CpuClock::setBreakAbs";
        qDebug() << "Cycle break point "<< future_cycle << "has already gone";
        qDebug() << "current cycle is  " << m_cycle << '\n';
        return 0;
    }
    if( !f )
    {
        qDebug() << "CpuClock::setBreakAbs  No TriggerObject" << '\n';
        return 0;
    }
    if( m_activeCB.contains( future_cycle, f ))
    {
        qDebug() << "CpuClock::setBreakAbs  Break already exist" << future_cycle<<'\n';
        return 0;
    }
    m_activeCB.insert( future_cycle, f );

    m_nextBreak = m_activeCB.firstKey();

    return 1;
}

void CpuClock::clear_break( TriggerObject* f ) // remove break for TriggerObject
{
    if( !f ) return;

    QList<uint64_t> breaks = m_activeCB.keys( f );
    for( uint64_t cb : breaks ) m_activeCB.remove( cb, f );


    if( !m_activeCB.isEmpty() ) m_nextBreak = m_activeCB.firstKey();
    else                        m_nextBreak = 0xFFFFFFFF;
}

void CpuClock::clear_break( uint64_t cb )
{
    QMultiMap<uint64_t, TriggerObject*>::iterator i = m_activeCB.find( cb );
    if( i == m_activeCB.end() )
    {
        //qDebug() << "Cycle_Counter::clear_break  No Breakpoints for Cycle" << cb<<'\n';
        return;
    }
    while( i!=m_activeCB.end() && i.key()==cb ) // Remove breakpoint for cb cycle
    {
        TriggerObject* f = i.value();
        ++i;
        m_activeCB.remove( cb, f );
    }
    if( !m_activeCB.isEmpty() ) m_nextBreak = m_activeCB.firstKey();
    else                        m_nextBreak = 0xFFFFFFFF;
}

bool CpuClock::reassign_break( uint64_t old_cycle, uint64_t new_cycle, TriggerObject* f )
{
    m_activeCB.remove( old_cycle, f );

    m_activeCB.insert( new_cycle, f );

    if( !m_activeCB.isEmpty() ) m_nextBreak = m_activeCB.firstKey();
    else                        m_nextBreak = 0xFFFFFFFF;

    return 1;
}

void CpuClock::clear_current_break( TriggerObject* f )
{
    m_activeCB.remove( m_cycle, f );

    if( !m_activeCB.isEmpty() ) m_nextBreak = m_activeCB.firstKey();
    else                        m_nextBreak = 0xFFFFFFFF;
}


void CpuClock::set_frequency(double f)
{
    pic_processor *pCpu = dynamic_cast<pic_processor*>(this);

    m_freq = f;
    update_cps();
    if( pCpu ) pCpu->wdt.update();

}
void CpuClock::set_frequency_rc( double f )
{
    m_RCfreq = f;
    update_cps();
}

void CpuClock::set_RCfreq_active( bool state )
{
    m_useRCfreq = state;
    update_cps();
}

double CpuClock::get_frequency()
{
    if( m_useRCfreq ) return m_RCfreq;
    else              return m_freq;
}

void CpuClock::update_cps( void )
{
    set_instruction_cps(( uint64_t)( get_frequency()/clocks_per_inst ) );
}

void CpuClock::set_instruction_cps( uint64_t cps )
{
    if( !cps ) return;

    m_cps =( double)cps;
    m_spc = 1.0/m_cps;
}




//========================================================================
ClockPhase::ClockPhase()
    : m_pNextPhase(this)
{ }

ClockPhase::~ClockPhase()
{ }
//========================================================================

ProcessorPhase::ProcessorPhase( Processor* pcpu )
    : ClockPhase()
    , m_pcpu( pcpu )
{ }
ProcessorPhase::~ProcessorPhase()
{ }
//========================================================================

PhaseExec1::PhaseExec1 (Processor* pcpu )
    : ProcessorPhase(pcpu)
{ }
PhaseExec1::~PhaseExec1()
{ }

ClockPhase *PhaseExec1::advance() // advances a processor's time one clock cycle.
{
    setNextPhase( this );
    m_pcpu->executePc();

    //if( m_pcpu->bp.global_break & GLOBAL_LOG) m_pcpu->bp.global_break &= ~GLOBAL_LOG;
    //if(!m_pcpu->bp.global_break)              m_pcpu->incrementCycle();
    if( !m_pcpu->have_pm_write() ) m_pcpu->incrementCycle();

    return m_pNextPhase;
}
//========================================================================

PhaseExec2::PhaseExec2(Processor *pcpu)
    : ProcessorPhase(pcpu)
    , m_uiPC(0)
{
}
PhaseExec2::~PhaseExec2()
{
}

ClockPhase* PhaseExec2::firstHalf( uint uiPC )
{
    m_pcpu->pc->value = uiPC;
    m_pcpu->pc->update_pcl();
    m_pcpu->m_phaseCurr->setNextPhase(this);
    return this;
}

ClockPhase* PhaseExec2::advance()
{
    m_pcpu->m_phaseCurr->setNextPhase( m_pcpu->m_phase1 );
    m_pcpu->incrementCycle();
    return m_pNextPhase;
}
//========================================================================

PhaseIdle::PhaseIdle(Processor *pcpu)
    : ProcessorPhase(pcpu)
{
}
PhaseIdle::~PhaseIdle()
{
}

ClockPhase *PhaseIdle::advance()
{
    setNextPhase( this );
    m_pcpu->incrementCycle();
    return m_pNextPhase;
}
//========================================================================

PhaseInter::PhaseInter( Processor* pcpu )
    : ProcessorPhase( pcpu )
    , m_pCurrentPhase(0)
    , m_pNextNextPhase(0)
{
}
PhaseInter::~PhaseInter()
{}

ClockPhase* PhaseInter::advance()
{
    if( m_pNextPhase == m_pcpu->m_phase2 ) m_pNextPhase->advance();

    if( m_pCurrentPhase == m_pcpu->m_phaseIdle ) // Interrupted sleep
    {
        m_pNextPhase = m_pNextNextPhase->advance(); // complete sleeping Phase

        if( m_pNextPhase == m_pcpu->m_phaseIdle )
        {
            m_pNextPhase = m_pcpu->m_phase1;
            do
            {
                m_pNextPhase = m_pcpu->m_phase1->advance();
            }
            while (m_pNextPhase != m_pcpu->m_phase1 );
        }
        m_pcpu->m_phaseCurr = this;

        //if( m_pcpu->bp.global_break ) m_pNextNextPhase = m_pNextPhase;
        //else                          m_pCurrentPhase = NULL;
        if( m_pcpu->have_pm_write() ) m_pNextNextPhase = m_pNextPhase;
        else                          m_pCurrentPhase = NULL;

        m_pcpu->exit_sleep();
        return this;
    }
    m_pcpu->interrupt();

    return m_pNextPhase;
}

void PhaseInter::firstHalf()
{
    m_pCurrentPhase = m_pcpu->m_phaseCurr;

    m_pNextPhase = this;
    m_pNextNextPhase = m_pcpu->m_phaseCurr->getNextPhase();
    m_pcpu->m_phaseCurr->setNextPhase( this );
    m_pcpu->m_phaseCurr = this;
}

