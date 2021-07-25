/***************************************************************************
 *   Copyright (C) 2021 by santiago González                               *
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

#include "mcudataspace.h"

DataSpace::DataSpace()
{
}

DataSpace::~DataSpace()
{
    for( uint16_t addr : m_regSignals.keys() )
        delete m_regSignals.value( addr );

    m_regSignals.clear();
    m_dataMem.clear();
}

void DataSpace::initialize()
{
    std::fill( m_dataMem.begin(), m_dataMem.end(), 0 );
    /// std::fill( m_sreg.begin(), m_sreg.end(), 0 );

    for( QString regName : m_regInfo.keys() )  // Set Registers Reset Values
    {
        regInfo_t regInfo = m_regInfo[regName];
        writeReg( regInfo.address, regInfo.resetVal );
    }
}

uint8_t DataSpace::readReg( uint16_t addr )
{
    regSignal_t* regSignal = m_regSignals.value( addr );
    if( regSignal ) regSignal->on_read.emitValue( m_dataMem[addr] );

    return m_dataMem[addr];
}

void DataSpace::writeReg( uint16_t addr, uint8_t v )
{
    regSignal_t* regSignal = m_regSignals.value( addr );
    if( regSignal )
    {
        m_regOverride = -1;
        regSignal->on_write.emitValue( v );
        if( m_regOverride >= 0 ) v = (uint8_t)m_regOverride; // Value overriden in callback
    }
    m_dataMem[addr] = v;
}

void DataSpace::readStatus( uint8_t v ) // Read SREG values and write to RAM
{
    uint8_t val = 0;
    for( int i=0; i<8; i++ ) { if( m_sreg[i] ) val |= 1<<i; }
    m_dataMem[m_sregAddr] = val;
}

void DataSpace::writeStatus( uint8_t v ) // Write SREG values from RAM
{
    for( int i=0; i<8; i++ ) m_sreg[i] = v & (1<<i);
}
