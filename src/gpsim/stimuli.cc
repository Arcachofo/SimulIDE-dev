/*
   Copyright( C) 1998 T. Scott Dattalo
   Copyright( C) 2006,2015 Roy R Rankin

This file is part of the libgpsim library of gpsim

This library is free software; you can redistribute it and/or
modify it under the terms of the GNU Lesser General Public
License as published by the Free Software Foundation; either
version 2.1 of the License, or( at your option) any later version.

This library is distributed in the hope that it will be useful,
but WITHOUT ANY WARRANTY; without even the implied warranty of
MERCHANTABILITY or FITNESS FOR A PARTICULAR PURPOSE.  See the GNU
Lesser General Public License for more details.

You should have received a copy of the GNU Lesser General Public
License along with this library; if not, see 
<http://www.gnu.org/licenses/lgpl-2.1.html>.
*/

#include <QDebug>
#include <stdio.h>
#include <ctype.h>
#include <iostream>
#include <stdlib.h>
#include <string>
#include <list>
#include <vector>

#include <math.h>

#include "pic-processor.h"
#include "stimuli.h"

//#define DEBUG
#if defined(DEBUG)
#define Dprintf(arg) {printf("%s:%d-%s() ",__FILE__,__LINE__,__FUNCTION__); printf arg; }
#else
#define Dprintf(arg) {}
#endif

PinMonitor::PinMonitor()
{
}

PinMonitor::~PinMonitor()
{
    // Release all of the sinks:
    list <SignalSink *> :: iterator ssi = sinks.begin();
    while( ssi != sinks.end())
    {
        Dprintf(("release sink %p\n", *ssi));
        fflush(stdout);
        ( *ssi)->release();
        ++ssi;
    }
}

void PinMonitor::addSink( SignalSink* new_sink )
{
    if( new_sink ) sinks.push_back( new_sink );
}

void PinMonitor::removeSink( SignalSink* pSink )
{
    if( pSink ) sinks.remove(pSink);
}

//========================================================================
//
IOPIN::IOPIN( const char* name , IOPIN_TYPE m_type )
    : bDrivenState( false ),
      m_monitor(0),
      l2h_threshold(2.0),       // PICs are CMOS and use CMOS-like thresholds
      h2l_threshold(1.0),
      Vdrive_high(4.4),
      Vdrive_low(0.6),
      m_type( m_type )
{
    bPullUp = false;
    is_analog = false;
    m_picPin = 0l;
    name_str = name;
}

IOPIN::~IOPIN()
{
    if( m_monitor )((PinModule*)m_monitor)->clrPin();
}

void IOPIN::set_digital_threshold(double vdd)
{
    set_l2h_threshold(vdd > 4.5 ? 2.0 : 0.25 * vdd + 0.8);
    set_h2l_threshold(vdd > 4.5 ? 0.8 : 0.15 * vdd);
    Vdrive_high = vdd - 0.6;
    Vdrive_low = 0.6;
}

void IOPIN::setMonitor(PinMonitor *new_pinMonitor)
{
    if( m_monitor && new_pinMonitor) cout << "IOPIN already has a monitor!" << endl;
    else  m_monitor = new_pinMonitor;
}

void IOPIN::get( char *return_str, int len )
{
    if( return_str )
    {
        if( get_direction() == DIR_OUTPUT)
            strncpy(return_str, IOPIN::getDrivingState()?"1": "0", len);
        else
            strncpy(return_str, IOPIN::getState()?"1": "0", len);
    }
}

void IOPIN::set_nodeVoltage( double nodeVoltage )
{
    m_nodeVoltage = nodeVoltage;

    if     ( nodeVoltage < h2l_threshold ) setDrivenState(false); // The voltage is below the low threshold
    else if( nodeVoltage > l2h_threshold ) setDrivenState(true); // The voltage is above the high threshold
}

// putState - called by peripherals when they wish to drive an I/O pin to a new state.

void IOPIN::putState( bool new_state )
{
    bDrivingState = new_state;

    if( m_monitor ) m_monitor->putState( new_state?'1':'0' );
}


bool IOPIN::getState()
{
    return bDriving ? getDrivingState() : getDrivenState();
}

void IOPIN::setDrivingState( bool new_state )
{ 
    bDrivingState = new_state;
    if(m_monitor) m_monitor->setDrivingState(bDrivingState?'1':'0');
}

void IOPIN::setDrivingState( char new3State )
{
    bDrivingState =( new3State=='1' || new3State=='W');

    if( m_picPin ) m_picPin->setState( bDrivingState ); // SimulIDE Pic pin
    if( m_monitor ) m_monitor->setDrivingState( new3State );
}

bool IOPIN::getDrivingState(void)
{
    return bDrivingState;
}

bool IOPIN::getDrivenState()
{
    return bDrivenState;
}

// An stimulus attached to this pin is driving us to a new state.
// This state will be recorded and propagate up to anything 
// monitoring this pin.

void IOPIN::setDrivenState( bool new_state )
{
    bDrivenState = new_state;

    // Propagate the new state to those things monitoring this pin.
    //( note that the 3-state value is what's propagated).
    if( m_monitor  /*&& !is_analog*/ ) // SimulIDE
    {
        m_monitor->setDrivenState( getBitChar() );
    }
}

void IOPIN::toggle()
{
    putState((bool)( getState() ^ true));
}

char IOPIN::getBitChar()
{
    return getDrivenState() ? '1' : '0';
}

void IOPIN::update_pullup( char new_state, bool refresh )
{
    bool bNewPullupState = new_state == '1' || new_state == 'W';

    if( bPullUp != bNewPullupState )
    {
        bPullUp = bNewPullupState;
        if( m_picPin ) m_picPin->setPullup( bPullUp ); // SimulIDE Pic pin
    }
}

void IOPIN::set_is_analog( bool flag )
{
    is_analog = flag;
}

void IOPIN::update_direction( uint new_direction, bool refresh )
{
    bool out = new_direction ? true : false;

    bDriving = out;
    m_direction = out ? DIR_OUTPUT : DIR_INPUT;

    if( m_picPin ) m_picPin->setDirection( out ); // SimulIDE Pic pin
}

/* getBitChar() returns bit status as follows
     Input pin
        1> Pin considered floating,
           return 'Z'
        2> Weak Impedance on pin,
           return 'W" if high or 'w' if low
        3> Pin being driven externally
           return '1' node voltage high '0' if low
     Output pin
        1> Node voltage opposite driven value
           return 'X' if node voltage high or 'x' if inode voltage low
        2> Node voltage same as driven value
           return '1' node voltage high '0' if low
*/


