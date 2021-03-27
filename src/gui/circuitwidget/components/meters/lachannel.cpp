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
}
LaChannel::~LaChannel()  { }

void LaChannel::initialize()
{
    m_rising   = false;
    m_falling  = false;

    m_risEdge = 0;
    m_lastValue = 0;
    m_bufferCounter = 0;

    m_buffer.fill(0);
    m_time.fill(0);

    updateStep();
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
    uint64_t simTime = Simulator::self()->circTime();

    double data = m_ePin[0]->getVolt();

    if( ++m_bufferCounter >= m_buffer.size() ) m_bufferCounter = 0;
    m_buffer[m_bufferCounter] = data;
    m_time[m_bufferCounter] = simTime;

    double delta = data-m_lastValue;

    if( delta > 0 )               // Rising
    {
        if( m_falling && !m_rising )     // Min To Rising
        {
            m_falling = false;
        }
        m_rising = true;
        m_risEdge = simTime;
        m_lastValue = data;
    }
    else                        // Falling
    {
        if( m_rising && !m_falling )    // Max Found
        {
            m_rising = false;
        }
        m_falling = true;
        m_lastValue = data;
    }
}

