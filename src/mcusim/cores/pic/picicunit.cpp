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

void PicIcUnit::configureA( uint8_t val ) // CCPxM0,CCPxM1,CCPxM2,CCPxM3
{
    uint8_t mode = getRegBitsVal( val, m_configBitsA );
    if( mode == 0 )
    {
        m_enabled = false;
        m_icPin->changeCallBack( this, false );
    }

    if( (mode & 0b1100) != 0b0100 ) return; // No Capture Mode
    mode = mode & 0b11;
    m_enabled = true;
    m_icPin->changeCallBack( this, true );

    if( mode == m_mode ) return;
    initialize();
    m_mode =  mode;

    switch( mode ) {
        case 0: m_fallingEdge = true; break; // Falling Edge
        case 1:                       break; // Rising Edge
        case 2: m_prescaler = 4;      break; // Rising Edge, Presc = 4
        case 3: m_prescaler = 16;            // Rising Edge, Presc = 16
    }
}
