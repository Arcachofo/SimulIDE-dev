/***************************************************************************
 *   Copyright (C) 2025 by Santiago González                               *
 *                                                                         *
 ***( see copyright.txt file at root folder )*******************************/

#include "analogclock.h"
#include "simulator.h"
#include "e-reactive.h"
#include "circuitwidget.h"

AnalogClock* AnalogClock::m_pSelf = nullptr;

AnalogClock::AnalogClock()
           : eElement("AnalogClock")
{
    m_pSelf = this;
    m_clkElement = nullptr;
    m_divider = 1;
    m_period = m_step = 1e6;
}
AnalogClock::~AnalogClock(){}

void AnalogClock::stamp()
{
    setDivider( 1 );
    if( m_clkElement ) Simulator::self()->addEvent( m_step, this );
}

void AnalogClock::updateStep()
{
    if( !m_changed ) return;
    m_changed = false;
    CircuitWidget::self()->updtAppDialog();
}

void AnalogClock::runEvent()
{
    eElement* clkElement = m_clkElement;
    while( clkElement )
    {
        clkElement->runEvent();
        clkElement = clkElement->nextEvent;
    }
    Simulator::self()->addEvent( m_step, this );
}

void AnalogClock::addClkElement( eElement* e )
{
    e->nextEvent = m_clkElement;
    m_clkElement = e;
}

void AnalogClock::remClkElement( eElement* e )
{
    eElement* clkElement = m_clkElement;
    eElement* lastElement = nullptr;
    eElement* nextElement = nullptr;

    while( clkElement )
    {
        if( clkElement == e )
        {
            nextElement = clkElement->nextEvent;
            if( lastElement ) lastElement->nextEvent = nextElement;
            else              m_clkElement = nextElement;
            clkElement->nextEvent = nullptr;
            return;
        }
        lastElement = clkElement;
        clkElement = clkElement->nextEvent;
    }
}

void AnalogClock::setPeriod( uint64_t p )
{
    m_period = p;
    m_step = m_period/m_divider;
}

void AnalogClock::setDivider( uint64_t d )
{
    m_divider = d;
    m_step = m_period/m_divider;
    m_changed = true;
}
