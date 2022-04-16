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

// Inductor model using backward euler approximation
// consists of a current source in parallel with a resistor.

#include <math.h>

#include "e-inductor.h"
#include "e-pin.h"
#include "e-node.h"
#include "simulator.h"

eInductor::eInductor( QString id )
         : eResistor( id )
{
    m_ind = 1; // H
}
eInductor::~eInductor(){}

void eInductor::initialize()
{
    m_nextStep = Simulator::self()->stepSize();
    m_tStep = (double)m_nextStep/1e12;
    m_curSource = 0;
    m_volt = 0;
    m_admit = m_tStep/m_ind;
}

void eInductor::stamp()
{
    eResistor::stamp();

    if( m_ePin[0]->isConnected() && m_ePin[1]->isConnected())
        Simulator::self()->addEvent( 1, this );
}

void eInductor::runEvent()
{
    double volt = m_ePin[0]->getVolt() - m_ePin[1]->getVolt();

    if( m_volt != volt)
    {
        m_volt = volt;
        m_curSource += volt*m_admit;

        m_ePin[0]->stampCurrent(-m_curSource );
        m_ePin[1]->stampCurrent( m_curSource );
    }
    Simulator::self()->addEvent( m_nextStep, this );
}

void  eInductor::setInd( double h ) 
{ 
    m_ind = h; 
    eResistor::setResSafe( m_ind/m_tStep );
}

