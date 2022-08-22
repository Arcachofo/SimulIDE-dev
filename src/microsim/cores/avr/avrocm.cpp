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

#include "avrocm.h"
#include "mcupin.h"
#include "e_mcu.h"

AvrOcm::AvrOcm( eMcu* mcu, QString name )
      : McuOcm( mcu, name )
{
}
AvrOcm::~AvrOcm(){}

void AvrOcm::configureA( uint8_t newVal )
{
    m_mode = newVal;
}

void AvrOcm::OutputOcm() //Set Ocm output from OCnB1 & OCnB2
{
    if( m_mode ) m_oPin->sheduleState( m_state1 || m_state2, 0 );
    else         m_oPin->sheduleState( m_state1 && m_state2, 0 );
}
