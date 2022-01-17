/***************************************************************************
 *   Copyright (C) 2010 by santiago González                               *
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

#include "logiccomponent.h"
#include "circuitwidget.h"
#include "simulator.h"
#include "connector.h"
#include "circuit.h"
#include "iopin.h"

QStringList LogicComponent::m_triggers = {"None","Clock","Enable"};

LogicComponent::LogicComponent( QObject* parent, QString type, QString id )
              : IoComponent( parent, type, id )
              , eClockedDevice( id )
{
    m_oePin   = NULL;
    m_tristate = false;
    m_openCol  = false;
    m_outEnable = true;
}
LogicComponent::~LogicComponent(){}

void LogicComponent::updateStep()
{
    if( !Circuit::self()->animate( ) ) return;
    IoComponent::updateStep();
    if( m_oePin ) m_oePin->updateStep();
    if( m_clkPin) m_clkPin->updateStep();
}

void LogicComponent::initialize()
{ IoComponent::initState(); eClockedDevice::initialize(); }

void LogicComponent::stamp()
{
    if( m_oePin ) m_oePin->changeCallBack( this );
    eClockedDevice::stamp();
}

void LogicComponent::remove()
{
    if( m_oePin ) m_oePin->removeConnector();
    eClockedDevice::remove();
    IoComponent::remove();
}

void LogicComponent::setOePin( IoPin* pin )
{ pin->setInverted( true ); m_oePin = pin; }

bool LogicComponent::outputEnabled()
{
    if( !m_oePin ) return true;

    double volt = m_oePin->getVolt();
    if     ( volt > m_inHighV ) m_outEnable = false;   // Active Low
    else if( volt < m_inLowV )  m_outEnable = true;

    m_oePin->setPinState( m_outEnable? input_low:input_high ); // Low-High colors
    return m_outEnable;
}

void LogicComponent::updateOutEnabled()
{
    if( !m_oePin ) return;

    bool outEnPrev = m_outEnable;
    bool outEn = outputEnabled();              // Refresh m_outEnable
    if( outEnPrev != outEn ) enableOutputs( outEn );
}

void LogicComponent::setTristate( bool t )  // Activate or deactivate OE Pin
{
    if( !t )
    {
        m_oePin->removeConnector();
        m_oePin->setLabelText( "" );
    }
    else m_oePin->setLabelText( "OE " );

    m_oePin->setVisible( t );
    m_tristate = t;
    updateOutEnabled();
}

void LogicComponent::setTriggerStr( QString t )
{
    bool ok = false;
    int index = t.toInt( &ok );
    if( !ok ) index = m_triggers.indexOf( t );
    setTrigger( (trigger_t)index );
}

void LogicComponent::enableOutputs( bool en )
{
    for( uint i=0; i<m_outPin.size(); ++i ) m_outPin[i]->setStateZ( !en );
    Simulator::self()->addEvent( 1, NULL );
}

void LogicComponent::setInputHighV( double volt )
{
    Simulator::self()->pauseSim();
    IoComponent::setInputHighV( volt );
    if( m_clkPin) m_clkPin->setInputHighV( volt );
    Simulator::self()->resumeSim();
}

void LogicComponent::setInputLowV( double volt )
{
    Simulator::self()->pauseSim();
    IoComponent::setInputLowV( volt );
    if( m_clkPin) m_clkPin->setInputLowV( volt );
    Simulator::self()->resumeSim();
}

void LogicComponent::setInputImp( double imp )
{
    Simulator::self()->pauseSim();
    IoComponent::setInputImp( imp );
    if( m_clkPin) m_clkPin->setInputImp( imp );
    Simulator::self()->resumeSim();
}
