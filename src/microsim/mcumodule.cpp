/***************************************************************************
 *   Copyright (C) 2021 by Santiago González                               *
 *                                                                         *
 ***( see copyright.txt file at root folder )*******************************/

#include "mcumodule.h"
#include "e_mcu.h"
#include "mcuinterrupts.h"

McuModule::McuModule( eMcu* mcu, QString name )
{
    m_mcu = mcu;
    m_name = name;
    m_sleepMode = 0;
    m_interrupt = NULL;
}
McuModule::~McuModule( ){}

void McuModule::sleep( int mode )
{
    if( mode < 0 ) m_sleeping = false;
    else           m_sleeping = (m_sleepMode & 1<<mode) > 0;
}

/*void McuModule::reset()
{
    Simulator::self()->cancelEvents( this );
}*/

/*void McuModule::raiseInt()
{
    if( m_interrupt ) m_interrupt->raise();
}*/
