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

#include "mcupin.h"
#include "mcuport.h"
#include "simulator.h"

McuPin::McuPin( McuPort* port, int i, QString id, Component* mcu )
      : eSource( id, 0l, source )
{
    m_port   = port;
    m_number = i;
    m_id     = id;

    m_pinMask = 1<<i;

    m_outState = false;
    m_openColl = false;
    m_puMask   = false;
    m_dirMask  = false;

    Pin* pin = new Pin( 0, QPoint( 0, 0 ), mcu->objectName()+id, i, mcu );
    m_ePin[0] = pin;

    setVoltHigh( 5 );
    setPinMode( input );
    initialize();
}
McuPin::~McuPin() {}

void McuPin::initialize()
{
    m_extCtrl = false;
    m_inState = false;

    setDirection( m_dirMask );
    setPullup( m_puMask );

    eSource::initialize();
}

void McuPin::stamp()
{
    if( m_ePin[0]->isConnected() )
        m_ePin[0]->getEnode()->voltChangedCallback( this ); // Receive voltage change notifications

    eSource::stamp();
}

void McuPin::voltChanged()
{
    bool state = m_ePin[0]->getVolt() > digital_thre;

    if( state == m_inState ) return;
    m_inState = state;

    m_port->pinChanged( m_pinMask, state );
}

void McuPin::setOutState( bool state )
{
    m_outState = state;
    if( !m_isOut ) return;

    eSource::setState( state, true );
}

bool McuPin::getState()
{
    if( m_pinMode == input ) return m_inState;
    else                     return m_outState;
}

void McuPin::setDirection( bool out )
{
    m_isOut = out;

    if( out )       // Set Pin to Output
    {
        if( m_openColl ) setPinMode( output_open );
        else             setPinMode( output );

        eSource::setState( m_outState, true );
        //setState( m_state );
    }
    else           // Set Pin to Input
    {
        setPinMode( input );
        //update();
    }
}

void McuPin::setPullup( bool up )
{
    m_pullup = up;

    if( up ) m_vddAdmEx = 1/1e5; // Activate pullup
    else     m_vddAdmEx = 0;     // Deactivate pullup

    update();
}

void McuPin::setExtraSource( double vddAdmit, double gndAdmit ) // Comparator Vref out to Pin for example
{
    m_vddAdmEx = vddAdmit;
    m_gndAdmEx = gndAdmit;

    update();
}

void McuPin::controlPin( bool ctrl )
{
    m_extCtrl = ctrl;
}


