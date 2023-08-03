/***************************************************************************
 *   Copyright (C) 2023 by Santiago González                               *
 *                                                                         *
 ***( see copyright.txt file at root folder )*******************************/

#include "corebase.h"

CoreBase::CoreBase()
{
    m_display = NULL;
}
CoreBase::~CoreBase() {}

int CoreBase::getCpuReg( QString reg )
{
    if( m_cpuRegs.contains( reg ) )
    {
        uint8_t* regPtr = m_cpuRegs.value( reg );
        return *regPtr;
    }
    return -1;
}
