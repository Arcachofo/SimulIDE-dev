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
              : eElement( id )
{
    m_clock   = false;
    m_clkPin  = NULL;
    m_trigger = Clock;
}
eClockedDevice::~eClockedDevice(){}

void eClockedDevice::initialize()
{
    if( m_clkPin ) m_clock = m_clkPin->inverted();
    else           m_clock = false;
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

void eClockedDevice::updateClock()
{
    if( !m_clkPin ) { m_clkState = Clock_Allow; return; }

    m_clkState = Clock_Low;

    bool clock = m_clkPin->getInpState(); // Clock pin volt.

    if( m_trigger == InEnable )
    {
        if( clock ) m_clkState = Clock_Allow;
    }
    else if( m_trigger == Clock )
    {
        if     (!m_clock &&  clock ) m_clkState = Clock_Rising;
        else if( m_clock &&  clock ) m_clkState = Clock_High;
        else if( m_clock && !clock ) m_clkState = Clock_Falling;
    }
    else m_clkState = Clock_Allow;
    m_clock = clock;
}

void eClockedDevice::setTrigger( trigger_t trigger )
{
    if( Simulator::self()->isRunning() ) CircuitWidget::self()->powerCircOff();

    m_trigger = trigger;
    m_clock = false;

    if( m_trigger == None )
    {
        m_clkPin->removeConnector();
        m_clkPin->reset();
        m_clkPin->setLabelText( "" );
        m_clkPin->setVisible( false );
    }
    else if( m_trigger == Clock )
    {
        m_clkPin->setLabelText( ">" );
        m_clkPin->setVisible( true );
    }
    else if( m_trigger == InEnable )
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
