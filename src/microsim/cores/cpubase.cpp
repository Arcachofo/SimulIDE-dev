/***************************************************************************
 *   Copyright (C) 2020 by santiago González                               *
 *   santigoro@gmail.com                                                   *
 *                                                                         *
 *   This program is free software; you can redistribute it and/or modify  *
 *   it under the terms of the GNU General Public License as published by  *
 *   the Free Software Foundation; either version 3 of the License, or     *
 *   (at your option) any later version.                                   *
 *                                                                         *
 *   This program is distributed in the hope that it will be useful,       *
 *   but WITHOUT ANY WARRANTY; without even the implied warranty of        *
 *   MERCHANTABILITY or FITNESS FOR A PARTICULAR PURPOSE.  See the         *
 *   GNU General Public License for more details.                          *
 *                                                                         *
 *   You should have received a copy of the GNU General Public License     *
 *   along with this program; if not, see <http://www.gnu.org/licenses/>.  *
 *                                                                         *
 ***************************************************************************/

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
