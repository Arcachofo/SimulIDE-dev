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

    m_openColl = false;
    m_puMask   = false;
    m_dirMask  = false;

    Pin* pin = new Pin( 0, QPoint( 0, 0 ), mcu->objectName()+id, i, mcu );
    m_ePin[0] = pin;
    setPinMode( input );

    initialize();
    setState( false );
}
McuPin::~McuPin() {}

void McuPin::initialize()
{
    m_extCtrl  = false;
    m_gndAdmit = cero_doub;
    m_vddAdmit = 0;
    m_vddAdmEx = 0;
    m_gndAdmEx = 0;
    m_pupAdmit = 0;
    //m_lastTime = 0;
    m_volt = 0;

    setDirection( m_dirMask );
    //setState( false );
    setPullup( m_puMask );

    eSource::setVoltHigh( 5 );
    update();

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
    //if( m_lastTime == Simulator::self()->circTime() ) return; // Avoid self-triggered changes

    double volt = m_ePin[0]->getVolt();

    if( fabs( volt-m_volt ) < 1e-5 ) return; // Avoid triggering because small volt changes

    if( volt > digital_thre ) m_state = 1;
    else                      m_state = 0;

    m_port->pinChanged( m_pinMask, m_state );
    m_volt = volt;
}

void McuPin::controlPin( bool ctrl )
{
    m_extCtrl = ctrl;
}

void McuPin::setPortState( bool state )
{
    if( m_extCtrl ) return;
    setState( state );
}

void McuPin::setState( bool state )
{
    m_state = state;
    if( !m_isOut ) return;

    if( m_openColl )
    {
        if( state )
        {
            m_vddAdmit = 0;
            m_gndAdmit = cero_doub;
        }
        else
        {
            m_vddAdmit = 0;
            m_gndAdmit = 1./40.;
        }
        update();
    }
    else
    {
        eSource::setState( state, true );
    }
}

void McuPin::setDirection( bool out )
{
    m_isOut = out;

    if( out )       // Set Pin to Output
    {
        //if( m_ePin[0]->isConnected() )
        //    m_ePin[0]->getEnode()->remFromChangedCallback( this ); // Don't Receive voltage change notifications

        eSource::setImp( 40 );
        setState( m_state );
    }
    else           // Set Pin to Input
    {
        //if( m_ePin[0]->isConnected() )
        //    m_ePin[0]->getEnode()->voltChangedCallback( this ); // Receive voltage change notifications

        //m_lastTime = Simulator::self()->circTime();
        m_vddAdmit = 0;
        m_gndAdmit = cero_doub;
        update();
    }
}

void McuPin::setPullup( bool up )
{
    m_pullup = up;

    if( up ) m_pupAdmit = 1/1e5; // Activate pullup
    else     m_pupAdmit = 0;     // Deactivate pullup

    /*if( !(m_ePin[0]->isConnected()) )
    {
        pullupNotConnected( up );
        return;
    }*/
    update();
}

void McuPin::update()
{
    double vddAdmit = m_vddAdmit+m_vddAdmEx+m_pupAdmit;
    double gndAdmit = m_gndAdmit+m_gndAdmEx;
    double Rth  = 1/(vddAdmit+gndAdmit);

    m_voltOut = 5*vddAdmit*Rth; // Vth

    eSource::setImp( Rth );
}
