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
#include "e-source.h"
#include "circuit.h"

eClockedDevice::eClockedDevice( QString id )
              : eElement( id )
{
    m_clock     = false;
    m_clockSource = NULL;
    m_etrigger = Trig_Clk;

    m_inputImp = 1e9;

    m_inputHighV = 2.5;
    m_inputLowV  = 2.5;
}
eClockedDevice::~eClockedDevice()
{
    //if( m_clockSource ) delete m_clockSource;
}

void eClockedDevice::stamp() // Register for callBack when eNode volt change on clock pin
{
    callBack( true );
}

void eClockedDevice::callBack( bool en )
{
    if( m_clockSource )
    {
        eNode* enode = m_clockSource->getEpin(0)->getEnode();
        if( enode )
        {
            if( en) enode->voltChangedCallback( this );
            else    enode->remFromChangedCallback( this );
        }
    }
}

void eClockedDevice::seteTrigger( int trigger )
{
    m_etrigger = trigger;
    m_clock = false;
}

void eClockedDevice::createClockPin()
{
    ePin* epin = new ePin( m_elmId+"-ePin-clock", 0 );
    createClockeSource( epin );
}

void eClockedDevice::createClockPin( ePin* epin )
{
    epin->setId( m_elmId+"-ePin-clock" );
    createClockeSource( epin );
}

void eClockedDevice::createClockeSource( ePin* epin )
{
    m_clockSource = new eSource( m_elmId+"-eSource-clock", epin, input );
    m_clockSource->setInputImp( m_inputImp );
}

bool eClockedDevice::clockInv()
{ return m_clockSource->isInverted(); }

void eClockedDevice::setClockInv( bool inv )
{
    if( !m_clockSource ) return;

    bool pauseSim = Simulator::self()->isRunning();
    if( pauseSim ) Simulator::self()->pauseSim();

    m_clockSource->setInverted(inv);
    Circuit::self()->update();

    if( pauseSim ) Simulator::self()->resumeSim();
}

clkState_t eClockedDevice::getClockState()
{
    if( !m_clockSource ) return Clock_Allow;

    clkState_t cState = Clock_Low;

    bool  clock = m_clock;
    double volt = m_clockSource->getVolt(); // Clock pin volt.

    if     ( volt > m_inputHighV ) clock = true;
    else if( volt < m_inputLowV )  clock = false;

    if( m_clockSource->isInverted() ) clock = !clock;

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

    m_clockSource->getPin()->setPinState( clock? input_high:input_low ); // High-Low colors

    return cState;
}

