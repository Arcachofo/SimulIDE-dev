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
         : McuModule( mcu, name )
         , eElement( mcu->getId()+"-"+name )
{
    m_timer = NULL;
    m_ocPin = NULL;
    m_ocm = NULL;
}
McuOcUnit::~McuOcUnit( ){}

void McuOcUnit::initialize()
{
    m_enabled = false;
    m_ctrlPin = false;
    m_comMatch = 0;
    m_mode = 0;

    m_comAct = ocNON;
    m_tovAct = ocNON;
    m_ocPin->controlPin( false, false );
}

void McuOcUnit::clockStep( uint16_t count )
{
    if( count == m_extMatch ) runEvent();
}

void McuOcUnit::runEvent()  // Compare match
{
    m_interrupt->raise();   // Trigger interrupt
    drivePin( m_comAct );
}

void McuOcUnit::drivePin( ocAct_t act )
{
    if( !act ) return;
    bool pinState = false;

    if     ( act == ocTOG ) pinState = !m_ocPin->getInpState();
    else if( act == ocCLR ) pinState = !m_pinSet;
    else if( act == ocSET ) pinState =  m_pinSet;

    setPinSate( pinState );
}

void McuOcUnit::setPinSate( bool state )
{
    m_ocPin->setOutState( state );
}

void McuOcUnit::sheduleEvents( uint32_t ovf, uint32_t countVal, int rot )
{
    ovf      <<= rot;  // Used by Pic CCP PWM mode: 8+2 bits (rot=2)
    countVal <<= rot;
    uint64_t cycles = 0;
    uint64_t match;

    if( m_timer->reverse() )
    {
        match = ovf-m_comMatch;
        m_pinSet = false;
    }else{
        match = m_comMatch;
        m_pinSet = true;
    }
    Simulator::self()->cancelEvents( this );

    if( m_timer->extClocked() ) m_extMatch = match; // Using external clock
    else{
        if( (match <= ovf )&&(match >= countVal) ) // be sure next comp match is still ahead
            cycles = (match-countVal)*m_timer->scale() + m_mcu->simCycPI()/*run it 1 cycle after match*/; // cycles in ps

        if( cycles ) Simulator::self()->addEvent( cycles>>rot, this );
    }
}

void McuOcUnit::setOcActs( ocAct_t comAct, ocAct_t tovAct )
{
    m_comAct = comAct;
    m_tovAct = tovAct;
}

void McuOcUnit::ocrWriteL( uint8_t val )
{
    m_comMatch = (m_comMatch & 0xFF00) | val;
    sheduleEvents( m_timer->ovfMatch(), m_timer->getCount() );
}

void McuOcUnit::ocrWriteH( uint8_t val )
{
    m_comMatch = (m_comMatch & 0x00FF) | (uint16_t)val<<8;
    //sheduleEvents( m_timer->ovfMatch(), m_timer->getCount() );
}
