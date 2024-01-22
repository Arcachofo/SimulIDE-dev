/***************************************************************************
 *   Copyright (C) 2020 by Santiago González                               *
 *                                                                         *
 ***( see copyright.txt file at root folder )*******************************/

#include "avrpin.h"
#include "mcuinterrupts.h"

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

void AvrPin::ConfExtInt( uint8_t bits )
{
    McuPin::ConfExtInt( bits );
    m_extInt->setContinuous( m_extIntTrigger == pinLow );
    voltChanged();
}
