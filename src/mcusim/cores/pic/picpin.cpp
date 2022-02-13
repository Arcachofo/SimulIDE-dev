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

#include "picpin.h"
#include "iopin.h"
#include "mcuport.h"
#include "datautils.h"

PicPin::PicPin( McuPort* port, int i, QString id, Component* mcu )
      : McuPin( port, i, id, mcu )
{
    m_extIntTrigger = pinFalling;
    m_changeCB = true; // Always call VoltChanged()
}
PicPin::~PicPin() {}

void PicPin::ConfExtInt( uint8_t bits )
{
   m_extIntTrigger = getRegBitsVal( bits, m_extIntBits ) ? pinRising : pinFalling;
}

void PicPin::setAnalog( bool an )
{
    if( m_isAnalog == an ) return;
    m_isAnalog = an;

    if( an ) // Disable Digital input
    {
        changeCallBack( this, false );
        m_port->pinChanged( m_pinMask, 0 );
    }
    else     // Enable Digital input if is input
    {
        changeCallBack( this, true ); // Receive voltage change notifications
        voltChanged(); // Update input state
        m_port->pinChanged( m_pinMask, m_inpState );
    }
}
