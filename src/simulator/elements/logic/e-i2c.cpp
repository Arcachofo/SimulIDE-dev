/***************************************************************************
 *   Copyright (C) 2017 by Santiago González                               *
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

#include "e-i2c.h"
#include "simulator.h"

eI2C::eI2C( QString id )
    : eLogicDevice( id )
{
    m_comp = 0l;
    m_enabled = true;
    m_freq = 1000000;
}
eI2C::~eI2C() { }

void eI2C::initialize()
{
    eLogicDevice::initialize();

    double stepsPerS = 1e12;
    m_stepsPe   = stepsPerS/m_freq/2;
    m_propDelay = m_stepsPe/2;
    
    m_txReg  = 0;
    m_rxReg  = 0;
    m_bitPtr = 0;
    m_SDA    = false;
    m_lastSDA = false;
    m_state = I2C_IDLE;
}

void eI2C::readBit()
{
    if( m_bitPtr > 0 ) m_rxReg <<= 1;
    m_rxReg += m_SDA;                            //Read one bit from sda
    m_bitPtr++;
}

void eI2C::writeBit()
{
    if( m_bitPtr < 0 ) { waitACK(); return; }

    bool bit = m_txReg>>m_bitPtr & 1;
    m_bitPtr--;

    if( bit != m_SDA ) setSDA( bit ); // Save some events
}

void eI2C::readByte()
{
    m_bitPtr = 0;
    ACK();
}

void eI2C::writeByte()
{
    m_bitPtr = 7;
}

void eI2C::ACK()
{
    m_lastState = m_state;
    m_state = I2C_ACK;
}

void eI2C::waitACK()
{
    setSDA( true );
    m_lastState = m_state;
    m_state = I2C_WAITACK;
}

void eI2C::I2Cstop()
{
    m_state = I2C_STOPPED;
}

void eI2C::setEnabled( bool en )
{
    if( m_enabled == en ) return;
    m_enabled = en;
    updatePins();
}

void eI2C::setFreq( double f )
{
    m_freq = f*1e3;
    double stepsPerS = 1e12;
    m_stepsPe = stepsPerS/m_freq/2;
    m_propDelay = m_stepsPe/2;
}

