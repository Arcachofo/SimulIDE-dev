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

#include "mcuport.h"
#include "mcupin.h"
#include "e_mcu.h"


McuPort::McuPort( eMcu* mcu )
       : eElement( "McuPort" )
{
    m_mcu = mcu;

    m_outReg = NULL;
    m_inReg  = NULL;
    m_dirReg = NULL;

    m_outAddr = 0;
    m_inAddr  = 0;
    m_dirAddr = 0;
    m_dirInv  = false;
}

McuPort::~McuPort()
{
    for( int i=0; i<m_numPins; ++i ) delete m_pins[i];
    m_pins.clear();
}

void McuPort::initialize()
{
    m_pinState = 0;
}

void McuPort::pinChanged( uint8_t pinMask, uint8_t val ) // Pin State is masked in val
{
    if( val ) m_pinState |= pinMask;
    else      m_pinState &= ~pinMask;

    m_mcu->writeReg( m_inAddr, m_pinState );
}

void McuPort::readInReg( uint8_t )
{
    *m_inReg = m_pinState;
}

void McuPort::outChanged( uint8_t val )
{
    uint8_t changed = *m_outReg ^ val;

    if( changed )
    {
        for( int i=0; i<m_numPins; ++i )
        {
            if( ( changed & (1<<i) )      // Pin changed
             && (!m_pins[i]->m_extCtrl )) // Port is controlling Pin
                m_pins[i]->setState( val & (1<<i) );
        }
    }
}

void McuPort::dirChanged( uint8_t val )
{
    uint8_t changed = *m_dirReg ^ val;
    if( m_dirInv ) val = ~val & 0xFF;

    if( changed )
    {
        for( int i=0; i<m_numPins; ++i )
        {
            if( (changed & 1<<i) )  // Pin changed
                m_pins[i]->setDirection( val & (1<<i));
        }
    }
}

void McuPort::setPullups(uint8_t puMask )
{
    //m_pullups = pu;

    for( int i=0; i<m_numPins; ++i )
    {
        bool pinPu = (puMask & 1<<i);// && !m_pins[i]->m_isOut;
        m_pins[i]->setPullup( pinPu );
    }
}

//  ------------------------------------------
McuPorts::McuPorts( eMcu* mcu )
{
    m_mcu = mcu;
}
McuPorts::~McuPorts()
{
}

void McuPorts::remove()
{
    for( McuPort* port : m_portList ) delete port;
    m_portList.clear();
}

McuPin* McuPorts::getPin( QString name )
{
    int pinNumber = name.right(1).toInt();
    QString portName = name.remove( name.size()-1, 1 );
    return getPort( portName )->m_pins[pinNumber];
}
