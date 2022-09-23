/***************************************************************************
 *   Copyright (C) 2020 by Santiago González                               *
 *                                                                         *
 ***( see copyright.txt file at root folder )*******************************/

#include "pic14core.h"
#include "datautils.h"
#include "regwatcher.h"

Pic14Core::Pic14Core( eMcu* mcu )
         : PicMrCore( mcu )
{
    m_stackSize = 8;

    m_Wreg = &m_WregHidden;

    m_FSR    = m_mcu->getReg( "FSR" );
    m_OPTION = m_mcu->getReg( "OPTION" );

    m_bankBits = getRegBits( "R0,R1", mcu );
    watchBitNames( "R0,R1", R_WRITE, this, &Pic14Core::setBank, mcu );
}
Pic14Core::~Pic14Core() {}

