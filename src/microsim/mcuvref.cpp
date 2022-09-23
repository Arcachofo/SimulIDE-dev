/***************************************************************************
 *   Copyright (C) 2021 by Santiago González                               *
 *                                                                         *
 ***( see copyright.txt file at root folder )*******************************/

#include "mcuvref.h"
#include "e_mcu.h"

McuVref::McuVref( eMcu* mcu, QString name )
       : McuModule( mcu, name )
       , eElement( mcu->getId()+"-"+name )
{
    m_pinOut = NULL;
}
McuVref::~McuVref(){}

void McuVref::initialize()
{
    m_enabled = false;
    m_mode = 0;
    m_vref = 0;
    //setMode( 0 );
}

/*void McuVref::setMode( uint8_t mode )
{
    m_mode = mode;
}*/

void McuVref::callBack( McuModule* mod, bool call ) // Add Modules to be called at Interrupt raise
{
    if( call )
    { if( !m_callBacks.contains( mod ) ) m_callBacks.append( mod ); }
    else m_callBacks.removeAll( mod );
}
