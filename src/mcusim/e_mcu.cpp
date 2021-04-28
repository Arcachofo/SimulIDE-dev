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

    for( uint16_t addr : m_regSignals.keys() )
        delete m_regSignals.value( addr );

    m_regSignals.clear();
    m_progMem.clear();
    m_dataMem.clear();
}

void eMcu::initialize()
{
    cyclesDone = 0;
    cpu->reset();

    std::fill( m_dataMem.begin(), m_dataMem.end(), 0 );
    std::fill( m_sreg.begin(), m_sreg.end(), 0 );

    for( QString regName : m_regInfo.keys() )  // Set Registers Reset Values
    {
        regInfo_t regInfo = m_regInfo[regName];
        writeReg( regInfo.address, regInfo.resetVal );
    }
    m_interrupts.resetInts();

    Simulator::self()->addEvent( 1, this );
}

void eMcu::runEvent()
{
    //if( m_state == cpu_Running )
    {
        if( cyclesDone > 1 ) cyclesDone -= 1;

        else if( cpu->PC < m_progMemSize )
        {
            m_interrupts.runInterrupts();     // Run Interrupts

            cpu->runDecoder();              // Run Decoder
        }
        Simulator::self()->addEvent( m_simCycPI, this );
    }
}

uint8_t eMcu::getRamValue( int address )
{

}

void eMcu::setRamValue( int address, uint8_t value )
{

}

int eMcu::getFlashValue( int address )
{

}

void eMcu::setFlashValue( int address, uint8_t value )
{

}

uint8_t eMcu::getRomValue( int address )
{

}

void eMcu::setRomValue( int address, uint8_t value )
{

}

void eMcu::enableInterrupts( uint8_t en )
{
    m_interrupts.enableGlobal( en );
}

uint8_t eMcu::readReg( uint16_t addr )
{
    regSignal_t* regSignal = m_regSignals.value( addr );
    if( regSignal ) regSignal->on_read.emitValue( m_dataMem[addr] );

    return m_dataMem[addr];
}

void eMcu::writeReg( uint16_t addr, uint8_t v )
{
    regSignal_t* regSignal = m_regSignals.value( addr );
    if( regSignal ) regSignal->on_write.emitValue( v );

    m_dataMem[addr] = v;
}

void eMcu::readStatus( uint8_t v ) // Read SREG values and write to RAM
{
    uint8_t val = 0;
    for( int i=0; i<8; i++ ) val |= m_sreg[i];
    m_dataMem[m_sregAddr] = val;
}

void eMcu::writeStatus( uint8_t v ) // Write SREG values from RAM
{
    for( int i=0; i<8; i++ ) m_sreg[i] = v & (1<<i);
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
