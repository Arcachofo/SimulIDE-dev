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

#include "e-latch_d.h"
#include "simulator.h"

eLatchD::eLatchD( QString id, int channels )
       : eLogicDevice( id )
{
}
eLatchD::~eLatchD(){}

void eLatchD::stamp()
{
    if( m_etrigger != Trig_Clk )
    {
        for( uint i=0; i<m_input.size(); ++i )
        {
            eNode* enode = m_input[i]->getEpin(0)->getEnode();
            if( enode ) enode->voltChangedCallback( this );
        }
    }
    eLogicDevice::stamp();
}

void eLatchD::voltChanged()
{
    eLogicDevice::updateOutEnabled();

    if( getClockState() == Clock_Allow )
    {
        m_nextOutVal = 0;
        for( int i=0; i<m_numOutputs; ++i )
            if( getInputState( i ) ) m_nextOutVal |= 1<<i;
    }
    sheduleOutPuts();
}

