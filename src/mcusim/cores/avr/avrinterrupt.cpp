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

#include "avrinterrupt.h"
#include "e_mcu.h"

AVRInterrupt::AVRInterrupt( QString name, uint16_t vector, eMcu* mcu )
            : Interrupt( name, vector, mcu )
{
    //m_SREG = mcu->getReg( "SREG" );

    m_I = mcu->getRegBits( "I" );
}
AVRInterrupt::~AVRInterrupt(){}

void AVRInterrupt::raise( uint8_t v )
{
    //if( m_name == "T1_OVF" )
       // m_name = "T1_OVF";
    //clearRegBits( m_I );// Deactivate Interrupts: SREG.I = 0
    Interrupt::raise( v );
}

void AVRInterrupt::exitInt() // Exit from this interrupt
{
    //setRegBits( m_I );// Activate Interrupts: SREG.I = 1
    Interrupt::exitInt();
}



// Static --------------------------

Interrupt* AVRInterrupt::getInterrupt( QString name, uint16_t vector, eMcu* mcu )
{
    return new AVRInterrupt( name, vector, mcu );
}

