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

#include "intmem.h"
#include "e_mcu.h"
#include "iopin.h"
#include "simulator.h"
#include "circuit.h"

IntMemModule::IntMemModule( eMcu* mcu, QString name )
            : McuModule( mcu, name )
            , eElement( mcu->getId()+"-"+name )
{
    m_rwPin = NULL;
    //m_rePin = NULL;
    m_cshPin = NULL; // Enable Pin
    m_cslPin = NULL; // !Enable Pin
    m_clkPin = NULL; // Clock Pin

    m_propDelay = 10*1000; // 10 ns
    m_asynchro = false;
}
IntMemModule::~IntMemModule() {}

//void IntMemModule::initialize(){;}

void IntMemModule::stamp()
{
    m_nextOutVal = 0;
    m_outValue = 0;
    m_cs = false;
    enableOutputs( false );
    m_clkPin->changeCallBack( this );
}

void IntMemModule::reset()  // NO: Reset happens after initialize() in Pins.
{
    m_memState = mem_IDLE;
    write( true );
}

void IntMemModule::voltChanged()
{
    bool CS = true;
    if( m_cshPin ) CS = CS && m_cshPin->getInpState();
    if( m_cslPin ) CS = CS && !m_cslPin->getInpState();
    if( m_clkPin ) CS = CS && !m_clkPin->getInpState();
    if( CS != m_cs ){
        m_cs = CS;
        //enableOutputs( CS );
    }
    if( !CS ) return;

    m_we = !m_rwPin->getInpState();
    m_addr = 0;
    for( uint i=0; i<m_addrPin.size(); ++i )        // Get Address
    {
        bool state = m_addrPin[i]->getInpState();
        if( state ) m_addr += pow( 2, i );
    }
    if( m_we ){                             // Write
        write( true );
        Simulator::self()->addEvent( m_propDelay, this );
    }
    else{                                   // Read
        write( false );
        m_nextOutVal = m_mcu->readReg( m_addr );
        sheduleOutPuts();
}   }

void IntMemModule::runEvent()
{
    if( m_write )
    {
        int value = 0;
        for( uint i=0; i<m_dataPin.size(); ++i )
        {
            bool state = m_dataPin[i]->getInpState();
            if( state ) value += pow( 2, i );
            if( Circuit::self()->animate() ) m_dataPin[i]->setPinState( state? input_high:input_low ); // High-Low colors
        }
        m_mcu->writeReg( m_addr, value );
    }
    else runOutputs();
}

void IntMemModule::write( bool w )
{
    m_write = w;
    for( IoPin* pin : m_dataPin )
    {
        pin->setPinMode( w ? input : output );
        if( m_asynchro ) pin->changeCallBack( this, w );
    }
    Simulator::self()->cancelEvents( this );
}

void IntMemModule::enableOutputs( bool en )
{
    for( uint i=0; i<m_dataPin.size(); ++i ) m_dataPin[i]->setStateZ( !en );
}

void IntMemModule::runOutputs()
{
    m_outValue = m_nextOutVal;

    for( uint i=0; i<m_dataPin.size(); ++i )
    {
        bool state = m_outValue & (1<<i);
        bool oldst = m_dataPin[i]->getOutState();

        if( state != oldst ) m_dataPin[i]->sheduleState( state, 0 );
    }
}

void IntMemModule::sheduleOutPuts()
{
    if( m_nextOutVal == m_outValue ) return;
    Simulator::self()->addEvent( m_propDelay, this );
}
