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

#include "picicunit.h"
#include "datautils.h"
#include "mcupin.h"

PicIcUnit::PicIcUnit( eMcu* mcu, QString name )
         : McuIcUnit( mcu, name )
{
}
PicIcUnit::~PicIcUnit( ){}

void PicIcUnit::initialize()
{
    McuIcUnit::initialize();
    m_prescaler = 1;
    m_counter = 0;
}

void PicIcUnit::voltChanged() // Pin change
{
    m_counter++;
    if( m_counter < m_prescaler ) return;
    m_counter = 0;

    McuIcUnit::voltChanged();
}

void PicIcUnit::configure( uint8_t CCPxM ) // CCPxM0,CCPxM1,CCPxM2,CCPxM3
{
    m_enabled = true;
    m_icPin->changeCallBack( this, true );

    switch( CCPxM ) {
        case 0: m_fallingEdge = true; break; // Falling Edge
        case 1:                       break; // Rising Edge
        case 2: m_prescaler = 4;      break; // Rising Edge, Presc = 4
        case 3: m_prescaler = 16;            // Rising Edge, Presc = 16
    }
}
