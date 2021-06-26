/***************************************************************************
 *   Copyright (C) 2021 by santiago González                               *
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

#include "mcuwdt.h"
#include "e_mcu.h"
#include "mcucore.h"
#include "simulator.h"

McuWdt::McuWdt( eMcu* mcu, QString name )
      : McuModule( mcu, name )
      , eElement( name )
{
}

McuWdt::~McuWdt()
{
}

void McuWdt::initialize()
{
    m_enabled  = false;
    m_ovfInter = false;
    m_ovfReset = false;
}

void McuWdt::runEvent()            // Overflow
{
    if( !m_enabled ) return;

    if( m_ovfInter ) interrupt.emitValue(1);
    if( m_ovfReset ) m_mcu->cpu->reset();

    Simulator::self()->addEvent( m_ovfPeriod, this );
}

