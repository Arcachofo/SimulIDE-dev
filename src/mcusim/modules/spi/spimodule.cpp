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
    m_SS   = NULL;

    m_dataOut = NULL;
    m_dataIn  = NULL;
}
SpiModule::~SpiModule( ){}

void SpiModule::initialize()
{
    m_mode = SPI_OFF;

    m_toggleSck = false;
    m_lsbFirst  = false;
    m_enabled   = false;

    m_leadEdge   = Clock_Rising;
    m_tailEdge   = Clock_Falling;
    m_sampleEdge = Clock_Rising;
}

void SpiModule::stamp()      // Called at Simulation Start
{ /* We are just avoiding eClockedDevice::stamp() call*/ }

void SpiModule::keepClocking()
{
    m_toggleSck = true;
    Simulator::self()->addEvent( m_clockPeriod, this );
}

void SpiModule::runEvent()
{
    if( m_mode != SPI_MASTER ) return;

    if( m_toggleSck )
    {
        m_clkPin->toggleOutState();
        m_toggleSck = false;

        m_clkState = m_clkPin->getOutState() ? Clock_Rising : Clock_Falling;

        step();
    }
}

void SpiModule::voltChanged() // Called in Slave mode on SCK or SS changes
{
    if( m_mode != SPI_SLAVE ) return;

    updateClock();

    bool enabled = m_SS->getInpState() ? false : true;  // SS active LOW

    if( enabled != m_enabled ) // Enabling or Disabling
    {
        if( enabled && !m_enabled ) // Enabling
        {
            StartTransaction(); // Start transaction
        }
        else                        // Disabling
        {
            ;// Reset SPI Logic
        }
        m_enabled = enabled;
    }
    if( !enabled ) return;
    if( (m_clkState == Clock_High) || (m_clkState == Clock_Low) ) return; // Not an Edge

    step();
}

void SpiModule::endTransaction()
{
    m_dataOut->setOutState( true );
}

void SpiModule::StartTransaction()
{
    if( m_lsbFirst )
    {
        m_bitPtr = 1;
        m_endOfByte = 256;
    }else{
        m_bitPtr = 256;
        m_endOfByte = 1;
    }
    Simulator::self()->cancelEvents( this );
    if( m_sampleEdge == m_leadEdge ) // Sample in first Leading Edge => setup now
    {
        m_clkState = m_tailEdge; // Force setup
        step();
    }
    else keepClocking();
}

void SpiModule::step()
{
    if( m_clkState == m_tailEdge ) // Rotate bit mask
    {
        if( m_bitPtr == m_endOfByte ) // Check end of byte
        {
            endTransaction();
            return;
        }
        if( m_lsbFirst ) m_bitPtr <<= 1;
        else             m_bitPtr >>= 1;
    }
    if( m_clkState == m_sampleEdge )         //Read one bit
    {
        if( m_dataIn->getInpState() ) m_rxReg |= m_bitPtr;
    }
    else if( m_dataOut )                     // Write one bit (Only if dataOut Pin exist)
        m_dataOut->sheduleState( m_txReg & m_bitPtr, 0 );

    if( m_mode == SPI_MASTER ) keepClocking();
}

void SpiModule::setMode( spiMode_t mode )
{
    if( mode == m_mode ) return;
    m_mode = mode;

    m_dataOut = NULL;
    m_dataIn  = NULL;

    switch( mode ) {
    case SPI_OFF:
        {
            if( m_MOSI )   m_MOSI->changeCallBack( this, false );
            if( m_MISO )   m_MISO->changeCallBack( this, false );
            if( m_clkPin ) m_clkPin->changeCallBack( this, false );
            if( m_SS )     m_SS->changeCallBack( this, false );
        }
        break;
    case SPI_MASTER:
        {
            if( !m_MOSI || !m_MISO || !m_clkPin ) { m_mode = SPI_OFF; return; }
            m_dataOut = m_MOSI;
            m_dataIn  = m_MISO;
        }
        break;
    case SPI_SLAVE:
        {
            if( !m_MOSI || !m_clkPin ) { m_mode = SPI_OFF; return; }
            m_dataOut = m_MISO;
            m_dataIn  = m_MOSI;

            m_clkPin->changeCallBack( this, true );
            if( m_SS ) m_SS->changeCallBack( this, true );
        }
        break;
    }
    if( m_dataOut ) m_dataOut->setOutState( true );
}

