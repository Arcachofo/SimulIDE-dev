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

#include "i51interrupt.h"
#include "i51extint.h"

I51Interrupt::I51Interrupt( QString name, uint16_t vector, eMcu* mcu )
            : Interrupt( name, vector, mcu )
{
}
I51Interrupt::~I51Interrupt(){}

// Static --------------------------
Interrupt* I51Interrupt::getInterrupt( QString name, uint16_t vector, eMcu* mcu )
{
    if( name.startsWith( "EXT") ) return new I51ExtInt( name, vector, mcu );

    return new Interrupt( name, vector, mcu );
}

