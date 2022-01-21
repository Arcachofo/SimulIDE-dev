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

#include "mcuicunit.h"
#include "mcutimer.h"
#include "mcuinterrupts.h"
#include "mcupin.h"

McuIcUnit::McuIcUnit( eMcu* mcu, QString name )
         : McuModule( mcu, name )
         , eElement( name )
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

    uint16_t count = m_timer->getCount();
    *m_icRegL = count & 0xFF;
    *m_icRegH = (count >> 8) & 0xFF;

    m_interrupt->raise();
}

void McuIcUnit::enable( bool en )
{
    m_enabled = en;
    m_icPin->changeCallBack( this, en );
}
