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
#include "e_mcu.h"
#include "simulator.h"
#include "datautils.h"

#define COUNT_L m_countL[0]
#define COUNT_H m_countH[0]

I51Timer::I51Timer( eMcu* mcu, QString name)
        : McuTimer( mcu, name )
{
}
I51Timer::~I51Timer(){}

void I51Timer::initialize()
{
    McuTimer::initialize();
    //configure( 0 );           // Defautl mode = 0
    m_ovfMatch = 0x1FFF;
    m_ovfPeriod = m_ovfMatch + 1;
    m_gate = 0;
}

void I51Timer::configureA( uint8_t val ) // TxM0,TxM1
{
    uint8_t mode = getRegBitsVal( val, m_configBitsA );//val & 0b00000011;

    if( mode == m_mode ) return;
    m_mode = mode;

    switch( mode )
    {
        case 0: m_ovfMatch = 0x1FFF; break; // 13 bits
        case 1: m_ovfMatch = 0xFFFF; break; // 16 bits
        case 2: m_ovfMatch = 0x00FF; break; // 8 bits
        case 3:                             // 8+8 bits
        {
            m_ovfMatch = 0x00FF;

            if( m_number == 0 )
            {
            }
            else if( m_number == 1 )
            {
            }
        }
    }
    m_ovfPeriod = m_ovfMatch+1;
}

void I51Timer::configureB( uint8_t val ) // C/Tx,GATEx
{
    val = getRegBitsVal( val, m_configBitsB );
    bool  clkSrc = val & 1;
    bool    gate = val & 2;

    if( clkSrc != m_clkSrc )
    {
        if( clkSrc ) m_clkSrc = clkEXT;
        else         m_clkSrc = clkMCU;
    }
    if( gate != m_gate )
    {
        m_gate = gate;
        /// TODO
    }
}

void I51Timer::updtCycles() // Recalculate ovf, comps, etc
{
    switch( m_mode )
    {
        case 0:  // 13 bits
        {
            m_countVal  = COUNT_H << 5;
            m_countVal |= COUNT_L & 0b00011111;
            m_countStart = 0;
        } break;
        case 1: // 16 bits
        {
            m_countVal  = COUNT_H << 8;
            m_countVal |= COUNT_L;
            m_countStart = 0;
        } break;
        case 2: // 8 bits
        {
            //m_ovfMatch = m_ovfPeriod-m_countH;
            m_countVal   = COUNT_H;
            m_countStart = COUNT_H;
        } break;
        case 3: // 8+8 bits
        {
            if     ( m_number == 0 ) m_countVal = COUNT_L;
            else if( m_number == 1 ) m_countVal = COUNT_H;
            m_countStart = 0;
        }
    }
    sheduleEvents();
}

void I51Timer::updtCount( uint8_t )     // Write counter values to Ram
{
    if( m_running ) // If no running, values were already written at timer stop.
    {
        if( m_mode == 1 ) // 16 bits
        {
            McuTimer::updtCount();
            return;
        }
        uint64_t timTime = m_ovfCycle-Simulator::self()->circTime(); // Next overflow time - current time
        uint16_t countVal = timTime/m_mcu->simCycPI()/m_prescaler;

        if( m_mode == 0 )  // 13 bits
        {
            COUNT_L = countVal & 0b00011111;
            COUNT_H = (countVal>>5) & 0xFF;
        }
        else if( m_mode == 2 ) // 8 bits
        {
            COUNT_L = countVal & 0xFF;
            //if( m_countH ) m_countH[0] = (countVal>>8) & 0xFF;
        }
        else                 // 8+8 bits
        {
            if     ( m_number == 0 ) COUNT_L = countVal & 0xFF;
            else if( m_number == 1 ) COUNT_H = countVal & 0xFF;
        }
    }
}
