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

#include "i51port.h"
#include "i51pin.h"
#include "mcu.h"
#include "e_mcu.h"

I51Port::I51Port( eMcu* mcu, QString name )
       : McuPort( mcu, name )
{
}
I51Port::~I51Port(){}

void I51Port::reset()
{
    m_pinState = 255;
    //if( m_rstIntMask ) m_intMask = 0;
    /// for( McuPin* pin : m_pins ) pin->reset();
}

McuPin* I51Port::createPin( int i, QString id , Component* mcu )
{
    return new I51Pin( this, i, id, mcu );
}
