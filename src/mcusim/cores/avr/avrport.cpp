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

#include "avrport.h"
#include "avrpin.h"
#include "mcu.h"
#include "e_mcu.h"

AvrPort::AvrPort( eMcu* mcu, QString name )
       : McuPort( mcu, name )
{
}
AvrPort::~AvrPort(){}

void AvrPort::createPins( Mcu* mcuComp, QString pins, uint8_t pinMask )
{
    m_numPins = pins.toUInt(0,0);
    m_pins.resize( m_numPins );

    for( int i=0; i<m_numPins; ++i )
    {
        m_pins[i] = new AvrPin( this, i, m_name+QString::number(i), mcuComp );
    }
}
