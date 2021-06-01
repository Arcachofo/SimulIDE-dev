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

#include "spimodule.h"
#include "iopin.h"
#include "simulator.h"

SpiModule::SpiModule( QString name )
         : eClockedDevice( name )
{
    m_MOSI = NULL;
    m_MISO = NULL;
    //m_SCK  = NULL;
    m_SS   = NULL;

    m_dataOut = NULL;
    m_dataIn  = NULL;
}
SpiModule::~SpiModule( ){}

void SpiModule::initialize()
{
    m_mode = SPI_OFF;

    m_lsbFirst = false;
    m_bitInc = 1;

    m_leadEdge   = Clock_Rising;
    m_sampleEdge = Clock_Rising;
}

void SpiModule::stamp()      // Called at Simulation Start
{ /* We are just avoiding eClockedDevice::stamp() call*/ }

void SpiModule::runEvent()
{
    if( m_mode != SPI_MASTER ) return;

}

void SpiModule::voltChanged()
{
    if( m_mode != SPI_SLAVE ) return;

    clkState_t clkState = eClockedDevice::getClockState();

    bool enabled = true;
    if( m_SS ) enabled = m_SS->getInpState() ? true : false;

    if( !enabled ) return;

    if( clkState == m_sampleEdge ) // Sample data
    {
        readBit();
    }
}

void SpiModule::readByte()
{

}

void SpiModule::readBit()
{
    /// if( m_bitPtr > 0 ) m_rxReg <<= 1;
    if( m_lsbFirst )
    {
        if( m_bitPtr < 8 ) m_rxReg >>= 1;
    }
    else
    {
        if( m_bitPtr > 0 ) m_rxReg <<= 1;
    }

    if( m_dataIn->getInpState() ) m_rxReg += 1;  //Read one bit
    m_bitPtr += m_bitInc;
}

void SpiModule::writeByte()
{

    if( m_mode == SPI_MASTER ) Simulator::self()->addEvent( m_clockPeriod, this ); // Start Clock
}

void SpiModule::writeBit()
{
    if( m_bitPtr < 0 ) { return; }

    bool bit = m_txReg>>m_bitPtr & 1;
    m_bitPtr += m_bitInc;

    m_dataOut->setOutState( bit );
}

void SpiModule::setMode( spiMode_t mode )
{
    if( mode == m_mode ) return;
    m_mode = mode;

    switch( mode ) {
    case SPI_OFF:
        {
            m_MOSI->changeCallBack( this, false );
            m_MISO->changeCallBack( this, false );
            m_clkPin->changeCallBack( this, false );
            if( m_SS ) m_SS->changeCallBack( this, false );
        }
        break;
    case SPI_MASTER:
        {
            if( !m_MOSI || !m_MISO || !m_clkPin )
            {
                m_mode = SPI_OFF;
                return;
            }
            m_dataOut = m_MOSI;
            m_dataIn  = m_MISO;
            // Simulator::self()->addEvent( m_clockPeriod, this ); // Start Clock
        }
        break;
    case SPI_SLAVE:
        {
            if( !m_MOSI || !m_MISO || !m_clkPin )
            {
                m_mode = SPI_OFF;
                return;
            }
            m_dataOut = m_MISO;
            m_dataIn  = m_MOSI;

            //m_MOSI->changeCallBack( this, true );
            m_clkPin->changeCallBack( this, true );
            if( m_SS ) m_SS->changeCallBack( this, true );
        }
        break;
    }
}

