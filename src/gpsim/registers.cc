/*
   Copyright (C) 1998-2003 Scott Dattalo

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
/****************************************************************
*                                                               *
*  Modified 2018 by Santiago Gonzalez    santigoro@gmail.com    *
*                                                               *
*****************************************************************/

#include <stdio.h>
#include <iostream>

#include <cstring>

#include "processor.h"
#include "registers.h"

// For now, initialize the register with valid data and set that data equal to 0.
// Eventually, the initial value will be marked as 'uninitialized.

Register::Register(Processor* cpu, const char *pName)
    : Value( pName, cpu)
    , value( RegisterValue(0, 0) )
    , address(AN_INVALID_ADDRESS)
    , alias_mask(0)
    , por_value(RegisterValue(0, 0))
    , m_replaced(0)
{
    mValidBits = 0xFF;
}
Register::~Register()
{
}

//  Return the contents of the file register.
// (note - breakpoints on file register reads are not checked here. Instead, a breakpoint
//  object replaces those instances of file registers for which we wish to monitor.
//  So a file_register::get call will invoke the breakpoint::get member function. Depending
//  on the type of break point, this get() may or may not get called).

uint Register::get()
{
    return(value.get());
}

//  Update the contents of the register.
//  See the comment above in file_register::get() with respect to break points

void Register::put(uint new_value)
{
    value.put(new_value);
}

//--------------------------------------------------
// set_bit
//
//  set a single bit in a register. Note that this
// is really not intended to be used on the file_register
// class. Instead, setbit is a place holder for high level
// classes that overide this function
void Register::setbit(uint bit_number, bool new_value)
{
    int set_mask = (1<<bit_number);

    if ( set_mask & mValidBits )
        value.put((value.get() & ~set_mask) | (new_value ? set_mask : 0));
}

//-----------------------------------------------------------
//  void Register::put_value(uint new_value)
//
//  put_value is used by the gui to change the contents of
// file registers. We could've let the gui use the normal
// 'put' member function to change the contents, however
// there are instances where 'put' has a cascading affect.
// For example, changing the value of an i/o port's tris
// could cause i/o pins to change states. In these cases,
// we'd like the gui to be notified of all of the cascaded
// changes. So rather than burden the real-time simulation
// with notifying the gui, I decided to create the 'put_value'
// function instead.
//   Since this is a virtual function, derived classes have
// the option to override the default behavior.
//
// inputs:
//   uint new_value - The new value that's to be
//                            written to this register
// returns:
//   nothing
//
//-----------------------------------------------------------

void Register::put_value(uint new_value)
{

    // go ahead and use the regular put to write the data.
    // note that this is a 'virtual' function. Consequently,
    // all objects derived from a file_register should
    // automagically be correctly updated.

    value.put(new_value);

    // Even though we just wrote a value to this register,
    // it's possible that the register did not get fully
    // updated (e.g. porta on many pics has only 5 valid
    // pins, so the upper three bits of a write are meaningless)
    // So we should explicitly tell the gui (if it's
    // present) to update its display.

    update();
}

void Register::new_name( const char* s )
{
    if( s )
    {
        string str( s );
        new_name( str );
    }
}

void Register::new_name(string &new_name)
{
    if( name_str != new_name ) name_str = new_name;
}
//------------------------------------------------------------------------
// set -- assgin the value of some other object to this Register
//
// This is used (primarily) during Register stimuli processing. If
// a register stimulus is attached to this register, then it will
// call ::set() and supply a Value pointer.

void Register::set( Value* pVal )
{
    Register *pReg = dynamic_cast<Register *>(pVal);

    if( pReg )
    {
        putRV( pReg->getRV() );
        return;
    }
    if( pVal ) put_value( (uint)* pVal );
}

void Register::get( int64_t &i )
{
    i = get_value();
}

//--------------------------------------------------
//--------------------------------------------------
SfrReg:: SfrReg( Processor* pCpu, const char *pName )
    : Register( pCpu, pName )
    , wdtr_value( 0,0xff )
{}

void  SfrReg::reset( RESET_TYPE r )
{
    switch( r )
    {
    case POR_RESET:
        putRV( por_value );
        break;

    default:
        if( wdtr_value.initialized() ) putRV( wdtr_value );// Most registers value on WDT resets.

        break;
    }
}

//--------------------------------------------------
// member functions for the InvalidRegister class
//--------------------------------------------------
void InvalidRegister::put(uint new_value)
{
    cout << "attempt write to invalid file register\n";

    if( address != AN_INVALID_ADDRESS) cout << "    address 0x" << hex << address << ',';
    cout << "   value 0x" << hex << new_value << endl;

    if( cpu->getBrkOnBadRegWrite()) cpu->halt();

    return;
}

uint InvalidRegister::get()
{
    cout << "attempt read from invalid file register\n";
    if( address != AN_INVALID_ADDRESS ) cout << "    address 0x" << hex << address << endl;

    if( cpu->getBrkOnBadRegRead()) cpu->halt();

    return(0);
}

InvalidRegister::InvalidRegister(Processor *pCpu, const char *pName )
    : Register( pCpu,pName )
{}


