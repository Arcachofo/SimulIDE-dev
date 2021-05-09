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

#include "avrtwi.h"
#include "mcupin.h"
#include "e_mcu.h"

AvrTwi::AvrTwi( eMcu* mcu, QString name )
      : McuTwi( mcu, name )
{
}

AvrTwi::~AvrTwi()
{
}

void AvrTwi::initialize()
{
}

void AvrTwi::configureA( uint8_t val ) // TWCR
{
    bool enable =  (( val & 0b00000100 )>0);
}

void AvrTwi::configureB( uint8_t val )
{
}

void AvrTwi::writeStatus( uint8_t val )
{
    val &= 0b00000011;
    m_prescaler = val;

    m_mcu->m_regOverride = val | (*m_twiStatus & 0b11111100); // Preserve Status bits
}
