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

#include "e-mux.h"
#include "simulator.h"

eMux::eMux( QString id )
    : eLogicDevice( id )
{
}
eMux::~eMux() {}

void eMux::stamp()
{
    for( int i=0; i<11; ++i )
    {
        eNode* enode = m_input[i]->getEpin()->getEnode();
        if( enode ) enode->voltChangedCallback( this );
    }
    eLogicDevice::stamp();
}

void eMux::voltChanged()
{
    eLogicDevice::updateOutEnabled();
    
    int address = 0;
    
    for( int i=8; i<11; ++i )
        if( getInputState( i ) ) address += pow( 2, i-8 );

    m_out = getInputState( address );

    Simulator::self()->addEvent( m_propDelay, this );
}

void eMux::runEvent()
{
    eLogicDevice::setOut( 0, m_out );
    eLogicDevice::setOut( 1,!m_out );
}
