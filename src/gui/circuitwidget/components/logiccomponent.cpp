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

LogicComponent::LogicComponent( QObject* parent, QString type, QString id )
              : IoComponent( parent, type, id )
              , eClockedDevice( id )
{
    m_oePin     = NULL;
    m_tristate  = false;
    m_openCol   = false;
    m_outEnable = true;

    m_enumUids = QStringList()
        << "None"
        << "Clock"
        << "Enable";

    m_enumNames = QStringList()
        << tr("None")
        << tr("Clock")
        << tr("Enable");
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
{ pin->setInverted( true ); m_oePin = pin; }

bool LogicComponent::outputEnabled()
{
    if( !m_oePin ) return true;

    double volt = m_oePin->getVoltage();
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
    int index = getEnumIndex( t );
    setTrigger( (trigger_t)index );

    if( m_showVal && (m_showProperty == "Trigger") )
        setValLabelText( m_enumNames.at( index ) );
}

void LogicComponent::enableOutputs( bool en )
{
    for( uint i=0; i<m_outPin.size(); ++i ) m_outPin[i]->setStateZ( !en );
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
