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

#include "e-bincounter.h"
#include "simulator.h"

eBinCounter::eBinCounter( QString id )
           : eLogicDevice( id )
{
    m_TopValue = 1;
    initialize();
}
eBinCounter::~eBinCounter() {}

void eBinCounter::stamp()
{
    eNode* enode = m_input[0]->getEpin()->getEnode();              // Reset pin
    if( enode ) enode->voltChangedCallback( this );
    
    eLogicDevice::stamp();
}

void eBinCounter::initialize()
{
    m_Counter = 0;
    eLogicDevice::initialize();
}

void eBinCounter::voltChanged()
{
    bool clkRising = (eLogicDevice::getClockState() == Clock_Rising);

    if( eLogicDevice::getInputState( 0 ) == true ) // Reset
    {
       m_Counter = 0;
       m_runOut = false;
       Simulator::self()->addEvent( m_propDelay, this );
    }
    else if( clkRising )
    {
        m_Counter++;

        if( m_Counter == m_TopValue )
        {
            m_runOut = true;
            Simulator::self()->addEvent( m_propDelay, this );
        }
        else if( m_Counter > m_TopValue )
        {
            m_Counter = 0;
            m_runOut = false;
            Simulator::self()->addEvent( m_propDelay, this );
        }
    }
}

void eBinCounter::runEvent()
{
    eLogicDevice::setOut( 0, m_runOut );
}

int eBinCounter::TopValue() const
{
    return m_TopValue;
}

void eBinCounter::setTopValue( int TopValue )
{
    m_TopValue = TopValue;
}

void eBinCounter::setResetInv( bool inv )
{
    m_resetInv = inv;
    m_input[0]->setInverted( inv );                       // Input Reset
}
