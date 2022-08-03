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

#include "picport.h"
#include "picpin.h"
#include "mcu.h"
#include "e_mcu.h"
#include "datautils.h"

PicPort::PicPort( eMcu* mcu, QString name )
       : McuPort( mcu, name )
{
}
PicPort::~PicPort(){}

void PicPort::configureA( uint8_t newANSEL ) // Analog pins
{
    for( int i=0; i<m_numPins; ++i )
        m_pins[i]->setAnalog( newANSEL & 1<<i );
}

McuPin* PicPort::createPin( int i, QString id , Component* mcu )
{
    return new PicPin( this, i, id, mcu );
}
