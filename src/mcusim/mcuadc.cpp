/***************************************************************************
 *   Copyright (C) 2021 by santiago González                               *
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

#include "mcuadc.h"
#include "mcupin.h"
#include "mcuinterrupts.h"
#include "simulator.h"

McuAdc::McuAdc( eMcu* mcu, QString name )
      : McuModule( mcu, name )
      , eElement( name )
{
    m_ADCL = NULL;
    m_ADCH = NULL;
}

McuAdc::~McuAdc()
{
}

void McuAdc::initialize()
{
    m_channel = 0;
    m_enabled = false;
    m_converting = false;
}

void McuAdc::runEvent()
{
    endConversion();

    if( m_ADCL ) *m_ADCL = m_adcValue & 0x00FF;
    if( m_ADCH ) *m_ADCH = (m_adcValue & 0xFF00) >> 8;

    m_interrupt->raise(); // .emitValue( 1 );
    m_converting = false;
}

void McuAdc::startConversion()
{
    if( !m_enabled ) return;
    if( m_channel >= m_adcPin.size() ) return;
    m_converting = true;

    double volt = m_adcPin[m_channel]->getVolt();

    m_adcValue = (double)m_maxValue*volt/getVref();
    if( m_adcValue > m_maxValue ) m_adcValue = m_maxValue;

    Simulator::self()->addEvent( m_convTime, this );
}


