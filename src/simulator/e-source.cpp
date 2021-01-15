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

#include "e-source.h"
#include "simulator.h"

eSource::eSource( QString id, ePin* epin )
       : eElement( id )
{
    m_ePin.resize(1);
    m_ePin[0] = epin;
    m_out     = false;
    m_inverted = false;

    m_voltHigh = cero_doub;
    m_voltLow  = cero_doub;
    m_voltOut  = cero_doub;
    m_imp      = cero_doub;
    m_admit    = 1/m_imp;

    m_timeLH = 3000;
    m_timeHL = 4000;

    m_scrEnode = new eNode( id+"scr" );
    m_scrEnode->setNodeNumber(0);

    Simulator::self()->remFromEnodeList( m_scrEnode, /*delete=*/ false );
}
eSource::~eSource(){ delete m_scrEnode; }

void eSource::initialize()
{
    m_ePin[0]->setEnodeComp( m_scrEnode );
}

void eSource::stamp()
{
    m_ePin[0]->stampAdmitance( m_admit );
    stampOutput();
}

void eSource::stampOutput()
{
    m_scrEnode->setVolt( m_voltOut );
    m_ePin[0]->stampCurrent( m_voltOut/m_imp );
}

void eSource::setVoltHigh( double v )
{
    m_voltHigh = v;
    if( m_out ) m_voltOut = v;
}

void eSource::setVoltLow( double v )
{
    m_voltLow = v;
    if( !m_out ) m_voltOut = v;
}

void eSource::setOut( bool out ) // Set Output to Hight or Low
{
    if( m_inverted ) m_out = !out;
    else             m_out =  out;

    if( m_out ) m_voltOut = m_voltHigh;
    else        m_voltOut = m_voltLow;
}

void eSource::setTimedOut( bool out )
{
    if( m_inverted ) out = !out;
    if( out == m_out ) return;

    if( out )
    {
        m_voltOut = m_voltLow + 1e-6;
        Simulator::self()->addEvent( m_timeLH, this );
    }
    else
    {
        m_voltOut = m_voltHigh - 1e-6;
        Simulator::self()->addEvent( m_timeHL, this );
    }
    stampOutput();
    m_nextOut = out;
}

void eSource::runEvent()
{
    setOut( m_nextOut );
    stampOutput();
}

void eSource::setInverted( bool inverted )
{
    if( inverted == m_inverted ) return;

    m_inverted = inverted;
    m_ePin[0]->setInverted( inverted );
    
    m_out = !m_out;
    if( m_out ) m_voltOut = m_voltHigh;
    else        m_voltOut = m_voltLow;
    
    stampOutput();
}

void eSource::setImp( double imp )
{
    m_imp = imp;
    m_admit = 1/m_imp;
    eSource::stamp();
}

void eSource::setRiseTime( uint64_t time )
{
    m_timeLH = time;
}

void eSource::setFallTime( uint64_t time )
{
    m_timeHL = time;
}

double eSource::getVolt()
{
    if( m_ePin[0]->isConnected() ) return m_ePin[0]->getVolt();
    else                           return m_voltOut;
}
