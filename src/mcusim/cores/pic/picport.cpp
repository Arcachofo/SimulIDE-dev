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

PicPort::PicPort( eMcu* mcu, QString name, uint8_t numPins )
       : McuPort( mcu, name, numPins )
{
}
PicPort::~PicPort(){}

void PicPort::createPins( Mcu* mcuComp, uint8_t pinMask )
{
    m_pins.resize( m_numPins );

    if( pinMask )
    {
        for( int i=0; i<m_numPins; ++i )
        {
            if( pinMask & 1<<i )
                m_pins[i] = new PicPin( this, i, m_name+QString::number(i), mcuComp );
        }
    }else{
        for( int i=0; i<m_numPins; ++i )
            m_pins[i] = new PicPin( this, i, m_name+QString::number(i), mcuComp );
    }
}

