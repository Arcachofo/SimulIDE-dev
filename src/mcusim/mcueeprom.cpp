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

#include "mcueeprom.h"
#include "e_mcu.h"

McuEeprom::McuEeprom( eMcu* mcu, QString name )
         : McuModule( mcu, name )
         , eElement( name )
{
    m_addressL = NULL;
    m_addressH = NULL;
    m_dataReg  = NULL;
}

McuEeprom::~McuEeprom()
{
}

void McuEeprom::initialize()
{
    m_address = 0;
}

void McuEeprom::readEeprom()
{
    *m_dataReg = m_mcu->getRomValue( m_address );
}

void McuEeprom::writeEeprom()
{

}

void McuEeprom::addrWriteL( uint8_t val )
{
    m_address = val;
    if( m_addressH ) m_address += *m_addressH << 8;
}

void McuEeprom::addrWriteH( uint8_t val )
{
    m_address = (val << 8) + *m_addressL;
}

