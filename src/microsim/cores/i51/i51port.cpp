/***************************************************************************
 *   Copyright (C) 2021 by Santiago González                               *
 *                                                                         *
 ***( see copyright.txt file at root folder )*******************************/

#include "i51port.h"
#include "i51pin.h"
#include "mcu.h"
#include "e_mcu.h"

I51Port::I51Port( eMcu* mcu, QString name )
       : McuPort( mcu, name )
{
}
I51Port::~I51Port(){}

void I51Port::reset()
{
    m_pinState = 255;
    //if( m_rstIntMask ) m_intMask = 0;
    /// for( McuPin* pin : m_pins ) pin->reset();
}

void I51Port::readPort( uint8_t )
{
    m_mcu->m_regOverride = m_pinState;
}

McuPin* I51Port::createPin( int i, QString id , Component* mcu )
{
    return new I51Pin( this, i, id, mcu );
}

