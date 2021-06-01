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
#include "circuitwidget.h"
#include "simulator.h"
#include "circuit.h"
#include "iopin.h"

eClockedDevice::eClockedDevice( QString id )
              : eElement ( id )
{
    m_clock    = false;
    m_clkPin = NULL;
    m_trigger  = Component::Clock;
}
eClockedDevice::~eClockedDevice(){}

void eClockedDevice::initialize()
{
    m_clock = false;
}

void eClockedDevice::stamp()
{
    if( m_clkPin ) m_clkPin->changeCallBack( this );
}

bool eClockedDevice::clockInv()
{
    return m_clkPin->inverted();
}

void eClockedDevice::setClockInv( bool inv )
{
    if( !m_clkPin ) return;

    Simulator::self()->pauseSim();

    m_clkPin->setInverted( inv );
    Circuit::self()->update();

    Simulator::self()->resumeSim();
}

clkState_t eClockedDevice::getClockState()
{
    if( !m_clkPin ) return Clock_Allow;

    clkState_t cState = Clock_Low;

    bool clock = m_clkPin->getInpState(); // Clock pin volt.

    if( m_trigger == Component::InEnable )
    {
        if     (!clock ) cState = Clock_Low;
        else if( clock ) cState = Clock_Allow;
    }
    else if( m_trigger == Component::Clock )
    {
        if     (!m_clock &&  clock ) cState = Clock_Rising;
        else if( m_clock &&  clock ) cState = Clock_High;
        else if( m_clock && !clock ) cState = Clock_Falling;
    }
    else cState = Clock_Allow;
    m_clock = clock;

    return cState;
}

void eClockedDevice::setTrigger( Component::trigger_t trigger )
{
    if( Simulator::self()->isRunning() ) CircuitWidget::self()->powerCircOff();

    m_trigger = trigger;
    m_clock = false;

    if( trigger == Component::None )
    {
        m_clkPin->removeConnector();
        m_clkPin->reset();
        m_clkPin->setLabelText( "" );
        m_clkPin->setVisible( false );
    }
    else if( trigger == Component::Clock )
    {
        m_clkPin->setLabelText( ">" );
        m_clkPin->setVisible( true );
    }
    else if( trigger == Component::InEnable )
    {
        m_clkPin->setLabelText( " IE" );
        m_clkPin->setVisible( true );
    }
    Circuit::self()->update();
}

void eClockedDevice::remove()
{
    if( m_clkPin) m_clkPin->removeConnector();
}
