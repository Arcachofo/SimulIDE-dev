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

DataSpace::DataSpace(){}

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

    for( QString regName : m_regInfo.keys() )  // Set Registers Reset Values
    {
        regInfo_t regInfo = m_regInfo[regName];
        writeReg( regInfo.address, regInfo.resetVal );
        m_dataMem[regInfo.address] = regInfo.resetVal;
}   }

uint8_t DataSpace::readReg( uint16_t addr )
{
    uint8_t v = m_dataMem[addr];
    regSignal_t* regSignal = m_regSignals.value( addr );
    if( regSignal )
    {
        m_regOverride = -1;
        regSignal->on_read.emitValue( v );
        if( m_regOverride >= 0 ) v = (uint8_t)m_regOverride; // Value overriden in callback
        else                     v = m_dataMem[addr]; // Timers update their counters in callback
    }
    return v;
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
    uint8_t mask = m_regMask[addr];
    if( mask != 0xFF ) v = (m_dataMem[addr] & ~mask) | (v & mask);
    m_dataMem[addr] = v;
}
