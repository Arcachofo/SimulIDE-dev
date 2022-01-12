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
#include "e-node.h"
#include "simulator.h"

IoPin::IoPin( int angle, const QPoint pos, QString id, int index, Component* parent, pinMode_t mode )
     : Pin( angle, pos, id, index, parent )
     , eElement( id )
{
    m_scrEnode = new eNode( id+"scr" );
    m_scrEnode->setNodeNumber(0);
    Simulator::self()->remFromEnodeList( m_scrEnode, /*delete=*/ false );

    m_outState    = false;
    m_stateZ   = false;

    m_inpHighV = 2.5;
    m_inpLowV  = 2.5;
    m_outHighV = cero_doub;
    m_outLowV  = cero_doub;
    m_outVolt  = cero_doub;

    m_vddAdmit = 0;
    m_gndAdmit = cero_doub;
    m_vddAdmEx = 0;
    m_gndAdmEx = 0;

    m_inputImp = high_imp;
    m_openImp  = 100/cero_doub;
    m_outputImp = 40;
    m_admit = 1/cero_doub;

    m_pinMode = undef_mode;
    setPinMode( mode );
}
IoPin::~IoPin(){ delete m_scrEnode; }

void IoPin::initialize()
{
    m_outCtrl = false;
    m_dirCtrl = false;
    m_inpState = false;
    m_outState = false;
    ePin::setEnodeComp( m_scrEnode );
    setPinMode( m_pinMode );
}

void IoPin::setPinMode( pinMode_t mode )
{
    if( m_pinMode == mode ) return;
    m_pinMode = mode;

    switch( mode )
    {
        case undef_mode:
            return;
        case input:
            m_vddAdmit = 0;
            m_gndAdmit = 1/m_inputImp;
            setPinState( input_low );
            break;
        case output:
            m_vddAdmit = 1/m_outputImp;
            m_gndAdmit = cero_doub;
            break;
        case openCo:
            m_vddAdmit = cero_doub;
            break;
        case source:
            m_vddAdmit = 1/cero_doub;
            m_gndAdmit = cero_doub;
            setPinState( out_high );
            break;
    }
    updtState();
    if( m_pinMode >= output ) IoPin::setOutState( m_outState );
    update();
}

void IoPin::updtState()
{
    double vddAdmit = m_vddAdmit+m_vddAdmEx;
    double gndAdmit = m_gndAdmit+m_gndAdmEx;
    double Rth  = 1/(vddAdmit+gndAdmit);

    m_outVolt = m_outHighV*vddAdmit*Rth;
    IoPin::setImp( Rth );
}

bool IoPin::getInpState()
{
    double volt = getVolt();

    if     ( volt > m_inpHighV ) m_inpState = true;
    else if( volt < m_inpLowV )  m_inpState = false;

    if     ( m_pinMode == openCo ) setPinState( m_inpState? open_high:driven_low ); // High : Low colors
    else if( m_pinMode == input )  setPinState( m_inpState? input_high:input_low ); // High : Low colors

    return m_inverted ? !m_inpState : m_inpState;
}

void IoPin::setOutState( bool out ) // Set Output to Hight or Low
{
    m_outState = out;
    if( m_inverted ) out = !out;

    if( m_stateZ ) return;

    if( m_pinMode == openCo )
    {
        m_gndAdmit = out ? 1/m_openImp : 1/m_outputImp;
        updtState();
        setPinState( out? open_high:open_low ); // Z-Low colors
    }else{
        m_outVolt = out ? m_outHighV : m_outLowV;
        stampOutput();
        setPinState( out? out_high:out_low ); // High-Low colors
}   }

void IoPin::setStateZ( bool z )
{
    m_stateZ = z;
    if( z ){
        m_outVolt = m_outLowV;
        setImp( m_openImp );
        setPinState( open_high );
    }else {
        pinMode_t pm = m_pinMode; // Force pinMode
        m_pinMode = undef_mode;
        setPinMode( pm );
}   }

void IoPin::setPullup( bool up )
{
    if( up ) m_vddAdmEx = 1/1e5; // Activate pullup
    else     m_vddAdmEx = 0;     // Deactivate pullup

    updtState();
}

void IoPin::setImp( double imp )
{
    m_admit = 1/imp;
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
    m_inverted = inverted;

    if( m_pinMode >= output ) setOutState( m_outState );
    update();
}

void IoPin::controlPin( bool outCtrl, bool dirCtrl )
{
    if( dirCtrl && !m_dirCtrl )      // Someone is getting control
    {
        m_oldPinMode = m_pinMode;  // Save old Pin Mode to restore later
    }
    else if( !dirCtrl && m_dirCtrl ) // External control is being released
    {
        setPinMode( m_oldPinMode ); // Set Previous Pin Direction
    }
    m_dirCtrl = dirCtrl;

    if( outCtrl && !m_outCtrl )      // Someone is getting control
    {
        m_oldState = m_outState;  // Save old Pin State to restore later
    }
    else if( !outCtrl && m_outCtrl ) // External control is being released
    {
        if( m_pinMode > input ) setOutState( m_oldState ); // Set Previous Pin State
        else                    m_outState = m_oldState;
    }
    m_outCtrl = outCtrl;
}

void IoPin::stampAll()
{
    ePin::stampAdmitance( m_admit );
    stampOutput();
}

void IoPin::stampOutput()
{
    m_scrEnode->setVolt( m_outVolt );
    ePin::stampCurrent( m_outVolt*m_admit );
}
