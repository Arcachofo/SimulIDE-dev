/***************************************************************************
 *   Copyright (C) 2012 by santiago González                               *
 *   santigoro@gmail.com                                                   *
 *                                                                         *
 *   This program is free software; you can redistribute it and/or modify  *
 *   it under the terms of the GNU General Public License as published by  *
 *   the Free Software Foundation; either version 2 of the License, or     *
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

#include "e-resistor.h"
#include "simulator.h"
#include "e-pin.h"

eResistor::eResistor( QString id )
         : eElement( id )
{
    m_admit  = 1.0/100.0;
    m_current = 0;
    m_ePin.resize(2);
}
eResistor::~eResistor(){}

void eResistor::stamp()
{
    m_ePin[0]->setEnodeComp( m_ePin[1]->getEnode() );
    m_ePin[1]->setEnodeComp( m_ePin[0]->getEnode() );
    stampAdmit();
}

void eResistor::stampAdmit()
{
    if( !m_ePin[0] ) return;
    if( !m_ePin[0]->isConnected() || !m_ePin[1]->isConnected() ) return;

    m_ePin[0]->stampAdmitance( m_admit );
    m_ePin[1]->stampAdmitance( m_admit );
}

/*void eResistor::stampCurrent( double current )
{
    if( !m_ePin[0] ) return;

    m_ePin[0]->stampCurrent( current );
    m_ePin[1]->stampCurrent( -current );
}*/

void eResistor::setRes( double resist )
{
    if( resist < 1e-12 ) resist = 1e-12;
    setAdmit( 1/resist );
}

void eResistor::setAdmit( double admit )   // Admit can be 0
{
    m_admit = admit;
    stampAdmit();
}

void eResistor::setResSafe( double resist )
{
    Simulator::self()->pauseSim();
    eResistor::setRes( resist );
    Simulator::self()->resumeSim();
}

double eResistor::current()
{
    updateVI();
    return m_current;
}

void eResistor::updateVI()
{
    if( m_ePin[0]->isConnected() && m_ePin[1]->isConnected() )
    {
        double volt = m_ePin[0]->getVoltage()-m_ePin[1]->getVoltage();
        m_current = volt*m_admit;
    }
    else m_current = 0;
}

