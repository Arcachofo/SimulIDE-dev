/***************************************************************************
 *   Copyright (C) 2021 by santiago González                               *
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

#include "e-clocked_device.h"
#include "simulator.h"
#include "iopin.h"
#include "circuit.h"

eClockedDevice::eClockedDevice()
{
    m_clock    = false;
    m_clockPin = NULL;
    m_etrigger = Trig_Clk;
}
eClockedDevice::~eClockedDevice(){}

void eClockedDevice::initState()
{
    m_clock = false;
}

void eClockedDevice::stamp( eElement* el )
{
    // Register for callBack when eNode volt change on clock pin
    if( m_clockPin )
    {
        eNode* enode = m_clockPin->getEnode();
        if( enode ) enode->voltChangedCallback( el );
    }
}

void eClockedDevice::seteTrigger( int trigger )
{
    m_etrigger = (trigtType_t)trigger;
    m_clock = false;
}

bool eClockedDevice::clockInv()
{
    return m_clockPin->isInverted();
}

void eClockedDevice::setClockInv( bool inv )
{
    if( !m_clockPin ) return;

    bool pauseSim = Simulator::self()->isRunning();
    if( pauseSim ) Simulator::self()->pauseSim();

    m_clockPin->setInverted( inv );
    Circuit::self()->update();

    if( pauseSim ) Simulator::self()->resumeSim();
}

clkState_t eClockedDevice::getClockState()
{
    if( !m_clockPin ) return Clock_Allow;

    clkState_t cState = Clock_Low;

    bool clock = m_clockPin->getInpState(); // Clock pin volt.

    if( m_etrigger == Trig_InEn )
    {
        if     (!clock ) cState = Clock_Low;
        else if( clock ) cState = Clock_Allow;
    }
    else if( m_etrigger == Trig_Clk )
    {
        if     (!m_clock &&  clock ) cState = Clock_Rising;
        else if( m_clock &&  clock ) cState = Clock_High;
        else if( m_clock && !clock ) cState = Clock_Falling;
    }
    else cState = Clock_Allow;
    m_clock = clock;

    return cState;
}

