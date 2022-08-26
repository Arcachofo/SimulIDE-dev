/***************************************************************************
 *   Copyright (C) 2022 by santiago González                               *
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

#include "mcucpu.h"

McuCpu::McuCpu( eMcu* mcu )
      : CpuBase( mcu )
{
    m_dataMem    = mcu->getRam();
    m_dataMemEnd = mcu->ramSize();
    if( m_dataMemEnd ) m_dataMemEnd--;
    m_progMem    = mcu->m_progMem.data();
    m_progSize   = mcu->flashSize();

    if( mcu->m_regStart > 0 ) m_lowDataMemEnd = mcu->m_regStart-1;
    else                      m_lowDataMemEnd = 0;

    m_regEnd = mcu->m_regEnd;
    if( m_dataMemEnd > 0 ) m_STATUS = &m_dataMem[mcu->m_sregAddr];

    if     ( m_progSize <= 0xFF )     m_progAddrSize = 1;
    else if( m_progSize <= 0xFFFF )   m_progAddrSize = 2;
    else if( m_progSize <= 0xFFFFFF ) m_progAddrSize = 3;
}
McuCpu::~McuCpu() {}

void McuCpu::CALL_ADDR( uint32_t addr ) // Used by MCU Interrupts:: All MCUs should use or override this
{
    PUSH_STACK( m_PC );
    setPC( addr );
    m_mcu->cyclesDone = m_retCycles;
}
