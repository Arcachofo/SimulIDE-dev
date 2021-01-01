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

#include "mcucore.h"

McuCore::McuCore( eMcu* mcu )
{
    m_mcu = mcu;

    m_dataMem       = mcu->m_dataMem.data();
    m_dataMemEnd    = mcu->ramSize()-1;
    m_progMem   = mcu->m_progMem.data();
    m_progSize  = mcu->flashSize();

    m_lowDataMemEnd = mcu->m_regStart-1;
    m_regEnd    = mcu->m_regEnd;
    sreg        = mcu->m_sreg.data();

    if     ( m_progSize <= 0xFF ) m_progAddrSize = 1;
    else if( m_progSize <= 0xFFFF ) m_progAddrSize = 2;
    else if( m_progSize <= 0xFFFFFF ) m_progAddrSize = 3;
}
McuCore::~McuCore() {}


