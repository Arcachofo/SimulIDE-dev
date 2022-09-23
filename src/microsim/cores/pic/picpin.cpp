/***************************************************************************
 *   Copyright (C) 2020 by Santiago González                               *
 *                                                                         *
 ***( see copyright.txt file at root folder )*******************************/

#include "picpin.h"
#include "iopin.h"
#include "mcuport.h"
#include "datautils.h"

PicPin::PicPin( McuPort* port, int i, QString id, Component* mcu )
      : McuPin( port, i, id, mcu )
{
    m_extIntTrigger = pinFalling;
}
PicPin::~PicPin() {}

void PicPin::ConfExtInt( uint8_t bits )
{
   m_extIntTrigger = getRegBitsVal( bits, m_extIntBits ) ? pinRising : pinFalling;
}

void PicPin::setAnalog( bool an )
{
    if( m_isAnalog == an ) return;
    m_isAnalog = an;

    if( an ) // Disable Digital input
    {
        changeCallBack( this, false );
        m_port->pinChanged( m_pinMask, 0 );
        m_inpState = 0;
    }
    else     // Enable Digital input if is input
    {
        changeCallBack( this, true ); // Receive voltage change notifications
        voltChanged(); // Update input state
    }
}
