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
      : IoPin( 0, QPoint(0,0), mcu->objectName()+"-"+id, 0, mcu, source )
{
    m_port   = port;
    m_number = i;
    m_id     = id;

    m_pinMask = 1<<i;

    m_outState = false;
    m_openColl = false;
    m_puMask   = false;
    m_outMask  = false;
    m_inpMask  = true;  // Inverted: true means inactive

    digital_thre = 2.5;

    setOutHighV( 5 );
    setPinMode( input );
    initialize();
}
McuPin::~McuPin() {}

void McuPin::initialize()
{
    setDirection( m_outMask );
    setPullup( m_puMask );

    IoPin::initialize();
}

void McuPin::stamp()
{
    if( m_enode ) // NO: Outputs are also called so they set Input register if needed
       changeCallBack( this ); // Receive voltage change notifications

    IoPin::stamp();
}

void McuPin::voltChanged()
{
    bool state = getVolt() > digital_thre;

    if( state == m_inpState ) return;
    m_inpState = state;

    uint8_t val = state ? m_pinMask : 0;
    m_port->pinChanged( m_pinMask, val );
}

void McuPin::setPortState( bool state )
{
    m_outState = state;
    if( !m_isOut ) return;
    IoPin::setOutState( state, true );
}

void McuPin::setOutState( bool state, bool )
{
    if( m_outCtrl ) IoPin::setOutState( state, true );
}

void McuPin::setDirection( bool out )
{
    m_isOut = (out || m_outMask) && m_inpMask; // Take care about permanent Inputs/Outputs

    if( m_isOut )  // Set Pin to Output
    {
        if( m_openColl ) m_oldPinMode =  open_col;
        else             m_oldPinMode =  output;
    }
    else           // Set Pin to Input
    {
        m_oldPinMode = input;
    }
    if( m_enode ) changeCallBack( this, !m_isOut ); // Receive voltage change notifications only if input
    if( !m_dirCtrl ) setPinMode( m_oldPinMode ); // Is someone is controlling us, just save Pin Mode
}

void McuPin::setPullup( bool up )
{
    m_pullup = up;

    if( up ) m_vddAdmEx = 1/1e5; // Activate pullup
    else     m_vddAdmEx = 0;     // Deactivate pullup

    if( !m_isOut ) updtState();
}

void McuPin::setExtraSource( double vddAdmit, double gndAdmit ) // Comparator Vref out to Pin for example
{
    m_vddAdmEx = vddAdmit;
    m_gndAdmEx = gndAdmit;

    updtState();
}

