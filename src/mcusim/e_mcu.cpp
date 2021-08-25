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
#include "mcuwdt.h"
#include "usartmodule.h"
#include "usartrx.h"
#include "simulator.h"

eMcu::eMcu( QString id )
    : McuInterface( id )
    , DataSpace()
    , m_interrupts( this )
    , m_ports( this )
    , m_timers( this )
    //, m_usarts( this )
{
    m_cPerInst = 1;
    setFreq( 16.0 );
}

eMcu::~eMcu()
{
    delete cpu;
    m_interrupts.remove();
    for( McuModule* module : m_modules ) delete module;
    for( McuUsart*   usart : m_usarts )  delete usart;
}

void eMcu::initialize()
{
    m_debugStep = false;
    m_cycle = 0;
    cyclesDone = 0;

    cpu->reset();
    DataSpace::initialize();
    m_interrupts.resetInts();

    Simulator::self()->addEvent( 1, this );
}

void eMcu::runEvent()
{
    if( m_debugging )
    {
        if( cyclesDone > 1 ) cyclesDone -= 1;
        else                 stepDebug();
        Simulator::self()->addEvent( m_simCycPI, this );
    }
    else
    {
        stepCpu();//if( m_state == cpu_Running )
        Simulator::self()->addEvent( cyclesDone*m_simCycPI, this );
    }
}
void eMcu::stepCpu()
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

uint16_t eMcu::getRegAddress( QString reg )// Get Reg address by name
{
    uint16_t addr = McuInterface::getRegAddress( reg );
    if( addr < 65535 ) return (uint16_t)addr;

    return m_regInfo.value( reg ).address;
}

uint8_t eMcu::getRamValue( int address )
{
    return m_dataMem[getMapperAddr(address)];   //m_dataMem[address];
}

void eMcu::setRamValue( int address, uint8_t value ) // Setting RAM from external source (McuMonitor)
{
    writeReg( getMapperAddr(address), value );
}

void eMcu::wdr()
{
    m_wdt->reset();
}

void eMcu::enableInterrupts( uint8_t en )
{
    if( en > 1 ) en = 1;
    m_interrupts.enableGlobal( en );
}

