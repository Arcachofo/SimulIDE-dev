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
#include "e-source.h"

const uint8_t eBcdTo7S::m_values[]={
        0b00111111,
        0b00000110,
        0b01011011,
        0b01001111,
        0b01100110,
        0b01101101,
        0b01111101,
        0b00000111,
        0b01111111,
        0b01101111,
        0b01110111,
        0b01111100,
        0b00111001,
        0b01011110,
        0b01111001,
        0b01110001,
        0b00000000
};

eBcdTo7S::eBcdTo7S( QString id )
        : eLogicDevice( id )
{

}
eBcdTo7S::~eBcdTo7S() {}

void eBcdTo7S::stamp()
{
    for( int i=0; i<4; ++i )
    {
        eNode* enode = m_input[i]->getEpin(0)->getEnode();
        if( enode ) enode->voltChangedCallback( this );
    }
    m_nextOutVal = m_values[0];
    m_changed = true;

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
    m_nextOutVal = m_values[digit];

    if( m_numOutputs ) sheduleOutPuts();
}
