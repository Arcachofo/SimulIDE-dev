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

#include "i51interrupt.h"
#include "i51usart.h"

I51Interrupt::I51Interrupt( QString name, uint16_t vector, eMcu* mcu )
            : Interrupt( name, vector, mcu )
{
}
I51Interrupt::~I51Interrupt(){}

// Static --------------------------
Interrupt* I51Interrupt::getInterrupt( QString name, uint16_t vector, eMcu* mcu )
{
    if( name.startsWith( "EXT") ) return new I51ExtInt( name, vector, mcu );

    return new Interrupt( name, vector, mcu );
}

//_________________________________________________________________
//_________________________________________________________________

I51ExtInt::I51ExtInt( QString name, uint16_t vector, eMcu* mcu )
         : Interrupt( name, vector, mcu )
{}
I51ExtInt::~I51ExtInt(){}

void I51ExtInt::reset()
{
    m_lastValue = 0;
    Interrupt::reset();
}

void I51ExtInt::raise( uint8_t v )
{
    if( !m_enable ) return;

    if( m_mode == 0 )
    {
        if( v == 0 ) Interrupt::raise( v ); // Low  state: raise
        else         m_raised = false;      // High state: stop triggering
    }
    else if( m_mode == 1 )
    {
        if( (v==0) && m_lastValue ) Interrupt::raise( v ); // Falling edge
    }
    m_lastValue = v;
}

void I51ExtInt::exitInt() // Exit from this interrupt
{
    Interrupt::exitInt();

    if( (m_mode == 0) && m_raised ) //In mode 0 keep triggering until pin change state
        m_interrupts->addToPending( m_priority, this ); // Add to pending interrupts

    else if( m_mode == 1 ) Interrupt::clearFlag();
}

//_________________________________________________________________
//_________________________________________________________________

I51T1Int::I51T1Int( QString name, uint16_t vector, eMcu* mcu )
         : Interrupt( name, vector, mcu )
{}
I51T1Int::~I51T1Int(){}

void I51T1Int::reset()
{
    m_usart = NULL;
    Interrupt::reset();
}

void I51T1Int::raise( uint8_t v )
{
    if( !m_enable ) return;

    if( m_usart ) m_usart->step();
    Interrupt::raise( v ); // Low  state: raise
}
