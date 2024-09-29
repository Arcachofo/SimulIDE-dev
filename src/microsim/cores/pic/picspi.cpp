/***************************************************************************
 *   Copyright (C) 2021 by Santiago González                               *
 *                                                                         *
 ***( see copyright.txt file at root folder )*******************************/

#include "picspi.h"
#include "datautils.h"
#include "iopin.h"
#include "e_mcu.h"
#include "mcuinterrupts.h"
#include "simulator.h"

PicSpi::PicSpi( eMcu* mcu, QString name )
      : McuSpi( mcu, name )
{
}
PicSpi::~PicSpi(){}

void PicSpi::setup()
{
    m_sleepMode = 0xFF;

    m_SSPEN = getRegBits("SSPEN", m_mcu );
    m_CKP   = getRegBits("CKP", m_mcu );
    m_CKE   = getRegBits("CKE", m_mcu );
}

void PicSpi::setMode( spiMode_t mode )
{
    m_clockPeriod = 4*m_mcu->psInst()*m_prescaler/2;

    if( mode == m_mode ) return;
    m_mode = mode;

    m_dataOutPin = m_MOSI;
    m_dataInPin  = m_MISO;
    Simulator::self()->cancelEvents( this );

    if( mode == SPI_OFF )
    {
        m_MOSI->controlPin( false, false );
        m_MISO->controlPin( false, false );
        m_clkPin->controlPin( false, false );
        m_SS->controlPin( false, false );
        return;
    }
    m_dataOutPin->controlPin( true, false );
    m_dataInPin->setPinMode( input );
    m_dataInPin->controlPin( true, true );
    m_SS->controlPin( true, false );

    if( mode == SPI_MASTER )
    {
        m_clkPin->controlPin( true, false );
        m_dataOutPin->setOutState( true );
    }
    else if( mode == SPI_SLAVE )
    {
        m_clkPin->controlPin( false, false );
        m_clkPin->changeCallBack( this, true );
        if( m_useSS && m_SS ) m_SS->changeCallBack( this, true );
    }
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

void PicSpi::sleep( int mode )
{
    McuModule::sleep( mode );
    if( m_sleeping ) pauseEvents();
    else             resumeEvents();
}

