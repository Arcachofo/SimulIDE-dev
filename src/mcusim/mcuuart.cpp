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

#include "mcuuart.h"

QHash<QString, UsartM*> McuUsart::m_usarts;

McuUsart::McuUsart(eMcu* mcu, QString name )
        : McuModule( mcu, name )
        , UsartM( name )
{
    m_mode = 0xFF; // Force first mode change.
}
McuUsart::~McuUsart( ){}

void McuUsart::dataAvailable( uint8_t data )
{
    *m_rxRegister = data; // Save data to Ram
}

