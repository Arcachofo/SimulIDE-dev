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

#include "e-dac.h"
#include "simulator.h"

eDAC::eDAC( QString id )
    : eLogicDevice( id )
{
}
eDAC::~eDAC() {}

void eDAC::stamp()
{
    for( int i=0; i<m_numInputs; ++i )
    {
        eNode* enode = m_input[i]->getEpin(0)->getEnode();
        if( enode ) enode->voltChangedCallback( this );
    }
    m_output[0]->setOut( true );
    m_value = -1;
    Simulator::self()->addEvent( 1, 0l );
}

void eDAC::voltChanged()
{
    m_value = 0;

    for( int i=0; i<m_numInputs; ++i )
        if( getInputState( i ) ) m_value += pow( 2, m_numInputs-1-i );

    Simulator::self()->addEvent( m_propDelay, this );
}

void eDAC::runEvent()
{
    double v = m_maxVolt*m_value/m_maxValue;

    m_output[0]->setVoltHigh( v );
    m_output[0]->stampOutput();
}
