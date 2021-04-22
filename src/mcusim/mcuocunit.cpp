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

#include "mcuocunit.h"
#include "mcupin.h"
#include "e_mcu.h"
#include "simulator.h"

McuOcUnit::McuOcUnit( eMcu* mcu, QString name )
         : eElement( name )
{
    m_mcu = mcu;
    m_timer = NULL;
    m_ocPin = NULL;
}
McuOcUnit::~McuOcUnit( ){}

void McuOcUnit::initialize()
{
    m_comMatch  = 0;
    m_mode = 0;

    m_comAct = ocNONE;
    m_tovAct = ocNONE;
}

void McuOcUnit::runEvent()           // Compare match
{
    on_match.emitValue( 1 );         // Trigger interrupt

    drivePin( m_comAct );
}

void McuOcUnit::tov()
{
    drivePin( m_tovAct );
}

void McuOcUnit::drivePin( ocAct_t act )
{
    if( !act ) return;
    bool pinState;

    if     ( act == ocTOGGLE ) pinState = !m_ocPin->getState();
    else if( act == ocCLEAR )  pinState = !m_pinSet;
    else if( act == ocSET )    pinState =  m_pinSet;

    m_ocPin->setState( pinState );
}

void McuOcUnit::sheduleEvents( uint32_t ovf, uint8_t countVal )
{
    uint64_t cycles = 0;
    uint64_t match;

    if( m_timer->m_reverse )
    {
         match = ovf-m_comMatch;
         m_pinSet = false;
    }
    else
    {
        match = m_comMatch;
        m_pinSet = true;
    }
    if( (match < ovf )&&(match > countVal) ) // be sure next comp match is still ahead
        cycles = (match-countVal)*m_timer->m_scale; // cycles in ps

    if( cycles ) Simulator::self()->addEvent( cycles, this );
    else         Simulator::self()->cancelEvents( this );
}

void McuOcUnit::ocrChanged( uint8_t val )
{
    m_comMatch = val;
}

void McuOcUnit::setOcActs( ocAct_t comAct, ocAct_t tovAct )
{
    m_comAct = comAct;
    m_tovAct = tovAct;
}
