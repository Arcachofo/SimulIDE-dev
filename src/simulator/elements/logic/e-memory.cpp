/***************************************************************************
 *   Copyright (C) 2018 by santiago González                               *
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

#include "e-memory.h"
#include "simulator.h"

eMemory::eMemory( QString id )
       : eLogicDevice( id )
{
    m_persistent = false;
    m_addrBits = 8;
    m_dataBits = 8;
    m_dataBytes = 1;
    
    m_dataPinState.resize( 8 );
    m_ram.resize(256);
    
    initialize();
}
eMemory::~eMemory(){}

void eMemory::stamp()                   // Called at Simulation Start
{
    for( int i=0; i<2+m_addrBits; ++i ) // Initialize control pins
    {
        eNode* enode =  m_input[i]->getEpin(0)->getEnode();
        if( enode ) enode->voltChangedCallback( this );
    }
    eLogicDevice::stamp();
}

void eMemory::initialize()
{
    m_we = true;
    m_cs = true;
    m_oe = true;
    m_read = false;
    
    double imp = 1e28;
    for( int i=0; i<m_numOutputs; ++i )
    {
        m_dataPinState[i] = false;
        m_output[i]->setImp( imp );
    }
    if( !m_persistent ) m_ram.fill( 0 );
    
    eLogicDevice::initialize();
}

void eMemory::voltChanged()        // Some Pin Changed State, Manage it
{
    bool CS = getInputState( 1 );
    bool csTrig = false;

    if( CS != m_cs )
    {
        if( CS && !m_cs ) csTrig = true;
        m_cs = CS;
        
        if( !CS && m_oe )
        {
            m_oe = false;
            setOutputEnabled( false ); // Deactivate
        }
    }
    if( !CS ) return;

    bool WE = getInputState( 0 );
    bool oe = outputEnabled() && !WE;
    
    if( oe != m_oe )
    {
        m_oe = oe;
        setOutputEnabled( oe );
    }

    m_address = 0;
    for( int i=0; i<m_addrBits; ++i )        // Get Address
    {
        bool state = getInputState(i+2);
        if( state ) m_address += pow( 2, i );
    }

    bool weTrig = WE && !m_we;
    m_we = WE;
    if( WE )                                // Write
    {
        if( csTrig || weTrig)  // Write action triggered
        {
            m_read = false;
            Simulator::self()->addEvent( m_propDelay, this );
        }
    }else                                  // Read
    {
        m_read = true;
        m_nextOutVal = m_ram[m_address];
        sheduleOutPuts();
    }
}

void eMemory::runEvent()
{
    if( m_read ) eLogicDevice::runEvent();
    else
    {
        int value = 0;
        for( int i=0; i<m_numOutputs; ++i )
        {
            int  volt  = m_output[i]->getEpin(0)->getVolt();
            bool state = m_dataPinState[i];

            if     ( volt > m_inputHighV ) state = true;
            else if( volt < m_inputLowV )  state = false;

            m_dataPinState[i] = state;
            if( state ) value += pow( 2, i );
        }
        m_ram[m_address] = value;
    }
}

void eMemory::setMem( QVector<int> m )
{
    if( m.size() == 1 ) return;       // Avoid loading data if not saved
    m_ram = m;
}

QVector<int> eMemory::mem()
{
    if( !m_persistent ) 
    {
        QVector<int> null;
        return null;
    }
    return m_ram;
}

int eMemory::addrBits()
{
    return m_addrBits;
}

void eMemory::setAddrBits( int bits )
{
    if( bits == 0 ) bits = 8;
    m_addrBits = bits;
    
    m_ram.resize( pow( 2, bits ) );
}

int eMemory::dataBits()
{
    return m_dataBits;
}

void eMemory::setDataBits( int bits )
{
    if( bits == 0 ) bits = 8;
    m_dataBits = bits;

    int bytes = bits/8;
    if( bits%8 ) bytes++;
    m_dataBytes = bytes;
    
    m_dataPinState.resize( bits );
}

bool eMemory::persistent()
{
    return m_persistent;
}

void eMemory::setPersistent( bool p )
{
    m_persistent = p;
}
