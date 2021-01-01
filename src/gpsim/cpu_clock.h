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

#if !defined(__CLOCK_Phase_H__)
#define __CLOCK_Phase_H__

#include <QMultiMap>

class TriggerObject;
class ClockPhase;
class PhaseExec1;
class PhaseExec2;
class PhaseInter;
class PhaseIdle;

class CpuClock
{
public:
    CpuClock();
    virtual ~CpuClock();

    void halt();
    bool isHalted() { return m_halted; }
    void incrementCycle();
    uint64_t currentCycle() { return m_cycle; }
    uint64_t getFutureCycle( double seconds )// Return the cycle for some time in seconds in the future
    { return m_cycle+(uint64_t)(m_cps*seconds); }

    bool setBreakRel( uint64_t cycles, TriggerObject* f );
    bool setBreakAbs(uint64_t future_cycle, TriggerObject* f );
    bool reassign_break( uint64_t old_cycle,uint64_t future_cycle, TriggerObject *f=0);
    void clear_current_break( TriggerObject* f=0 );
    void clear_break( uint64_t cb );
    void clear_break( TriggerObject* f );
    //void dump_breakpoints();

    bool have_pm_write()  { return m_havePmWrite; }
    void clear_pm_write() { m_havePmWrite = false; }
    void set_pm_write()   { m_havePmWrite = true; }

    // Processor Clock control
    void set_frequency( double f);
    void set_frequency_rc( double f);
    void set_RCfreq_active( bool);
    virtual double get_frequency();
    void set_ClockPerInstr( uint cpi ) { clocks_per_inst = cpi; }
    uint get_ClockPerInstr( void )     { return clocks_per_inst; }
    void update_cps(void);

    void set_instruction_cps( uint64_t cps );
    double instruction_cps() { return m_cps; }
    double seconds_per_cycle() { return m_spc; }

    uint clocks_per_inst; /// Oscillator cycles for 1 instruction

    ClockPhase* m_phaseCurr;
    PhaseExec1* m_phase1;
    PhaseExec2* m_phase2;     // misnomer - should be 2-cycle
    PhaseInter* m_phaseInter;
    PhaseIdle*  m_phaseIdle;

protected:
    double m_freq;
    double m_RCfreq;
    bool   m_useRCfreq;

    uint64_t m_cycle;
    uint64_t m_nextBreak;
    double   m_cps;           // Number of Instruction Cycles in one second
    double   m_spc;

    bool m_BrkOnBadRegRead;
    bool m_BrkOnBadRegWrite;
    bool m_havePmWrite;

    bool m_halted;

    QMultiMap<uint64_t, TriggerObject*> m_activeCB;
};




/*
  Clock Phase

  The Clock Phase base class takes an external clock source as its
  input and uses this to control a module's simulation state. For
  example, the clock input on a microcontroller drives all of the
  digital state machines. On every edge of the clock, there is digital
  logic that can potentially change states. The Clock Phase base class
  can be thought of the "logic" that responds to the clock input and
  redirects control to the state machines inside of a processor.
*/

class Processor;

class ClockPhase
{
public:
    ClockPhase();
    virtual ~ClockPhase();
    virtual ClockPhase *advance()=0;
    void setNextPhase( ClockPhase *pNextPhase) { m_pNextPhase = pNextPhase; }
    ClockPhase* getNextPhase() { return m_pNextPhase; }
protected:
    ClockPhase* m_pNextPhase;
};


//  The Processor Phase base class is a Clock Phase class that contains a
//  pointer to a Processor object. It's the base class from which all of
//  the processor's various Phase objects are derived.

class ProcessorPhase : public ClockPhase
{
public:
    explicit ProcessorPhase(Processor *pcpu);
    virtual ~ProcessorPhase();
protected:
    Processor* m_pcpu;
};


//  The Execute 1 Cycle class is a Processor Phase class designed to
//  execute a single instruction.

class PhaseExec1 : public ProcessorPhase
{
public:
    explicit PhaseExec1(Processor *pcpu);
    virtual ~PhaseExec1();
    virtual ClockPhase *advance();
};

class PhaseExec2 : public ProcessorPhase
{
public:
    explicit PhaseExec2(Processor *pcpu);
    virtual ~PhaseExec2();
    virtual ClockPhase *advance();
    ClockPhase* firstHalf( unsigned int uiPC );

protected:
    unsigned int m_uiPC;
};

class PhaseInter : public ProcessorPhase
{
public:
    explicit PhaseInter(Processor *pcpu);
    ~PhaseInter();
    virtual ClockPhase *advance();
    void firstHalf();

protected:
    ClockPhase *m_pCurrentPhase;
    ClockPhase *m_pNextNextPhase;
};

// PhaseIdle - when a processor is idle, the current
// clock source can be handled by this class.

class PhaseIdle : public ProcessorPhase
{
public:
    explicit PhaseIdle(Processor *pcpu);
    virtual ~PhaseIdle();
    virtual ClockPhase *advance();

protected:
};

#endif  //if !defined(__CLOCK_Phase_H__)
