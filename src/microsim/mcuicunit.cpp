/***************************************************************************
 *   Copyright (C) 2021 by Santiago González                               *
 *                                                                         *
 ***( see copyright.txt file at root folder )*******************************/

#include "mcuicunit.h"
#include "mcutimer.h"
#include "mcuinterrupts.h"
#include "mcupin.h"
#include "e_mcu.h"

McuIcUnit::McuIcUnit( eMcu* mcu, QString name )
         : McuModule( mcu, name )
         , eElement( mcu->getId()+"-"+name )
{
    m_timer  = NULL;
    m_icPin  = NULL;
    m_icRegL = NULL;
    m_icRegH = NULL;
}
McuIcUnit::~McuIcUnit( ){}

void McuIcUnit::initialize()
{
    m_enabled = false;
    m_inState = false;
    m_fallingEdge = false;
    m_mode = 0;
    if( m_icPin ) m_icPin->changeCallBack( this, false );
}

void McuIcUnit::voltChanged() // Pin change
{
    bool inState = m_icPin->getInpState();

    if( m_inState == inState ) return;  // No Edge
    m_inState = inState;
    if( inState == m_fallingEdge ) return; // Wrong Edge

    if( m_icRegL )
    {
        uint16_t count = m_timer->getCount();
        *m_icRegL = count & 0xFF;
        if( m_icRegH ) *m_icRegH = (count >> 8) & 0xFF;
    }
    m_interrupt->raise();
}

void McuIcUnit::enable( bool en )
{
    m_enabled = en;
    if( m_icPin ) m_icPin->changeCallBack( this, en );
}
