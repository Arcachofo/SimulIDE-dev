/***************************************************************************
 *   Copyright (C) 2010 by Santiago González                               *
 *                                                                         *
 ***( see copyright.txt file at root folder )*******************************/

#include "logiccomponent.h"
#include "circuitwidget.h"
#include "simulator.h"
#include "connector.h"
#include "circuit.h"
#include "iopin.h"

#define tr(str) simulideTr("LogicComponent",str)

const QString LogicComponent::m_triggerList = "None,Clock,Enable;"+tr("None")+","+tr("Clock")+","+tr("Enable");

LogicComponent::LogicComponent( QString type, QString id )
              : IoComponent( type, id )
              , eClockedDevice( id )
{
    m_oePin     = nullptr;
    m_tristate  = false;
    m_outEnable = true;
}
LogicComponent::~LogicComponent(){}

void LogicComponent::stamp()
{
    IoComponent::initState();
    eClockedDevice::stamp();
    if( m_oePin ) m_oePin->changeCallBack( this );
    m_outEnable = true;
}

std::vector<Pin*> LogicComponent::getPins()
{
    std::vector<Pin*> pins = IoComponent::getPins();
    if( m_oePin  ) pins.emplace_back( m_oePin );
    if( m_clkPin ) pins.emplace_back( m_clkPin );
    return pins;
}

void LogicComponent::remove()
{
    if( m_oePin ) m_oePin->removeConnector();
    eClockedDevice::remove();
    IoComponent::remove();
}

void LogicComponent::setOePin( IoPin* pin )
{
    m_oePin = pin;
    pin->setInverted( true );
    m_oePin->setLabelText("OE ");
}

bool LogicComponent::outputEnabled()
{
    if( !m_oePin ) return true;

    m_outEnable = m_oePin->getInpState();
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
    if( !m_oePin ) return;

    if( !t ) m_oePin->removeConnector();

    m_oePin->setVisible( t );
    m_tristate = t;
    updateOutEnabled();
}

void LogicComponent::setTriggerStr( QString t )
{
    m_triggerStr = t;

    trigger_t trigger = Clock;
    if     ( t == "None"  ) trigger = None;
    else if( t == "Clock" ) trigger = Clock;
    else if( t == "Enable") trigger = InEnable;

    setTrigger( trigger );

    if( m_showVal && (m_showProperty == "Trigger") )
        setValLabelText( t );
}

void LogicComponent::enableOutputs( bool en )
{
    for( uint i=0; i<m_outPin.size(); ++i ) m_outPin[i]->setStateZ( !en );
}

void LogicComponent::setInpHighV( double volt )
{
    IoComponent::setInpHighV( volt );
    if( m_clkPin) m_clkPin->setInputHighV( m_inHighV );
}

void LogicComponent::setInpLowV( double volt )
{
    IoComponent::setInpLowV( volt );
    if( m_clkPin) m_clkPin->setInputLowV( m_inLowV );
}

void LogicComponent::setInputImp( double imp )
{
    IoComponent::setInputImp( imp );
    Simulator::self()->pauseSim();
    if( m_clkPin) m_clkPin->setInputImp( m_inImp );
    Simulator::self()->resumeSim();
}
