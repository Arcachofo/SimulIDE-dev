/***************************************************************************
 *   Copyright (C) 2021 by santiago González                               *
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

#include "iopin.h"
#include "simulator.h"

IoPin::IoPin(int angle, const QPoint pos, QString id, int index, Component* parent, pinMode_t mode )
       : Pin( angle, pos, id, index, parent )
       , eElement( id )
{
    m_scrEnode = new eNode( id+"scr" );
    m_scrEnode->setNodeNumber(0);
    Simulator::self()->remFromEnodeList( m_scrEnode, /*delete=*/ false );

    m_state    = false;
    m_stateZ   = false;
    m_inverted = false;

    m_voltHigh = cero_doub;
    m_voltLow  = cero_doub;
    m_voltOut  = cero_doub;

    m_vddAdmit = 0;
    m_gndAdmit = cero_doub;
    m_vddAdmEx = 0;
    m_gndAdmEx = 0;

    m_inputImp = high_imp;
    m_openImp  = 1e28;
    m_outputImp = 40;
    m_imp = cero_doub;
    m_admit = 1/ m_imp;

    m_pinMode = undef_mode;
    setPinMode( mode );
}
IoPin::~IoPin(){ delete m_scrEnode; }

void IoPin::initialize()
{
    ePin::setEnodeComp( m_scrEnode );
}

void IoPin::stamp()
{
    stampAll();
}

void IoPin::stampAll()
{
    ePin::stampAdmitance( m_admit );
    stampOutput();
}

void IoPin::stampOutput()
{
    m_scrEnode->setVolt( m_voltOut );
    ePin::stampCurrent( m_voltOut/m_imp );
}

void IoPin::setPinMode( pinMode_t mode )
{
    if( m_pinMode == mode ) return;
    m_pinMode = mode;

    if( mode == source )
    {
        m_vddAdmit = 1/cero_doub;
        m_gndAdmit = cero_doub;
        ePin::setPinState( out_high );
    }
    else if( mode == input )
    {
        m_vddAdmit = 0;
        m_gndAdmit = 1/m_inputImp;

        ePin::setPinState( input_low );
    }
    else if( mode == output )
    {
        m_vddAdmit = 1/m_outputImp;
        m_gndAdmit = cero_doub;

        if( m_inverted ) m_state = !m_state;
        setState( m_state );
    }
    else if( mode == open_col )
    {
        m_vddAdmit = cero_doub;

        if( m_inverted ) m_state = !m_state;
        setState( m_state );
    }
    update();
}

void IoPin::update()
{
    double vddAdmit = m_vddAdmit+m_vddAdmEx;
    double gndAdmit = m_gndAdmit+m_gndAdmEx;
    double Rth  = 1/(vddAdmit+gndAdmit);

    m_voltOut = m_voltHigh*vddAdmit*Rth;

    IoPin::setImp( Rth );
}

void IoPin::setState( bool out, bool st ) // Set Output to Hight or Low
{
    if( m_inverted ) m_state = !out;
    else             m_state =  out;

    if( m_stateZ ) return;

    if( m_pinMode == open_col )
    {
        if( m_state ) m_gndAdmit = 1/m_openImp;
        else          m_gndAdmit = 1/m_outputImp;

        if( st ) update();
        ePin::setPinState( m_state? out_open:out_low ); // Z-Low colors
    }
    else
    {
        if( m_state ) m_voltOut = m_voltHigh;
        else          m_voltOut = m_voltLow;

        if( st ) stampOutput();
        ePin::setPinState( m_state? out_high:out_low ); // High-Low colors
    }
}

void IoPin::setStateZ( bool z )
{
    m_stateZ = z;
    if( z )
    {
        m_voltOut = m_voltLow;
        setImp( m_openImp );
        ePin::setPinState( out_open );
    }
    else
    {
        pinMode_t pm = m_pinMode; // Force pinMode
        m_pinMode = undef_mode;
        setPinMode( pm );
    }
}

void IoPin::setVoltHigh( double v )
{
    m_voltHigh = v;
    /// if( m_state ) m_voltOut = v;
}

void IoPin::setVoltLow( double v )
{
    m_voltLow = v;
    /// if( !m_state ) m_voltOut = v;
}

void IoPin::setImp( double imp )
{
    m_imp = imp;
    m_admit = 1/m_imp;
    stampAll();
}

void IoPin::setInputImp( double imp )
{
    m_inputImp = imp;
    if( m_pinMode == input ) m_gndAdmit = 1/m_inputImp;
}

void IoPin::setOutputImp( double imp )
{
    m_outputImp = imp;
    if( m_pinMode == output ) m_vddAdmit = 1/m_outputImp;
}

void IoPin::setInverted( bool inverted )
{
    if( inverted == m_inverted ) return;

    if( inverted ) setState( !m_state );
    else           setState( m_state );

    m_inverted = inverted;
    ePin::setInverted( inverted );
}

double IoPin::getVolt()
{
    if( ePin::isConnected() ) return ePin::getVolt();
    else                           return m_voltOut;
}

void IoPin::controlPin( bool ctrl )
{
    if( ctrl == m_extCtrl ) return;

    if( ctrl && !m_extCtrl ) // Someone is getting control
    {
        m_oldPinMode = m_pinMode; // Save old Pin Mode to restore later
    }
    else                     // External control is being released
    {
        setPinMode( m_oldPinMode ); // Set Previous Pin MOde
    }
    m_extCtrl = ctrl;
}
