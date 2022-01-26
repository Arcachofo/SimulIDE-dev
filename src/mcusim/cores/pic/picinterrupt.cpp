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

#include "picinterrupt.h"
#include "e_mcu.h"
#include "datautils.h"

Interrupt* PicInterrupt::getInterrupt( QString name, uint16_t vector, eMcu* mcu ) // Static
{
    return new PicInterrupt( name, vector, mcu );
}

PicInterrupt::PicInterrupt( QString name, uint16_t vector, eMcu* mcu )
            : Interrupt( name, vector, mcu )
{
    m_GIE = getRegBits( "GIE", mcu );

    m_autoClear = false;
    m_remember  = true;
}
PicInterrupt::~PicInterrupt(){}

void PicInterrupt::execute()
{
    clearRegBits( m_GIE );
    m_interrupts->enableGlobal( 0 ); // Disable interrupts
    Interrupt::execute();
}

void PicInterrupt::exitInt() // Exit from this interrupt
{
    setRegBits( m_GIE );
    m_interrupts->enableGlobal( 1 );  // Enable interrupts (execute next cycle)
    Interrupt::exitInt();
}

