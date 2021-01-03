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

#include "e-led.h"
#include "simulator.h"

eLed::eLed( QString id ) 
    : eDiode( id )
{
    m_threshold  = 2.4;
    m_maxCurrent = 0.03;
    initialize();
}
eLed::~eLed() {}

void eLed::initialize()
{
    m_prevStep = 0;
    m_lastCurrent = 0.0;
    m_bright = 25;
    m_disp_brightness  = 0;
    m_avg_brightness   = 0;
    m_lastUpdatePeriod = 0;

    eDiode::initialize();
}

void eLed::voltChanged()
{
    eDiode::voltChanged();
    updateVI();
}
void eLed::updateVI()
{
    eDiode::updateVI();
    
    const uint64_t step = Simulator::self()->circTime();
    uint64_t period = (step-m_prevStep);

    m_prevStep = step;
    m_lastUpdatePeriod += period;

    if( m_lastCurrent > 0) m_avg_brightness += m_lastCurrent*period/m_maxCurrent;
    
    m_lastCurrent = m_current;
}

void eLed::updateBright()
{
    if( !Simulator::self()->isRunning() )
    {
        m_avg_brightness = 0;
        m_lastUpdatePeriod = 0;
        m_bright = 25;
        return;
    }
    updateVI();

    uint64_t sPF = Simulator::self()->stepsPerFrame();
    uint64_t sPS = Simulator::self()->stepSize();

    if( m_lastUpdatePeriod > sPF*sPS )
    {
        m_disp_brightness = m_avg_brightness/m_lastUpdatePeriod;
        
        m_disp_brightness = pow( m_disp_brightness, 1.0/2.0 );

        m_avg_brightness   = 0;
        m_lastUpdatePeriod = 0;
        m_bright = uint32_t(m_disp_brightness*255)+25;
    }
}
