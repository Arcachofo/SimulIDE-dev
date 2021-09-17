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

#include <math.h>

#include "e-led.h"
#include "simulator.h"

eLed::eLed( QString id ) 
    : eDiode( id )
{
    m_maxCurrent = 0.03;
    initialize();
}
eLed::~eLed() {}

void eLed::initialize()
{
    m_prevStep = 0;
    m_lastCurrent = 0.0;
    m_intensity = 25;
    m_brightness  = 0;
    m_avgBright   = 0;
    m_lastPeriod = 0;

    eDiode::initialize();
}

void eLed::voltChanged()
{
    eDiode::voltChanged();
    if( m_converged ) updateVI();
}

void eLed::updateVI()
{
    const uint64_t step = Simulator::self()->circTime();
    uint64_t period = (step-m_prevStep);
    m_prevStep = step;
    m_lastPeriod += period;

    if( m_lastCurrent > 0 ) m_avgBright += m_lastCurrent*period/m_maxCurrent;
    m_lastCurrent = m_current;
}

void eLed::updateBright()
{
    if( !Simulator::self()->isRunning() )
    {
        m_avgBright = 0;
        m_lastPeriod = 0;
        m_intensity = 25;
        return;
    }
    updateVI();

    uint64_t sPF = Simulator::self()->stepsPerFrame();
    uint64_t sPS = Simulator::self()->stepSize();

    if( m_lastPeriod > sPF*sPS/2 ) // Update 2 times per frame
    {
        m_brightness = pow( m_avgBright/m_lastPeriod, 1.0/2.0 );

        m_avgBright  = 0;
        m_lastPeriod = 0;
        m_intensity  = uint32_t(m_brightness*255)+25;
}   }
