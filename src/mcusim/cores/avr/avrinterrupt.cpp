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
#include "datautils.h"

Interrupt* AVRInterrupt::getInterrupt( QString name, uint16_t vector, eMcu* mcu ) // Static
{
    return new AVRInterrupt( name, vector, mcu );
}

AVRInterrupt::AVRInterrupt( QString name, uint16_t vector, eMcu* mcu )
            : Interrupt( name, vector, mcu )
{
    m_I = getRegBits( "I", mcu );

    m_autoClear = true;
    m_remember  = true;
}
AVRInterrupt::~AVRInterrupt(){}

void AVRInterrupt::execute()
{
    clearRegBits( m_I );             // Clear SREG I flag
    m_interrupts->enableGlobal( 0 ); // Disable interrupts
    Interrupt::execute();
}

void AVRInterrupt::exitInt() // Exit from this interrupt
{
    setRegBits( m_I );                // Set SREG I flag
    m_interrupts->enableGlobal( 2 );  // Enable interrupts (execute next cycle)
    Interrupt::exitInt();
}

