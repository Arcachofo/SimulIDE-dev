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

static const char* LogicComponent_properties[] = {
    QT_TRANSLATE_NOOP("App::Property","Propagation Delay ns"),
    QT_TRANSLATE_NOOP("App::Property","Input High V"),
    QT_TRANSLATE_NOOP("App::Property","Input Low V"),
    QT_TRANSLATE_NOOP("App::Property","Input Imped"),
    QT_TRANSLATE_NOOP("App::Property","Out High V"),
    QT_TRANSLATE_NOOP("App::Property","Out Low V"),
    QT_TRANSLATE_NOOP("App::Property","Out Imped"),
    QT_TRANSLATE_NOOP("App::Property","Inverted"),
    QT_TRANSLATE_NOOP("App::Property","Tristate"),
    QT_TRANSLATE_NOOP("App::Property","Clock Inverted"),
    QT_TRANSLATE_NOOP("App::Property","Reset Inverted"),
    QT_TRANSLATE_NOOP("App::Property","Invert Inputs"),
    QT_TRANSLATE_NOOP("App::Property","S R Inverted"),
    QT_TRANSLATE_NOOP("App::Property","Num Inputs"),
    QT_TRANSLATE_NOOP("App::Property","Num Outputs"),
    QT_TRANSLATE_NOOP("App::Property","Num Bits"),
    QT_TRANSLATE_NOOP("App::Property","Channels"),
    QT_TRANSLATE_NOOP("App::Property","Open Collector"),
    QT_TRANSLATE_NOOP("App::Property","Trigger"),
    QT_TRANSLATE_NOOP("App::Property"," 16 Bits")
};

LogicComponent::LogicComponent( QObject* parent, QString type, QString id )
              : IoComponent( parent, type, id )
              , eClockedDevice()
{
    Q_UNUSED( LogicComponent_properties );

    m_oePin   = NULL;
    m_tristate = false;
    m_openCol  = false;
    m_outEnable = true;
}
LogicComponent::~LogicComponent(){}

void LogicComponent::initState()
{
    IoComponent::initState();
    eClockedDevice::initState();
}

void LogicComponent::stamp( eElement* el )
{
    if( m_oePin ) m_oePin->changeCallBack( el );
    eClockedDevice::stamp( el );
}

void LogicComponent::remove()
{
    if( m_oePin ) m_oePin->removeConnector();
    eClockedDevice::remove();
    IoComponent::remove();
}

void LogicComponent::setOePin( IoPin* pin )
{
    pin->setInverted( true );
    m_oePin = pin;
}

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

    if( outEnPrev != outEn ) setOutputEnabled( outEn );
}

void LogicComponent::setTristate( bool t )
{
    if( !t )
    {
        m_oePin->removeConnector();
        m_oePin->reset();
        m_oePin->setLabelText( "" );
    }
    else m_oePin->setLabelText( "OE " );

    m_oePin->setVisible( t );
    m_tristate = t;

    updateOutEnabled();
}

void LogicComponent::setOutputEnabled( bool enabled )
{
    for( uint i=0; i<m_outPin.size(); ++i ) m_outPin[i]->setStateZ( !enabled );
    Simulator::self()->addEvent( 1, NULL );
}

void LogicComponent::setInputHighV( double volt )
{
    bool pauseSim = Simulator::self()->isRunning();
    if( pauseSim ) Simulator::self()->pauseSim();

    IoComponent::setInputHighV( volt );
    if( m_clockPin) m_clockPin->setInputHighV( volt );

    if( pauseSim ) Simulator::self()->resumeSim();
}

void LogicComponent::setInputLowV( double volt )
{
    bool pauseSim = Simulator::self()->isRunning();
    if( pauseSim ) Simulator::self()->pauseSim();

    IoComponent::setInputLowV( volt );
    if( m_clockPin) m_clockPin->setInputLowV( volt );

    if( pauseSim ) Simulator::self()->resumeSim();
}

void LogicComponent::setInputImp( double imp )
{
    bool pauseSim = Simulator::self()->isRunning();
    if( pauseSim ) Simulator::self()->pauseSim();

    IoComponent::setInputImp( imp );
    if( m_clockPin) m_clockPin->setInputImp( imp );

    if( pauseSim ) Simulator::self()->resumeSim();
}

#include "moc_logiccomponent.cpp"
