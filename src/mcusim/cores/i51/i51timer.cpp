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

#include "i51timer.h"
#include "simulator.h"

I51Timer::I51Timer( eMcu* mcu )
        : McuTimer( mcu )
{
}
I51Timer::~I51Timer(){}

void I51Timer::initialize()
{
    McuTimer::initialize();
    configure( 0 );           // Defautl mode = 0
}

void I51Timer::configure( uint8_t val ) // T0M0,T0M1,C/T0,GATE0
{
    uint8_t mode = val & 0b00000011;
    bool  clkSrc = val & 0b00000100;
    bool    gate = val & 0b00001000;

    if( mode != m_mode )
    {
        m_mode = mode;

        if     ( mode == 0 )  // 13 bits
        {
            m_ovfMatch = 0x1FFF;
        }
        else if( mode == 1 ) // 16 bits
        {
            m_ovfMatch = 0xFFFF;
        }
        else if( mode == 2 ) // 8 bits
        {
            m_ovfMatch = 0x00FF;
        }
        else                 // 8+8 bits
        {
            if( m_name == "TIMER0" )
            {
            }
            else if( m_name == "TIMER1" )
            {
            }
        }
        m_ovfPeriod = m_ovfMatch + 1;
    }
    if( clkSrc != m_clkSrc )
    {
        m_clkSrc = clkSrc;
    }
    if( gate != m_gate )
    {
        m_gate = gate;
    }
}

void I51Timer::updtCycles() // Recalculate ovf, comps, etc
{
    if     ( m_mode == 0 )  // 13 bits
    {
        m_countVal  = m_countValH << 5;
        m_countVal |= m_countValL & 0b00011111;
    }
    else if( m_mode == 1 ) // 16 bits
    {
        m_countVal  = m_countValH << 8;
        m_countVal |= m_countValL;
    }
    else if( m_mode == 2 ) // 8 bits
    {
        //m_ovfMatch = m_ovfPeriod-m_countH;
        //m_countVal = m_countL;
    }
    else                 // 8+8 bits
    {
        if( m_name == "TIMER0" )
        {
        }
        else if( m_name == "TIMER1" )
        {
        }
    }
    sheduleEvents();
}

