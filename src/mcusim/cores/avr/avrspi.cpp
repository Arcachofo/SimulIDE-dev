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
//#include "mcupin.h"
#include "iopin.h"
#include "e_mcu.h"

AvrSpi::AvrSpi( eMcu* mcu, QString name )
      : McuSpi( mcu, name )
{
    m_SPCR = mcu->getReg( "SPCR" );

    m_SPIE = mcu->getRegBits( "SPIE" );
    m_SPE  = mcu->getRegBits( "SPE" );
    m_DODR = mcu->getRegBits( "DODR" );
    m_MSTR = mcu->getRegBits( "MSTR" );
    m_CPOL = mcu->getRegBits( "CPOL" );
    m_CPHA = mcu->getRegBits( "CPHA" );
}
AvrSpi::~AvrSpi(){}

void AvrSpi::initialize()
{
    McuSpi::initialize();
}

void AvrSpi::setMode( spiMode_t mode )
{
    if( mode == m_mode ) return;
    SpiModule::setMode( mode );

    if( mode == SPI_OFF )
    {
        m_MOSI->controlPin( false );
        m_MISO->controlPin( false );
        m_clkPin->controlPin( false );
        m_SS->controlPin( false );
    }
    else
    {
        m_MOSI->controlPin( true );
        m_MISO->controlPin( true );
        m_clkPin->controlPin( true );
        m_SS->controlPin( true );

        if     ( mode == SPI_MASTER ) m_MISO->setPinMode( input );
        else if( mode == SPI_SLAVE )
        {
            m_MOSI->setPinMode( input );
            m_clkPin->setPinMode( input );
            m_SS->setPinMode( input );
        }
    }
}

void AvrSpi::configureA( uint8_t newSPCR ) // SPCR is being written
{
    bool oldEn  = getRegBitsVal( *m_SPCR, m_SPE );
    bool enable = getRegBitsVal( newSPCR, m_SPE );
    if( oldEn && !enable )                 /// Disable SPI
    {
        setMode( SPI_OFF );
    }
    if( !enable ) return;           // Not enabled, do nothing

    bool oldMst = getRegBitsVal( *m_SPCR, m_MSTR );
    bool master = getRegBitsVal( newSPCR, m_MSTR );
    if( (master != oldMst) || !oldEn ) // Mode changed or enabled
    {
        spiMode_t mode = master ? SPI_MASTER : SPI_SLAVE;
        setMode( mode );
    }

    m_lsbFirst  = getRegBitsVal( newSPCR, m_DODR ); // Data order

    bool clkPol = getRegBitsVal( newSPCR, m_CPOL ); // Clock polarity
    m_leadEdge = clkPol ? Clock_Falling : Clock_Rising;

    bool clkPha = getRegBitsVal( newSPCR, m_CPHA ); // Clock phase
    m_sampleEdge = ( clkPol == clkPha ) ? Clock_Rising : Clock_Falling;

    m_prescaler = m_prescList[newSPCR & 0b00000011];
}

void AvrSpi::writeStatus( uint8_t newSPSR ) // SPSR is being written
{
    m_mcu->m_regOverride = newSPSR | (*m_statReg & 0b00000001); // Preserve Status bits
}

void AvrSpi::writeSpiReg( uint8_t newSPDR ) // SPDR is being written
{

}
