/***************************************************************************
 *   Copyright (C) 2012 by santiago González                               *
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

#include "e-dectobcd.h"
#include "simulator.h"

eDecToBcd::eDecToBcd( QString id )
         : eLogicDevice( id )
{
    m_16Bits = false;
    m_bits = 10;
}
eDecToBcd::~eDecToBcd() {}

void eDecToBcd::stamp()
{
    for( int i=0; i<15; ++i )
    {
        eNode* enode = m_input[i]->getEpin(0)->getEnode();
        if( enode ) enode->voltChangedCallback( this );
    }
    m_bcd = -1;

    eLogicDevice::stamp();
}

void eDecToBcd::voltChanged()
{
    eLogicDevice::updateOutEnabled();

    int i;
    for( i=m_bits-2; i>=0; --i )
        if( eLogicDevice::getInputState( i ) ) break;

    m_bcd = i+1;

    Simulator::self()->addEvent( m_propDelay, this );
}

void eDecToBcd::runEvent()
{
    for( int i=0; i<4; ++i ) setOut( i, m_bcd & (1<<i) );
}

void eDecToBcd::set_16bits( bool set )
{
    m_16Bits = set;

    if( m_16Bits ) m_bits = 16;
    else           m_bits = 10;
}

