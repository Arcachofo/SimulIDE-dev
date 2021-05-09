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
#include "e-source.h"

eBcdToDec::eBcdToDec( QString id )
         : eLogicDevice( id )
{
}
eBcdToDec::~eBcdToDec() {}

void eBcdToDec::stamp()
{
    for( int i=0; i<4; ++i )
    {
        eNode* enode = m_input[i]->getEpin(0)->getEnode();
        if( enode ) enode->voltChangedCallback( this );
    }
    eLogicDevice::stamp();
}

void eBcdToDec::initialize()
{
    eLogicDevice::initialize();
    setOut( 0, true );
}

void eBcdToDec::voltChanged()
{
    eLogicDevice::updateOutEnabled();

    int dec = 0;

    for( int i=0; i<4; ++i )
        if( getInputState( i ) ) dec += pow( 2, i );

    m_nextOutVal = 1<<dec;
    sheduleOutPuts();
}

void eBcdToDec::set_16bits( bool set )
{
    m_16Bits = set;
}
