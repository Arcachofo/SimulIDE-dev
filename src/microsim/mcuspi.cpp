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
}

McuSpi::~McuSpi()
{
}

/*void McuSpi::initialize()
{
    SpiModule::initialize();

}

void McuSpi::setMode( spiMode_t mode )
{
    SpiModule::setMode(  mode );
}*/
