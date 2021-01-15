/***************************************************************************
 *   Copyright (C) 2018 by santiago González                               *
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

#include "e-bcdto7s.h"
#include "simulator.h"

eBcdTo7S::eBcdTo7S( QString id )
        : eLogicDevice( id )
{
    m_values.resize( 17 );

    m_values[0]  = 0b00111111;
    m_values[1]  = 0b00000110;
    m_values[2]  = 0b01011011;
    m_values[3]  = 0b01001111;
    m_values[4]  = 0b01100110;
    m_values[5]  = 0b01101101;
    m_values[6]  = 0b01111101;
    m_values[7]  = 0b00000111;
    m_values[8]  = 0b01111111;
    m_values[9]  = 0b01101111;
    m_values[10] = 0b01110111;
    m_values[11] = 0b01111100;
    m_values[12] = 0b00111001;
    m_values[13] = 0b01011110;
    m_values[14] = 0b01111001;
    m_values[15] = 0b01110001;
    m_values[16] = 0b00000000;
}
eBcdTo7S::~eBcdTo7S() {}

void eBcdTo7S::stamp()
{
    for( int i=0; i<4; i++ )
    {
        eNode* enode = m_input[i]->getEpin(0)->getEnode();
        if( enode ) enode->voltChangedCallback( this );
    }
    m_changed = true;
    m_digit = -1;

    eLogicDevice::stamp();
}

void eBcdTo7S::voltChanged()
{
    eLogicDevice::updateOutEnabled();
    
    m_changed = true;
    
    bool a = getInputState( 0 );
    bool b = getInputState( 1 );
    bool c = getInputState( 2 );
    bool d = getInputState( 3 );

    int digit = a*1+b*2+c*4+d*8;
    if( digit == m_digit ) return;
    m_digit = digit;

    if( m_numOutputs ) Simulator::self()->addEvent( m_propDelay, this );
}

void eBcdTo7S::runEvent()
{
    uint8_t value = m_values[m_digit];
    for( int i=0; i<m_numOutputs; ++i ) setOut( i, value & (1<<i) );
}
