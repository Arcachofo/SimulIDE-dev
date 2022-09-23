/***************************************************************************
 *   Copyright (C) 2020 by Santiago González                               *
 *                                                                         *
 ***( see copyright.txt file at root folder )*******************************/

#include "cpubase.h"

CpuBase::CpuBase( eMcu* mcu )
{
    m_mcu = mcu;

    m_retCycles = 2;

    m_spl = NULL;
    m_sph = NULL;
    m_STATUS = NULL;
}
CpuBase::~CpuBase() {}

void CpuBase::reset()
{
    m_PC = 0;
}

int CpuBase::getCpuReg( QString reg )
{
    if( m_cpuRegs.contains( reg ) )
    {
        uint8_t* regPtr = m_cpuRegs.value( reg );
        return *regPtr;
    }
    return -1;
}
