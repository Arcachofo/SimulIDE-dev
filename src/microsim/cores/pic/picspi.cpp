/***************************************************************************
 *   Copyright (C) 2021 by Santiago González                               *
 *                                                                         *
 ***( see copyright.txt file at root folder )*******************************/

#include "picspi.h"
#include "datautils.h"
//#include "mcupin.h"
#include "iopin.h"
#include "e_mcu.h"
#include "mcuinterrupts.h"

PicSpi::PicSpi( eMcu* mcu, QString name )
      : McuSpi( mcu, name )
{
    //m_SPCR = mcu->getReg( "SPCR" );
    //m_SPIF = getRegBits( "SPIF", mcu );
}
PicSpi::~PicSpi(){}

/*void PicSpi::initialize()
{
    McuSpi::initialize();
}*/

void PicSpi::setMode( spiMode_t mode )
{
    if( mode == m_mode ) return;
    m_mode = mode;

    bool ctrl = mode > SPI_OFF;

    m_MOSI->controlPin( ctrl, false );
    m_MISO->controlPin( ctrl, false );
    m_clkPin->controlPin( ctrl, false );
    m_SS->controlPin( mode == SPI_SLAVE, false );

    SpiModule::setMode( mode );
}

void PicSpi::configureA( uint8_t newSPCR ) // SPCR is being written
{
}

void PicSpi::writeStatus( uint8_t newSPSR ) // SPSR is being written
{
}

void PicSpi::writeSpiReg( uint8_t newSPDR ) // SPDR is being written
{
}

void PicSpi::endTransaction()
{
    SpiModule::endTransaction();
    //m_interrupt->raise();
}
