/*
   Copyright (C) 1998 T. Scott Dattalo

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


#ifndef __STIMULI_H__
#define __STIMULI_H__

#include <string>
using namespace std;
#include <list>

#include "gpsim_classes.h"
#include "trigger.h"
#include "piccomponentpin.h"

//class stimulus;
class IOPIN;

/****************************************************************************
 *
 * Include file support stimuli.
 *
 *       stimulus             TriggerObject
 *       |  \                      /
 *       |   -----------------+----
 *       |                    |
 *       |- IOPIN             |- source_stimulus
 *          |                              |
 *          |- IO_input                    |- square_wave
 *             |                           |- triangle_wave
 *             |- IO_open_collector        |- asynchronous_stimulus
 *             |- IO_bi_directional        |- dc_supply
 *                |                        |- open_collector
 *                |- IO_bi_directional_pu
 *
 *  A stimulus is used to stimulate stimuli. What's that mean? Well,
 * in gpsim, the pic I/O pins are derived from the stimulus base class
 * (as can be seen from above). The I/O pins are what interface to the
 * 'external' world. In some cases, I/O pins are inputs and others they're
 * outputs. The stimulus base class defines the basic functionality of
 * a stimulus and how this interface to the outside world is to occur.
 *
 */

#define MAX_DRIVE        0x100000
#define MAX_ANALOG_DRIVE 0x1000


///------------------------------------------------------------
///
/// SignalSink - A pure virtual class that allows signals driven by external
/// stimuli to be routed to one or more objects monitoring them (e.g. one
/// sink may be a bit in a port register while another may be a peripheral)

class SignalSink
{
public:
    virtual ~SignalSink(){}

    virtual void setSinkState(char)=0;
    virtual void release()=0;
};


///------------------------------------------------------------
/// The PinMonitor class allows other objects to be notified whenever
/// a Pin changes states.
/// (Note: In older versions of gpsim, iopins notified the Port registers
/// in which they were contained by direcly calling the register setbit()
/// method. This is deprecated - and eventually will cause compile time errors.)
class PinMonitor
{
public:
    PinMonitor();
    virtual ~PinMonitor();

    void addSink( SignalSink* );
    void removeSink (SignalSink* );

    virtual void setDrivenState( char )=0;
    virtual void setDrivingState( char )=0;
    virtual void set_nodeVoltage( double )=0;
    virtual void putState(char)=0;
    virtual void setDirection()=0;

protected:
    /// The SignalSink list is a list of all sinks that can receive digital data
    list <SignalSink *> sinks;
};


class PICComponentPin;
class IOPIN //: public stimulus
{
public:
    IOPIN( const char* n="0", IOPIN_TYPE m_type=BI_DIRECTIONAL );
    virtual ~IOPIN();

    enum IOPIN_DIRECTION
    {
        DIR_INPUT,
        DIR_OUTPUT
    };

    virtual void setMonitor( PinMonitor* );
    virtual PinMonitor* getMonitor() { return m_monitor; }

    virtual void set_nodeVoltage(double v);

    virtual bool getDrivingState( void );
    virtual void setDrivingState( bool new_dstate );
    virtual void setDrivingState( char );
    virtual bool getDrivenState( void );
    virtual void setDrivenState( bool new_dstate );

    virtual bool getState();
    virtual void putState( bool new_dstate );

    virtual void set_digital_threshold ( double vdd );
    virtual void get( char *return_str, int len );

    virtual void set_l2h_threshold(double V) { l2h_threshold=V; }
    virtual double get_l2h_threshold()       { return l2h_threshold;}
    virtual void set_h2l_threshold(double V) { h2l_threshold=V; }
    virtual double get_h2l_threshold()       { return h2l_threshold;}

    virtual void toggle(void);

    virtual void update_direction( uint x, bool refresh );
    virtual void update_pullup( char new_state, bool refresh );
    virtual void set_is_analog( bool flag );
    virtual IOPIN_DIRECTION  get_direction(void) {return m_direction;/*((getDriving()) ? DIR_OUTPUT : DIR_INPUT);*/}

    virtual char getBitChar();
    virtual IOPIN_TYPE getType() { return m_type;}

    void setPicPin( PICComponentPin* pin ){ m_picPin = pin; }

    double get_nodeVoltage() { return m_nodeVoltage; }
    virtual bool getDriving() { return bDriving; }
    virtual void setDriving( bool d ) { bDriving = d; }

    PICComponentPin* m_picPin;

    string name_str;

protected:
    bool is_analog;          // Pin is in analog mode
    bool bDrivenState;       // binary state we're being driven to

    PinMonitor* m_monitor;

    // These are the low to high and high to low input thresholds. The units are volts.
    double l2h_threshold;
    double h2l_threshold;
    double Vdrive_high;
    double Vdrive_low;

    bool bDrivingState;        // 0/1 digitization of the analog state we're driving
    bool bDriving;             // True if this stimulus is a driver
    double m_nodeVoltage;        // The voltage driven on to this stimulus by the snode
    bool bPullUp;
    IOPIN_DIRECTION m_direction;
    IOPIN_TYPE m_type;
};

class IO_bi_directional : public IOPIN
{
public:

    IO_bi_directional(const char *n=0 ){}
};

class IO_bi_directional_pu : public IO_bi_directional
{
public:
    IO_bi_directional_pu(const char *n=0){}
    ~IO_bi_directional_pu(){}
};

#endif  // __STIMULI_H__
