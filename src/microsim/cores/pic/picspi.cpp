/***************************************************************************
 *   Copyright (C) 2021 by Santiago González                               *
 *                                                                         *
 ***( see copyright.txt file at root folder )*******************************/

#include "picspi.h"
#include "datautils.h"
#include "iopin.h"
#include "e_mcu.h"
#include "mcuinterrupts.h"

PicSpi::PicSpi( eMcu* mcu, QString name )
      : McuSpi( mcu, name )
{
    m_SSPEN = getRegBits( "SSPEN", mcu );
    m_CKP   = getRegBits( "CKP", mcu );
    m_CKE   = getRegBits( "CKE", mcu );
}
PicSpi::~PicSpi(){}

void PicSpi::setMode( spiMode_t mode )
{
    m_clockPeriod = 4*m_mcu->psCycle()*m_prescaler/2;

    if( mode == m_mode ) return;
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
        m_clkPin->controlPin( true, false );
        m_SS->setPinMode( input );
        m_SS->controlPin( true, false );
        m_MISO->controlPin( true, false );
    }
    SpiModule::setMode( mode );
}

void PicSpi::configureA( uint8_t newSSPCON ) // SSPCON is being written
{
    m_clkPol = getRegBitsBool( newSSPCON, m_CKP ); // Clock polarity
    m_sampleEdge = ( m_clkPol == m_clkPha ) ? Clock_Rising : Clock_Falling;
    m_leadEdge = m_clkPol ? Clock_Falling : Clock_Rising;
    m_tailEdge = m_clkPol ? Clock_Rising  : Clock_Falling;

    if( m_mode == SPI_MASTER ) m_clkPin->setOutState( m_clkPol );
}

void PicSpi::writeStatus( uint8_t newSSPSTAT ) // SSPSTAT is being written
{
    m_clkPha = getRegBitsBool( newSSPSTAT, m_CKE ); // Clock phase
    m_sampleEdge = ( m_clkPol == m_clkPha ) ? Clock_Rising : Clock_Falling;

}

void PicSpi::writeSpiReg( uint8_t newSSPBUF ) // SSPBUF is being written
{
    m_srReg = newSSPBUF;

    if( m_mode == SPI_MASTER ) StartTransaction();
}

void PicSpi::endTransaction()
{
    SpiModule::endTransaction();
    *m_dataReg = m_srReg;
    m_interrupt->raise();
}
