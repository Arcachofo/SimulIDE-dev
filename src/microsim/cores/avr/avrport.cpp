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

void AvrPort::pinRegChanged( uint8_t newPIN )
{
    if( newPIN == 0 ) return;
    McuPort::outChanged( *m_outReg ^ newPIN ); // Toggle bits = 1
    m_mcu->m_regOverride = *m_inReg;
}

McuPin* AvrPort::createPin( int i, QString id , Component* mcu )
{
    return new AvrPin( this, i, id, mcu );
}
