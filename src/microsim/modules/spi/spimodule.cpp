/***************************************************************************
 *   Copyright (C) 2021 by Santiago González                               *
 *                                                                         *
 ***( see copyright.txt file at root folder )*******************************/

#include "spimodule.h"
#include "iopin.h"
#include "simulator.h"

SpiModule::SpiModule( QString name )
         : eClockedDevice( name )
{
    m_MOSI = NULL;
    m_MISO = NULL;
    m_SS   = NULL;

    m_dataOutPin = NULL;
    m_dataInPin  = NULL;
}
SpiModule::~SpiModule( ){}

void SpiModule::initialize()
{
    m_mode = SPI_OFF;

    m_srReg = 0;

    m_toggleSck = false;
    m_lsbFirst  = false;
    m_enabled   = false;
    m_useSS     = true;

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
        if( m_bitCount == 8 ) endTransaction();
        else                  step();
    }
}

void SpiModule::voltChanged() // Called in Slave mode on SCK or SS changes
{
    if( m_mode != SPI_SLAVE ) return;

    updateClock();

    bool enabled = true;
    if( m_useSS && m_SS ) enabled = !m_SS->getInpState(); // SS active LOW

    if( enabled != m_enabled ) // Enabling or Disabling
    {
        if( enabled && !m_enabled ) // Enabling
        {
            m_clkPin->changeCallBack( this, true );
            StartTransaction(); // Start transaction
        }
        else                        // Disabling
        {
            m_clkPin->changeCallBack( this, false );
            m_srReg = 0;// Reset SPI Logic
        }
        m_enabled = enabled;
    }
    if( !enabled ) return;
    if( m_clkState == Clock_High || m_clkState == Clock_Low ) return; // Not an Edge

    step();
}

void SpiModule::endTransaction()
{
    if( m_dataOutPin && m_mode == SPI_MASTER ) m_dataOutPin->setOutState( true );
}

void SpiModule::StartTransaction()
{
    resetSR();
    Simulator::self()->cancelEvents( this );
    if( m_sampleEdge == m_leadEdge ) // Sample in first Leading Edge => setup now
    {
        //m_clkState = m_tailEdge; // Force setup
        step();
    }
    else if( m_mode == SPI_MASTER ) keepClocking();
}

void SpiModule::resetSR()
{
    m_bitCount = 0;
    if( m_lsbFirst )
    {
        m_outBit = 1;
        m_inBit  = 1<<7;
    }else{
        m_outBit = 1<<7;
        m_inBit  = 1;
    }
}

void SpiModule::step()
{
    if( m_mode == SPI_MASTER ) keepClocking();

    if( m_clkState == m_sampleEdge )         //Read one bit
    {
        m_bitCount++;

        if( m_lsbFirst ) m_srReg >>= 1; // Rotate bit mask
        else             m_srReg <<= 1;

        if( m_dataInPin->getInpState() ) m_srReg |= m_inBit;
    }
    else if( m_dataOutPin )                 // Write one bit (Only if dataOut Pin exist)
    {
        if( m_bitCount == 8 ) endTransaction();
        else                  m_dataOutPin->sheduleState( (m_srReg & m_outBit)>0, 0 );
    }
}

void SpiModule::setMode( spiMode_t mode )
{
    if( mode == m_mode ) return;
    m_mode = mode;

    m_dataOutPin = NULL;
    m_dataInPin  = NULL;

    if( mode == SPI_OFF )
    {
        if( m_MOSI )   m_MOSI->changeCallBack( this, false );
        if( m_MISO )   m_MISO->changeCallBack( this, false );
        if( m_clkPin ) m_clkPin->changeCallBack( this, false );
        if( m_SS )     m_SS->changeCallBack( this, false );
    }
    else if( mode == SPI_MASTER )
    {
        if( !m_MOSI || !m_MISO || !m_clkPin ) { m_mode = SPI_OFF; return; }
        m_dataOutPin = m_MOSI;
        m_dataInPin  = m_MISO;
    }
    else if( mode == SPI_SLAVE )
    {
        if( !m_MOSI || !m_clkPin ) { m_mode = SPI_OFF; return; }
        m_dataOutPin = m_MISO;
        m_dataInPin  = m_MOSI;

        if( m_useSS && m_SS ) m_SS->changeCallBack( this, true );
    }
    if( m_dataOutPin && m_mode == SPI_MASTER ) m_dataOutPin->setOutState( true );
}

