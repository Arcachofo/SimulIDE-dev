/***************************************************************************
 *   Copyright (C) 2020 by Santiago González                               *
 *                                                                         *
 ***( see copyright.txt file at root folder )*******************************/

#include "cpubase.h"

CpuBase::CpuBase( eMcu* mcu )
       : CoreBase()
{
    m_mcu = mcu;

    m_retCycles = 2;

    m_spl = nullptr;
    m_sph = nullptr;
    m_STATUS = nullptr;
}
CpuBase::~CpuBase() {}

void CpuBase::reset()
{
    m_PC = 0;
}
