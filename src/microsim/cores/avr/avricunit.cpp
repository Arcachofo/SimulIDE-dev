/***************************************************************************
 *   Copyright (C) 2021 by Santiago González                               *
 *                                                                         *
 ***( see copyright.txt file at root folder )*******************************/

#include "avricunit.h"
#include "datautils.h"
#include "mcupin.h"
#include "mcutimer.h"
#include "mcuinterrupts.h"

AvrIcUnit::AvrIcUnit( eMcu* mcu, QString name )
         : McuIcUnit( mcu, name )
{
}
AvrIcUnit::~AvrIcUnit( ){}

void AvrIcUnit::configureA( uint8_t val ) // ICES,ICNC
{
    m_fallingEdge = (val & 1) == 0;
    /// TODO INCN
}
