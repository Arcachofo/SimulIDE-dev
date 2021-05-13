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

#include "simulator.h"

eMcu::eMcu( QString id )
    : McuInterface( id )
    , DataSpace()
    , m_interrupts( this )
    , m_ports( this )
    , m_timers( this )
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
    m_timers.remove();
    m_ports.remove();

    m_progMem.clear();
}

void eMcu::initialize()
{
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
        if( cyclesDone > 1 ) cyclesDone -= 1;

        else if( cpu->PC < m_flashSize )
        {
            cpu->runDecoder();              // Run Decoder

            m_interrupts.runInterrupts();     // Run Interrupts
        }
        Simulator::self()->addEvent( m_simCycPI, this );
    }
}

uint8_t eMcu::getRamValue( int address )
{
    return m_dataMem[address];
}

void eMcu::setRamValue( int address, uint8_t value )
{
    /// TODO (used by McuMonitor)
}

uint16_t eMcu::getFlashValue( int address )
{
    return m_progMem[address];
}

void eMcu::setFlashValue( int address, uint16_t value )
{
    m_progMem[address] = value;
}

uint8_t eMcu::getRomValue( int address )
{
    return 0;
}

void eMcu::setRomValue( int address, uint8_t value )
{

}

void eMcu::enableInterrupts( uint8_t en )
{
    m_interrupts.enableGlobal( en );
}

int eMcu::status()
{
    readStatus( 0 );
    return getRamValue( m_sregAddr );
}

int eMcu::pc() { return cpu->PC; }

void eMcu::setFreq( double freq )
{
    if     ( freq < 0  )  freq = 0;
    else if( freq > 100 ) freq = 100;

    m_freq = freq;
    m_simCycPI = 1e6*(m_cPerInst/m_freq); // Set Simulation cycles per Instruction cycle
}
