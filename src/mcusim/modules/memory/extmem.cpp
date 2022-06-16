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
    m_rwPin = NULL;
    m_rePin = NULL;
    m_enPin = NULL;
    m_laPin = NULL;

    m_laEnSetTime  = 0;
    m_addrSetTime  = 0; //250*1e9;
    m_laEnEndTime  = 0;
    m_readSetTime  = 0; //750*1e9;
    m_writeSetTime = 0; //600*1e9;
    m_readBusTime  = 0; //800*1e9;
}
ExtMemModule::~ExtMemModule() {}

//void ExtMemModule::initialize(){;}

void ExtMemModule::reset()  // NO: Reset happens after initialize() in Pins.
{
    Simulator::self()->cancelEvents( this );
    m_memState = mem_IDLE;
    m_read = true;

    if( m_rwPin )
    {
        m_rwPin->setPinMode( output );
        m_rwPin->setOutState( true );
    }
    if( m_rePin ) m_rePin->sheduleState( true, 0 );
    if( m_enPin )
    {
        m_enPin->setPinMode( output );
        m_enPin->setOutState( true );
        m_enPin->updateStep();
    }
    if( m_laPin )
    {
        m_laPin->setPinMode( output );
        m_laPin->setOutState( false );
        m_laPin->updateStep();
    }

    for( McuPin* pin : m_addrPin )
    {
        pin->setDirection( true );
        pin->controlPin( true, true );
        pin->setOutState( false );
        pin->updateStep();
    }
    for( McuPin* pin : m_dataPin )
    {
        pin->setDirection( false );
        pin->controlPin( true, true );
        pin->updateStep();
    }
}

void ExtMemModule::runEvent()
{
    switch( m_memState ) {
        case mem_IDLE: break;
        case mem_LAEN:
        {
            m_laPin->sheduleState( true, 0 ); // Enable Latch for Low addr
            uint64_t time = m_addrSetTime - m_laEnSetTime;
            Simulator::self()->addEvent( time, this );
            m_memState = mem_ADDR;
        }break;
        case mem_ADDR:            // Set Address Bus
        {
            if( m_readMode & RW ) m_rwPin->sheduleState( m_read, 0 );

            uint64_t time;

            uint addr = m_addr;
            if( m_readMode & LA )
            {
                for( IoPin* pin : m_dataPin )  // Low byte addr to Data Pins
                {
                    bool state = m_addr & 1;
                    m_addr >>= 1;
                    pin->sheduleState( state, 0 );
                }
                addr = m_addrH;               // Addr Pins for High byte addr
                time = m_laEnEndTime - m_addrSetTime;
                m_memState = mem_LADI;
            }
            else{
                m_memState = mem_DATA;
                time = m_read ? m_readSetTime : m_writeSetTime;
                time -= m_addrSetTime;
                m_dataTime = m_addrSetTime + time;
            }

            for( IoPin* pin : m_addrPin )     // Set addr Pins states
            {
                bool state = addr & 1;
                addr >>= 1;
                pin->sheduleState( state, 0 );
            }
            Simulator::self()->addEvent( time, this );
        } break;
        case mem_LADI:           // Disable Latch (if in use)
        {
            m_laPin->sheduleState( false, 0 );
            m_memState = mem_DATA;

            uint64_t time = m_read ? m_readSetTime : m_writeSetTime;
            time -= m_laEnEndTime;
            m_dataTime = m_laEnEndTime + time;
            Simulator::self()->addEvent( time, this );
        }break;
        case mem_DATA:           // Set Data Bus for Read or Write
        {
            for( IoPin* pin : m_dataPin )
            {
                pin->setPinMode( m_read ? input : output ); // Set data  Pins In/Out

                if( !m_read )    // Writting: set data Pins states
                {
                    bool state = m_data & 1;
                    m_data >>= 1;
                    pin->sheduleState( state, 1 );
                }
            }
            if( m_read )
            {
                if( m_readMode & EN ) m_enPin->sheduleState( false, 1 ); // Set EN  Pin Low
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
            if( m_readMode & EN ) m_enPin->sheduleState( true, 1 ); // Set EN  Pin High
        } break;
    }
}

void ExtMemModule::voltChanged()
{;}

void ExtMemModule::read( uint32_t addr, uint8_t mode )
{
    m_read = true;
    m_addr = addr;
    m_readMode = mode;
    m_dataTime = 0;

    if( mode & LA ){
        m_addrH = addr >> 8;
        m_memState = mem_LAEN;
        Simulator::self()->addEvent( m_laEnSetTime, this );
    }else{
        if( m_memState != mem_IDLE )
            qDebug() << "ERROR: ExtMemModule::read Operation not finished";
        m_memState = mem_ADDR;
        Simulator::self()->addEvent( m_addrSetTime, this );
    }
}

void ExtMemModule::write( uint32_t addr, uint32_t data )
{
    m_read = false;
    m_addr = addr;
    m_data = data;
    m_memState = mem_ADDR;
    Simulator::self()->addEvent( m_addrSetTime, this );
}
