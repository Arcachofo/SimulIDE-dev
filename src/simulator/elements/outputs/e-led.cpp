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
#include "e-pin.h"
#include "e-node.h"
#include "simulator.h"

eLed::eLed( QString id ) 
    : eResistor( id )
{
    m_imped = 0.6;
    m_threshold  = 2.4;
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

    m_admit = 1/high_imp;
    m_voltPN  = 0;
    m_deltaV  = 0;
    m_current = 0;
    m_lastCurrent = 0;
}

void eLed::stamp()
{
    eNode* node = m_ePin[0]->getEnode();
    if( node )
    {
        node->addToNoLinList( this );
        //node->setSwitched( true );
    }
    node = m_ePin[1]->getEnode();
    if( node )
    {
        node->addToNoLinList( this );
        //node->setSwitched( true );
    }
    eResistor::stamp();
}

void eLed::voltChanged()
{
    m_voltPN = m_ePin[0]->getVolt()-m_ePin[1]->getVolt();

        double deltaR = m_imped;
        double deltaV = m_threshold;

        double delta = m_threshold-m_voltPN;
        if( delta > 1e-12 )   // Not conducing
        {
            deltaV = m_voltPN;
            deltaR = high_imp;
        }
        if( deltaR != 1/m_admit )
        {
            m_admit = 1/deltaR;
            //if( deltaR == high_imp ) eResistor::setAdmit( 0 );
            //else
                eResistor::setAdmit( m_admit );
        }
        m_deltaV = deltaV;

        double current = deltaV*m_admit;
        if( deltaR == high_imp ) current = 0;

        if( current == m_lastThCurrent )
        {
            updateVI();
            return;
        }
        m_lastThCurrent = current;

        Simulator::self()->notCorverged();

        m_ePin[0]->stampCurrent( current );
        m_ePin[1]->stampCurrent(-current );
}

void eLed::updateVI()
{
    updateCurrent();

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

void eLed::setRes( double resist )
{
    Simulator::self()->pauseSim();

    if( resist == 0 ) resist = 0.1;
    m_imped = resist;
    voltChanged();

    Simulator::self()->resumeSim();
}

void eLed::updateCurrent()
{
    m_current = 0;

    if( m_admit == 1/high_imp ) return;

    if( m_ePin[0]->isConnected() && m_ePin[1]->isConnected() )
    {
        double volt = m_voltPN - m_deltaV;
        if( volt>0 ) m_current = volt*m_admit;
}   }
