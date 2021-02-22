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

#include "e-source.h"
#include "simulator.h"

eSource::eSource( QString id, ePin* epin, pinMode_t mode )
       : eElement( id )
{
    m_ePin.resize(1);
    m_ePin[0] = epin;

    m_scrEnode = new eNode( id+"scr" );
    m_scrEnode->setNodeNumber(0);
    Simulator::self()->remFromEnodeList( m_scrEnode, /*delete=*/ false );

    m_state    = false;
    m_stateZ   = false;
    m_inverted = false;

    m_voltHigh = cero_doub;
    m_voltLow  = cero_doub;
    m_voltOut  = cero_doub;

    m_inputImp = high_imp;
    m_openImp  = 1e28;
    m_outputImp = 40;
    m_imp = cero_doub;
    m_admit = 1/ m_imp;

    m_pinMode = source;
    if( mode > source ) setPinMode( mode );
}
eSource::~eSource(){ delete m_scrEnode; }

void eSource::initialize()
{
    m_ePin[0]->setEnodeComp( m_scrEnode );
}

void eSource::stamp()
{
    m_ePin[0]->stampAdmitance( m_admit );
    stampOutput();
}

void eSource::stampOutput()
{
    m_scrEnode->setVolt( m_voltOut );
    m_ePin[0]->stampCurrent( m_voltOut/m_imp );
}

void eSource::setPinMode( pinMode_t mode )
{
    if( m_pinMode == mode ) return;
    m_pinMode = mode;

    if( mode == source )
    {
        setImp( cero_doub );
    }
    else if( mode == input )
    {
        m_voltOut = cero_doub;
        setImp( m_inputImp );
        m_ePin[0]->setPinState( 0 );
    }
    else if( mode == output )
    {
        if( m_inverted ) m_state = !m_state;
        setState( m_state );
        setImp( m_outputImp );
    }
    else if( mode == output_open )
    {
        m_voltOut = m_voltLow;
        if( m_inverted ) m_state = !m_state;
        setState( m_state );
    }
}

void eSource::setState( bool out, bool st ) // Set Output to Hight or Low
{
    if( m_inverted ) m_state = !out;
    else             m_state =  out;

    if( m_stateZ ) return;

    if( m_pinMode == output_open )
    {
        if( m_state ) m_admit = 1/m_openImp;
        else          m_admit = 1/m_outputImp;
        if( st ) stamp();
        m_ePin[0]->setPinState( m_state? 3:1 ); // Z : Low colors
    }else{
        if( m_state ) m_voltOut = m_voltHigh;
        else          m_voltOut = m_voltLow;
        if( st ) stampOutput();
        if( m_pinMode == output ) m_ePin[0]->setPinState( m_state? 2:1 ); // High : Low colors
    }
}

void eSource::setStateZ( bool z )
{
    m_stateZ = z;
    if( z )
    {
        m_voltOut = m_voltLow;
        setImp( m_openImp );
        m_ePin[0]->setPinState( 3 );
    }else{
        pinMode_t pm = m_pinMode; // Force pinMode
        m_pinMode = undefined;
        setPinMode( pm );
    }
}

void eSource::setVoltHigh( double v )
{
    m_voltHigh = v;
    if( m_state ) m_voltOut = v;
}

void eSource::setVoltLow( double v )
{
    m_voltLow = v;
    if( !m_state ) m_voltOut = v;
}

void eSource::setImp( double imp )
{
    m_imp = imp;
    m_admit = 1/m_imp;
    eSource::stamp();
}

void eSource::setInverted( bool inverted )
{
    if( inverted == m_inverted ) return;

    if( inverted ) setState( !m_state );
    else           setState( m_state );

    m_inverted = inverted;
    m_ePin[0]->setInverted( inverted );
}

double eSource::getVolt()
{
    if( m_ePin[0]->isConnected() ) return m_ePin[0]->getVolt();
    else                           return m_voltOut;
}
