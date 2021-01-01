/*
   Copyright( C) 1998 T. Scott Dattalo

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

#include "watchdog.h"
#include "pic-processor.h"

WDT::WDT( pic_processor* p_cpu, double _timeout )
    : cpu( p_cpu ), breakpoint(0),prescale(1), postscale(128), future_cycle(0),
      timeout(_timeout), wdte(false), cfgw_enable(false)
{
}

//--------------------------------------------------
void WDT::update()
{
    if( wdte )
    {  //cout << "WDT::update() : WDT Enabled\n\n";
        // FIXME - the WDT should not be tied to the instruction counter...
        uint64_t delta_cycles;

        if( !use_t0_prescale ) postscale = 1;

        delta_cycles =( uint64_t)(postscale*prescale*timeout/cpu->seconds_per_cycle());

        uint64_t fc = cpu->currentCycle() + delta_cycles ;

        if( future_cycle ) cpu->reassign_break( future_cycle, fc, this );
        else               cpu->setBreakAbs( fc, this );

        future_cycle = fc;
    }
}

//--------------------------------------------------
// WDT::put - shouldn't be called?
//
void WDT::put( uint new_value )
{
    cout << "WDT::put should not be called\n";
}

void WDT::set_timeout( double _timeout)
{
    timeout = _timeout;
    update();
}

void WDT::set_postscale( uint newPostscale ) //  TMR0 prescale is WDT postscale
{
    uint value = 1<< newPostscale;

    if( value != postscale )
    {
        postscale = value;
        update();
    }
}

void WDT::swdten( bool enable )
{
    if( cfgw_enable) return;

    if( wdte != enable )
    {
        wdte = enable;
        warned = 0;

        if( wdte ) update();
        else
        {
            if( future_cycle)
            {
                cout << "Disabling WDT\n";
                cpu->clear_break( this );
                future_cycle = 0;
            }
        }
    }
}

void WDT::set_prescale(uint newPrescale) // For WDT period select 0-11
{
    uint value = 1<<( 5 + newPrescale);
    if( value != prescale)
    {
        prescale = value;
        update();
    }
}

void WDT::initialize( bool enable, bool _use_t0_prescale )
{
    wdte = enable;
    cfgw_enable = enable;
    use_t0_prescale = _use_t0_prescale;
    warned = 0;

    if( wdte ) update();
    else
    {
        if( future_cycle)
        {
            cout << "Disabling WDT\n";
            cpu->clear_break( this );
            future_cycle = 0;
        }
    }
}

void WDT::reset(RESET_TYPE r)
{
    switch( r)
    {
    case POR_RESET:

    case EXIT_RESET:
        update();
        break;

    case MCLR_RESET:
        if( future_cycle) cpu->clear_break(this);
        future_cycle = 0;
        break;

    default:
        ;
    }
}

void WDT::set_breakpoint(uint bpn)
{
    breakpoint = bpn;
}

void WDT::callback()
{
    if( wdte )
    {
        if( breakpoint ) cpu->halt();

        else if( cpu->is_sleeping() && cpu->exit_wdt_sleep())
        {
            cout << "WDT expired during sleep\n";
            update();
            cpu->exit_sleep();
            cpu->status->put_TO(0);
        }
        else      // The TO bit gets cleared when the WDT times out.
        {
            cout << "WDT expired reset\n";
            update();
            cpu->status->put_TO(0);
            cpu->reset(WDT_RESET);
        }
    }
}

void WDT::clear()
{
    if( wdte ) update();

    else if(!warned)
    {
        warned = 1;
        cout << "The WDT is not enabled - clrwdt has no effect!\n";
    }
}

void WDT::callback_print()
{
    cout << " has callback, ID = " << CallBackID << '\n';
}

