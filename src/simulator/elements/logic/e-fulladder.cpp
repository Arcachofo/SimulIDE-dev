/***************************************************************************
 *   Copyright (C) 2010 by santiago González                               *
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

#include "e-fulladder.h"
#include "simulator.h"

eFullAdder::eFullAdder( QString id ) 
          : eLogicDevice( id )
{
}

void eFullAdder::stamp()
{
    for( int i=0; i<m_numInputs; ++i )
    {
        eNode* enode = m_input[i]->getEpin(0)->getEnode();
        if( enode ) enode->voltChangedCallback( this );
    }
    eLogicDevice::stamp();
}

void eFullAdder::voltChanged()
{
    bool X  = getInputState( 0 );
    bool Y  = getInputState( 1 );
    bool Ci = getInputState( 2 );
    
    bool sum = (X ^ Y) ^ Ci;                    // Sum
    bool co  = (X & Ci) | (Y & Ci) | (X & Y);   // Carry out

    m_nextOutVal = 0;
    if( sum ) m_nextOutVal += 1;
    if( co  ) m_nextOutVal += 2;
    sheduleOutPuts();
}

