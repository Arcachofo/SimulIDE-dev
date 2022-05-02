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

#include "avrspi.h"
#include "datautils.h"
#include "iopin.h"
#include "e_mcu.h"
#include "mcuinterrupts.h"

AvrSpi::AvrSpi( eMcu* mcu, QString name )
      : McuSpi( mcu, name )
{
    QString n = m_name.right(1);
    bool ok = false;
    n.toInt( &ok );
    if( !ok ) n = "";

    m_SPE  = getRegBits( "SPE"+n, mcu );
    m_DODR = getRegBits( "DODR"+n, mcu );
    m_MSTR = getRegBits( "MSTR"+n, mcu );
    m_CPOL = getRegBits( "CPOL"+n, mcu );
    m_CPHA = getRegBits( "CPHA"+n, mcu );
}
AvrSpi::~AvrSpi(){}

void AvrSpi::setMode( spiMode_t mode )
{
    if     ( mode == m_mode ) return;
    else if( mode == SPI_OFF )
    {
        m_MOSI->controlPin( false, false );
        m_MISO->controlPin( false, false );
        m_clkPin->controlPin( false, false );
        m_SS->controlPin( false, false );
    }
    else if( mode == SPI_MASTER )
    {
        m_MOSI->controlPin( true, false );
        m_MISO->setPinMode( input );
        m_MISO->controlPin( true, true );
        m_clkPin->controlPin( true, false );
        //m_SS->controlPin( true, false );
    }
    else if( mode == SPI_SLAVE )
    {
        m_MOSI->setPinMode( input );
        m_MOSI->controlPin( true, true );
        m_clkPin->setPinMode( input );
        m_clkPin->controlPin( true, true );
        m_SS->setPinMode( input );
        m_SS->controlPin( true, true );
        m_MISO->controlPin( true, false );
    }
    SpiModule::setMode( mode );
}

void AvrSpi::configureA( uint8_t newSPCR ) // SPCR is being written
{
    bool enable = getRegBitsVal( newSPCR, m_SPE );
    if( !enable )                 /// Disable SPI
    { setMode( SPI_OFF ); return; }

    bool master = getRegBitsVal( newSPCR, m_MSTR );
    spiMode_t mode = master ? SPI_MASTER : SPI_SLAVE;
    setMode( mode );

    m_lsbFirst  = getRegBitsVal( newSPCR, m_DODR ); // Data order

    bool clkPol = getRegBitsVal( newSPCR, m_CPOL ); // Clock polarity
    m_leadEdge = clkPol ? Clock_Falling : Clock_Rising;
    m_tailEdge = clkPol ? Clock_Rising : Clock_Falling;
    m_clkPin->sheduleState( clkPol );

    bool clkPha = getRegBitsVal( newSPCR, m_CPHA ); // Clock phase
    m_sampleEdge = ( clkPol == clkPha ) ? Clock_Rising : Clock_Falling; // This shows up in the truth table

    m_prescaler = m_prescList[newSPCR & 0b00000011];
    m_clockPeriod = m_mcu->simCycPI()*m_prescaler/2;
}

void AvrSpi::writeStatus( uint8_t newSPSR ) // SPSR is being written
{
    m_mcu->m_regOverride = newSPSR | (*m_statReg & 0b00000001); // Preserve Status bits
}

void AvrSpi::writeSpiReg( uint8_t newSPDR ) // SPDR is being written
{
    m_txReg = newSPDR;

    /// SPIF is cleared by first reading the SPI Status Register with SPIF set,
    /// then accessing the SPI Data Register (SPDR).
    m_interrupt->clearFlag();          // Clear Iterrupt flag

    if( m_mode == SPI_MASTER ) StartTransaction();
}

void AvrSpi::endTransaction()
{
    SpiModule::endTransaction();
    m_interrupt->raise();
}
