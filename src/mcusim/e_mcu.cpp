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

#include "e_mcu.h"
#include "mcucore.h"
#include "mcupin.h"
#include "usartmodule.h"

#include "simulator.h"

eMcu::eMcu( QString id )
    : McuInterface( id )
    , DataSpace()
    , m_interrupts( this )
    , m_ports( this )
    , m_timers( this )
    //, m_usarts( this )
{
    m_regStart = 0xFFFF;
    m_regEnd   = 0;

    m_cPerInst = 1;
    setFreq( 16.0 );
}

eMcu::~eMcu()
{
    delete cpu;
    m_interrupts.remove();
    for( McuModule* module : m_modules ) delete module;
}

void eMcu::initialize()
{
    m_cycle = 0;
    cyclesDone = 0;
    cpu->reset();

    DataSpace::initialize();

    m_interrupts.resetInts();

    Simulator::self()->addEvent( 1, this );
}

void eMcu::runEvent()
{
    //if( m_state == cpu_Running )
    {
        uint32_t pc = cpu->PC;
        //if( cyclesDone > 1 ) cyclesDone -= 1;
        //else
        if( pc < m_flashSize )
        {
            cpu->runDecoder();              // Run Decoder

            m_interrupts.runInterrupts();     // Run Interrupts
        }
        m_cycle += cyclesDone;
        Simulator::self()->addEvent( cyclesDone*m_simCycPI, this );
    }
}

int eMcu::status() { return getRamValue( m_sregAddr ); }

int eMcu::pc() { return cpu->PC; }

void eMcu::setFreq( double freq )
{
    if     ( freq < 0  )  freq = 0;
    else if( freq > 100 ) freq = 100;

    m_freq = freq;
    m_simCycPI = 1e6*(m_cPerInst/m_freq); // Set Simulation cycles per Instruction cycle
}

void eMcu::uartOut( int number, uint32_t value ) // Send value to OutPanelText
{
    //if( (uint)number > m_usarts.size() ) return;
    //m_usarts.at(number-1)->byteSent( value );
}

void eMcu::uartIn( int number, uint32_t value ) // Receive one byte on Uart
{
    if( (uint)number > m_usarts.size() ) return;
    m_usarts.at(number-1)->byteReceived( value );
}

