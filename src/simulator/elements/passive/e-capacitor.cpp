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

// Capacitor model using backward euler approximation
// consists of a current source in parallel with a resistor.

#include "e-capacitor.h"
#include "e-pin.h"
#include "e-node.h"
#include "simulator.h"

eCapacitor::eCapacitor( QString id ) 
          : eResistor( id )
{
    m_cap = 0.00001; // Farads
    m_InitVolt = 0;
}
eCapacitor::~eCapacitor(){}

/*void eCapacitor::initialize()
{
    m_nextStep = Simulator::self()->stepSize(); // Time in ps
    m_tStep = (double)m_nextStep/1e12;          // Time in seconds
    m_curSource = 0;
    m_volt = m_InitVolt;
    m_admit = m_cap/m_tStep;
}*/

void eCapacitor::stamp()
{
    if( m_ePin[0]->isConnected() && m_ePin[1]->isConnected())
    {
        m_nextStep = Simulator::self()->stepSize(); // Time in ps
        m_tStep = (double)m_nextStep/1e12;          // Time in seconds
        m_admit = m_cap/m_tStep;
        eResistor::stamp();

        m_volt = m_InitVolt;
        m_curSource = m_volt*m_admit;
        if( m_curSource )
        {
            m_ePin[0]->stampCurrent( m_curSource );
            m_ePin[1]->stampCurrent(-m_curSource );
        }

        m_ePin[0]->changeCallBack( this );
        m_ePin[1]->changeCallBack( this );
    }
    //    Simulator::self()->addEvent( 1, this );

    m_lastTime = 0;
}

void eCapacitor::voltChanged()
{
    double volt = m_ePin[0]->getVolt() - m_ePin[1]->getVolt();

    uint64_t simTime = Simulator::self()->circTime();
    uint64_t deltaTime = simTime - m_lastTime;
    m_lastTime = simTime;

    if( deltaTime < m_nextStep )
    {
        Simulator::self()->cancelEvents( this );

        //if( deltaTime == 0 )return;
        //qDebug() << "Time:" << deltaTime << m_nextStep;
    }
//qDebug() << "Cap:"<<volt << m_ePin[0]->getVolt() << m_ePin[1]->getVolt();
    if( m_volt != volt )
    {
        m_volt = volt;
        m_curSource = volt*m_admit;
        //qDebug() << "Cap:" << m_volt << m_curSource;

        //m_ePin[0]->stampCurrent( m_curSource );
        //m_ePin[1]->stampCurrent(-m_curSource );
        Simulator::self()->addEvent( m_nextStep, this );
    }
    else qDebug() << "Final Voltage:" << volt;
}

void eCapacitor::runEvent()
{
    //double volt = m_ePin[0]->getVolt() - m_ePin[1]->getVolt();

    //if( m_volt != volt )
    {
        //m_volt = volt;
        //m_curSource = volt*m_admit;

        m_ePin[0]->stampCurrent( m_curSource );
        m_ePin[1]->stampCurrent(-m_curSource );
    }
    //Simulator::self()->addEvent( m_nextStep, this );
}

