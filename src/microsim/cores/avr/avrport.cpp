/***************************************************************************
 *   Copyright (C) 2021 by Santiago González                               *
 *                                                                         *
 ***( see copyright.txt file at root folder )*******************************/

#include "avrport.h"
#include "avrpin.h"
#include "mcu.h"
#include "e_mcu.h"

AvrPort::AvrPort( eMcu* mcu, QString name )
       : McuPort( mcu, name )
{
}
AvrPort::~AvrPort(){}

McuPin* AvrPort::createPin( int i, QString id , Component* mcu )
{
    return new AvrPin( this, i, id, mcu );
}
