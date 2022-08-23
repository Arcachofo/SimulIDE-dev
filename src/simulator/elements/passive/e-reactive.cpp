/***************************************************************************
 *   Copyright (C) 2022 by santiago González                               *
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

#include "e-reactive.h"
#include "e-pin.h"
#include "e-node.h"
#include "simulator.h"

eReactive::eReactive( QString id )
         : eResistor( id )
{
    m_value    = 0;
    m_autoStep = 0;
    m_InitCurr = 0;
    m_InitVolt = 0;
}
eReactive::~eReactive(){}

void eReactive::stamp()
{
    eResistor::stamp();

    if( m_ePin[0]->isConnected() && m_ePin[1]->isConnected())
    {
        m_ePin[0]->createCurrent();
        m_ePin[1]->createCurrent();

        m_reacStep = Simulator::self()->reactStep(); // Time in ps
        updtReactStep();

        m_volt = m_InitVolt;
        m_curSource = m_InitCurr;
        m_curSource = updtCurr();

        if( m_curSource )
        {
            m_ePin[0]->stampCurrent( m_curSource );
            m_ePin[1]->stampCurrent(-m_curSource );
        }
        m_ePin[0]->changeCallBack( this );
        m_ePin[1]->changeCallBack( this );
    }

    m_lastTime = 0;
    m_stepError = false;
    m_running = false;
}

void eReactive::voltChanged()
{
    uint64_t simTime = Simulator::self()->circTime();
    m_deltaTime = simTime - m_lastTime;
    m_lastTime = simTime;

    if( m_deltaTime < m_reacStep && !m_stepError )
    {
        if( m_deltaTime > 10 )
        {
            if( m_autoStep )  // Auto-resize step
            {
                m_reacStep = m_deltaTime;
                updtReactStep();
            }else{
                m_stepError = true;
                stepError();
            }
        }
    }
    else if( m_stepError ) m_stepError = false;

    if( !m_running )
    {
        m_running = true;
        Simulator::self()->addEvent( m_reacStep, this );
    }
}

void eReactive::runEvent()
{
    double volt = m_ePin[0]->getVolt() - m_ePin[1]->getVolt();

    if( m_volt != volt )
    {
        m_volt = volt;
        m_curSource = updtCurr();

        m_ePin[0]->stampCurrent( m_curSource );
        m_ePin[1]->stampCurrent(-m_curSource );
        Simulator::self()->addEvent( m_reacStep, this );
    }
    else m_running = false;
}

void eReactive::updtReactStep()
{
    if( m_autoStep ) m_reacStep = m_reacStep/m_autoStep;
    m_tStep = (double)m_reacStep/1e12;         // Time in seconds
    eResistor::setRes( updtRes() );

    m_running = false;
    Simulator::self()->cancelEvents( this );
}
