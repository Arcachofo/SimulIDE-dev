/***************************************************************************
 *   Copyright (C) 2020 by Santiago González                               *
 *                                                                         *
 ***( see copyright.txt file at root folder )*******************************/

#include "avrpin.h"
#include "iopin.h"

AvrPin::AvrPin( McuPort* port, int i, QString id, Component* mcu )
      : McuPin( port, i, id, mcu )
{
}
AvrPin::~AvrPin() {}

void AvrPin::setPortState( bool state )
{
    setPullup( state );
    McuPin::setPortState( state );
}
