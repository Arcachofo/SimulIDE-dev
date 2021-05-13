/***************************************************************************
 *   Copyright (C) 2020 by santiago González                               *
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

#include "lachannel.h"
#include "plotdisplay.h"
#include "simulator.h"
#include "utils.h"

LaChannel::LaChannel( LAnalizer* la, QString id )
         : DataChannel( la, id )
{
    m_analizer = la;

    //m_cond = C_NONE;
    m_lastCond = C_NONE;
}
LaChannel::~LaChannel()  { }

void LaChannel::initialize()
{
    m_rising  = false;
    m_risEdge = 0;

    m_bufferCounter = 0;
    m_buffer.fill(0);
    m_time.fill(0);

    updateStep();
}

void LaChannel::stamp()    // Called at Simulation Start
{
    DataChannel::stamp();
    m_analizer->conditonMet( m_channel, C_LOW );
}

void LaChannel::updateStep()
{
    voltChanged();
    double dispMax = m_analizer->voltDiv()*10;
    double dispMin = 0;
    m_analizer->display()->setLimits( m_channel, dispMax, dispMin );
}

void LaChannel::voltChanged()
{
    if( !m_connected ) return;
    uint64_t simTime = Simulator::self()->circTime();

    double volt = m_ePin[0]->getVolt();

    if( ++m_bufferCounter >= m_buffer.size() ) m_bufferCounter = 0;
    m_buffer[m_bufferCounter] = volt;
    m_time[m_bufferCounter] = simTime;

    if( volt > m_analizer->threshold() )             // High
    {
        if( !m_rising )     // Rising Edge
        {
            m_rising = true;
            m_risEdge = simTime;

            if( m_pauseOnCond )
                m_analizer->conditonMet( m_channel, C_RISING );
        }
        if( m_pauseOnCond && (m_lastCond != C_HIGH) )
        {
            m_analizer->conditonMet( m_channel, C_HIGH );
            m_lastCond = C_HIGH;
        }
    }
    else                                // Low
    {
        if( m_rising  )    // Falling Edge
        {
            m_rising = false;

            if( m_pauseOnCond )
                m_analizer->conditonMet( m_channel, C_FALLING );
        }
        if( m_pauseOnCond && (m_lastCond != C_LOW) )
        {
            m_analizer->conditonMet( m_channel, C_LOW );
            m_lastCond = C_LOW;
        }
    }
}

