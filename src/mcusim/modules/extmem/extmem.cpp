/***************************************************************************
 *   Copyright (C) 2022 by santiago González                               *
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

#include "extmem.h"
#include "e_mcu.h"
#include "mcupin.h"
#include "simulator.h"

ExtMemModule::ExtMemModule( eMcu* mcu, QString name )
            : McuModule( mcu, name )
            , eElement( mcu->getId()+"-"+name )
{
    m_addrSetTime  = 250*1e9;
    m_readSetTime  = 750*1e9;
    m_writeSetTime = 600*1e9;
    m_readBusTime  = 800*1e9;
}
ExtMemModule::~ExtMemModule() {}

//void ExtMemModule::initialize(){;}

void ExtMemModule::reset()
{
    m_memState = mem_IDLE;
    m_read = true;
    for( McuPin* pin : m_addrPin )
    {
        pin->setDirection( true );
        pin->controlPin( true, true );
        //pin->setOutHighV( 5 );
        //pin->setPinMode( output );
        pin->updateStep();
    }
    for( McuPin* pin : m_dataPin )
    {
        pin->setDirection( false );
        pin->controlPin( true, true );
        //pin->setOutHighV( 5 );
        //pin->setPinMode( input );
        pin->updateStep();
    }
    m_rwPin->setDirection( true );
    m_rwPin->controlPin( true, true );
    //m_rwPin->setOutHighV( 5 );
    //m_rwPin->setPinMode( output );
    m_rwPin->sheduleState( true, 0 );
}

void ExtMemModule::runEvent()
{
    switch( m_memState ) {
        case mem_IDLE: break;
        case mem_START:            // Set Address Bus
        {
            m_rwPin->sheduleState( m_read, 0 );
            for( IoPin* pin : m_addrPin )
            {
                bool state = m_addr & 1;
                m_addr >>= 1;
                pin->sheduleState( state, 0 );
            }
            uint64_t time = m_read ? m_readSetTime : m_writeSetTime;
            time -= m_addrSetTime;
            m_dataTime = m_addrSetTime + time;
            Simulator::self()->addEvent( time, this );
            m_memState = mem_DATA;
        } break;
        case mem_DATA:           // Set Data Bus
        {
            for( IoPin* pin : m_dataPin )
            {
                pin->setPinMode( m_read ? input : output );
                if( !m_read )
                {
                    bool state = m_data & 1;
                    m_data >>= 1;
                    pin->sheduleState( state, 1 );
                }
            }
            if( m_read )
            {
                uint64_t time = m_readBusTime - m_dataTime;
                Simulator::self()->addEvent( time, this );
            }
            m_memState = m_read ? mem_READ : mem_IDLE;
        } break;
        case mem_READ:           // Read Data Bus
        {
            m_data = 0;
            for( uint i=0; i<m_dataPin.size(); ++i )
            {
                bool state = m_dataPin[i]->getInpState();
                if( state ) m_data += pow( 2, i );
            }
            m_memState = mem_IDLE;
        } break;
    }
}

void ExtMemModule::voltChanged()
{;}

void ExtMemModule::read( uint32_t addr )
{
    m_read = true;
    m_addr = addr;
    m_memState = mem_START;
    Simulator::self()->addEvent( m_addrSetTime, this );
}

void ExtMemModule::write( uint32_t addr, uint32_t data )
{
    m_read = false;
    m_addr = addr;
    m_data = data;
    m_memState = mem_START;
    Simulator::self()->addEvent( m_addrSetTime, this );
}
