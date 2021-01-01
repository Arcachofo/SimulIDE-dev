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

#include "e-bcdtodec.h"
#include "simulator.h"

eBcdToDec::eBcdToDec( QString id )
         : eLogicDevice( id )
{
}
eBcdToDec::~eBcdToDec() {}

void eBcdToDec::stamp()
{
    for( int i=0; i<4; ++i )
    {
        eNode* enode = m_input[i]->getEpin()->getEnode();
        if( enode ) enode->voltChangedCallback( this );
    }
    m_dec = -1;

    eLogicDevice::stamp();
}

void eBcdToDec::initialize()
{
    eLogicDevice::initialize();
    setOut( 0, true );
    m_old = -1;
}

void eBcdToDec::voltChanged()
{
    eLogicDevice::updateOutEnabled();

    m_dec = 0;

    for( int i=0; i<4; ++i )
        if( getInputState( i ) ) m_dec += pow( 2, i );

    Simulator::self()->addEvent( m_propDelay, this );
}

void eBcdToDec::runEvent()
{
    if( m_old >= 0 ) setOut( m_old, false );
    if( m_dec >= 0 ) setOut( m_dec, true );

    m_old = m_dec;
}

void eBcdToDec::set_16bits( bool set )
{
    m_16Bits = set;
}
