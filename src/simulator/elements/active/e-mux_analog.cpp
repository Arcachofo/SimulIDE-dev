/***************************************************************************
 *   Copyright (C) 2019 by santiago González                               *
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

#include "e-mux_analog.h"
#include "e-resistor.h"
#include "simulator.h"
#include "e-node.h"

eMuxAnalog::eMuxAnalog( QString id )
          : eElement( id )
{
    m_channels = 0;
    m_addrBits = 0;
    m_address  = 0;
    m_admit = 1;
}
eMuxAnalog::~eMuxAnalog()
{ 
    for( eResistor* res : m_resistor ) delete res;
    for( ePin* pin : m_chanPin ) delete pin;
    for( ePin* pin : m_addrPin ) delete pin;
}

void eMuxAnalog::stamp()
{
    eNode* enode = m_inputPin->getEnode();
    if( enode ) enode->setSwitched( true );
    
    for( int i=0; i<m_channels; ++i )
    {
        m_ePin[i]->setEnode( enode );
        
       eNode* node = m_chanPin[i]->getEnode();
       if( node ) node->setSwitched( true );
    }
    
    enode = m_enablePin->getEnode();
    if( enode ) enode->voltChangedCallback( this );
    
    for( int i=0; i<m_addrBits; ++i )
    {
        enode = m_addrPin[i]->getEnode();
        if( enode ) enode->voltChangedCallback( this );
    }
    m_enabled = false;
}

void eMuxAnalog::voltChanged()
{
    bool enabled = m_enablePin->getVolt() < 2.5;
    
    m_enabled = enabled;
    
    int address = 0;
    for( int i=0; i<m_addrBits; ++i )
    {
        bool state = (m_addrPin[i]->getVolt()>2.5);
        if( state ) address += pow( 2, i );
    }
    m_address = address;

    Simulator::self()->addEvent( m_propDelay, this );
}

void eMuxAnalog::runEvent()
{
    if( m_enabled )
    {
        for( int i=0; i<m_channels; ++i )
        {
            if( i == m_address )
            {
                if( m_resistor[i]->admit() == 0 )
                    m_resistor[i]->setAdmit( m_admit );
            }
            else if( m_resistor[i]->admit() != 0 )
                m_resistor[i]->setAdmit( 0 );
        }
    }else
    {
        for( int i=0; i<m_channels; ++i )
        {
            if( m_resistor[i]->admit() != 0 )
                m_resistor[i]->setAdmit( 0 );
        }
    }
}

double eMuxAnalog::resist() 
{ 
    return 1/m_admit; 
}

void eMuxAnalog::setResist( double r )
{
    m_admit = 1/r;
}

void eMuxAnalog::setBits( int bits )
{
    m_inputPin  = new ePin( m_elmId+"-PinInput", 0 );
    m_enablePin = new ePin( m_elmId+"-PinEnable", 0 );
    
    if( bits < 1 ) bits = 1;
    
    m_addrBits = bits;
    m_addrPin.resize( bits );

    for( int i=0; i<m_addrBits; ++i )
    {
        m_addrPin[i] = new ePin( m_elmId+"-pinAddr"+i, i );
    }
    m_channels = pow( 2, bits );
    m_resistor.resize( m_channels );
    m_chanPin.resize( m_channels );
    //m_ePin.resize( m_channels );
    setNumEpins( m_channels );

    for( int i=0; i<m_channels; ++i )
    {
        m_resistor[i] = new eResistor( m_elmId+"-resistor"+i );
        m_resistor[i]->setEpin( 0, m_ePin[i] );
        m_chanPin[i] = new ePin( m_elmId+"-pinY"+i, i );
        m_resistor[i]->setEpin( 1, m_chanPin[i] );
        m_resistor[i]->setAdmit( 0 );
    }
    Simulator::self()->addEvent( 0, NULL );
    //qDebug() << "eMuxAnalog::setBits"<<m_ePin.size();
}

