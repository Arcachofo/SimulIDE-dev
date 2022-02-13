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
#include "datautils.h"
#include "mcuinterrupts.h"
#include "simulator.h"
#include "circuit.h"

McuPin::McuPin( McuPort* port, int i, QString id, Component* mcu )
      : IoPin( 0, QPoint(0,0), mcu->getUid()+"-"+id, 0, mcu )
{
    m_id     = id;
    m_port   = port;
    m_number = i;

    m_pinMask = 1<<i;

    m_extInt = NULL;
    m_extIntTrigger = pinLow;

    m_outState = false;
    m_openColl = false;
    m_puMask   = false;
    m_outMask  = false;
    m_inpMask  = true;  // Inverted: true means inactive
    m_changeCB = false;

    setOutHighV( 5 );
    initialize();
}
McuPin::~McuPin() {}

void McuPin::initialize()
{
    m_outCtrl = false;
    m_dirCtrl = false;
    m_isAnalog = false;
    m_portState = false;

    setDirection( m_outMask );
    setPullup( m_puMask );
    IoPin::initialize();
}

void McuPin::stamp()
{
    if( m_enode ) changeCallBack( this ); // Receive voltage change notifications
    IoPin::stamp();
}

void McuPin::voltChanged()
{
    bool state = m_inpState;

    double volt = getVolt();
    if     ( volt > m_inpHighV ) state = true;
    else if( volt < m_inpLowV  ) state = false;

    if( state != m_inpState )
    {
        if( m_extInt && m_extInt->enabled() )
        {
            bool raise = false;
            switch( m_extIntTrigger ) {
                case pinLow:     raise = !state; break;
                case pinChange:  raise = (state != m_inpState); break;
                case pinFalling: raise = (m_inpState && !state); break;
                case pinRising:  raise = (state && !m_inpState); break;
            }
            if( raise ) m_extInt->raise();
        }
        m_inpState = state;
        if( Circuit::self()->animate() )
            setPinState( m_inpState? input_high:input_low ); // High : Low colors

        uint8_t val = state ? m_pinMask : 0;
        m_port->pinChanged( m_pinMask, val );
    }
}

bool McuPin::getInpState()
{
    voltChanged();
    return m_inpState;
}

void McuPin::setPortState( bool state ) // Port Is being witten
{
    m_portState = state;
    if( m_outCtrl ) return; // Port is not controlling Pin State

    m_outState = state; /// ??? Should this be controlled by IoPin?
    if( m_isOut ) IoPin::sheduleState( state, 0 );
}

void McuPin::setOutState( bool state ) // Some periferical is controlling this Pin
{ if( m_outCtrl ) IoPin::sheduleState( state, 0 ); }

void McuPin::sheduleState( bool state, uint64_t time ) // Some periferical is controlling this Pin
{ if( m_outCtrl ) IoPin::sheduleState( state, time ); }

void McuPin::setDirection( bool out )
{
    m_isOut = (out || m_outMask) && m_inpMask; // Take care about permanent Inputs/Outputs

    if( m_isOut ) m_portMode = m_openColl ? openCo : output; // Set Pin to Output
    else          m_portMode = input;                        // Set Pin to Input

    if( m_dirCtrl ) return; // Is someone is controlling us, just save Pin Mode

    changeCallBack( this, m_changeCB || !m_isOut ); // Receive voltage change notifications only if input or always (m_changeCB)
    setPinMode( m_portMode );
}

void McuPin::controlPin( bool outCtrl, bool dirCtrl )
{
    if( !dirCtrl && m_dirCtrl ) // External control is being released
    {
        setPinMode( m_portMode ); // Set Previous Pin Direction
    }
    m_dirCtrl = dirCtrl;

    if( !outCtrl && m_outCtrl ) // External control is being released
    {
        if( m_pinMode > input ) sheduleState( m_portState, 0 ); // Set Previous Pin State
        else                    m_outState = m_portState;
    }
    m_outCtrl = outCtrl;
}

void McuPin::setExtraSource( double vddAdmit, double gndAdmit ) // Comparator Vref out to Pin for example
{
    m_vddAdmEx = vddAdmit;
    m_gndAdmEx = gndAdmit;
    updtState();
}

void McuPin::ConfExtInt( uint8_t bits )
{ m_extIntTrigger = (extIntTrig_t)getRegBitsVal( bits, m_extIntBits ); }

