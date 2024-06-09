/***************************************************************************
 *   Copyright (C) 2021 by Santiago González                               *
 *                                                                         *
 ***( see copyright.txt file at root folder )*******************************/

#include "mcuspi.h"
#include "iopin.h"
#include "e_mcu.h"

McuSpi::McuSpi( eMcu* mcu, QString name )
      : McuPrescaled( mcu, name )
      , SpiModule( mcu->getId()+"-"+name )
{
    m_dataReg = NULL;
    m_statReg = NULL;
}

McuSpi::~McuSpi()
{
}

void McuSpi::initialize()
{
    SpiModule::initialize();
    if( m_statReg ) *m_statReg = 0;
}

/*void McuSpi::setMode( spiMode_t mode )
{
    SpiModule::setMode(  mode );
}*/
