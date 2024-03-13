/***************************************************************************
 *   Copyright (C) 2016 by Santiago González                               *
 *                                                                         *
 ***( see copyright.txt file at root folder )*******************************/

#include "flipflopbase.h"
#include "simulator.h"
#include "circuit.h"
#include "iopin.h"

#include "boolprop.h"
#include "stringprop.h"

#define tr(str) simulideTr("FlipFlopBase",str)

FlipFlopBase::FlipFlopBase( QString type, QString id )
            : LogicComponent( type, id )
{
    m_dataPins = 0;
    m_useRS = true;
    m_srInv = false;

    addPropGroup( { tr("Main"), {
        new BoolProp<FlipFlopBase>("UseRS", tr("Use Set/Reset Pins"),""
                                  , this, &FlipFlopBase::pinsRS, &FlipFlopBase::usePinsRS, propNoCopy ),

        new BoolProp<FlipFlopBase>("Reset_Inverted", tr("Set/Reset Inverted"),""
                                  , this, &FlipFlopBase::srInv, &FlipFlopBase::setSrInv, propNoCopy ),

        new BoolProp<FlipFlopBase>("Clock_Inverted", tr("Clock Inverted"),""
                                  , this, &FlipFlopBase::clockInv, &FlipFlopBase::setClockInv, propNoCopy ),

        new StrProp <FlipFlopBase>("Trigger", tr("Trigger Type"),""
                                  , this, &FlipFlopBase::triggerStr,&FlipFlopBase::setTriggerStr, propNoCopy,"enum" ),
    }, groupNoCopy } );

    addPropGroup( { tr("Electric"), IoComponent::inputProps()+IoComponent::outputProps(),0 } );
    addPropGroup( { tr("Timing")  , IoComponent::edgeProps(),0 } );
}
FlipFlopBase::~FlipFlopBase(){}

void FlipFlopBase::stamp()
{
    m_Q0 = (std::rand()%2);
    m_setPin->changeCallBack( this );
    m_resetPin->changeCallBack( this );

    if( m_trigger != Clock ) // J K or D
    { for( int i=0; i<m_dataPins; i++ ) m_inPin[i]->changeCallBack( this ); }

    LogicComponent::stamp();
    m_outPin[0]->setOutState( m_Q0 );
    m_outPin[1]->setOutState( !m_Q0 );
    m_nextOutVal = m_outValue = m_Q0? 1:2;
}

void FlipFlopBase::updateStep()
{
    if( !m_changed ) return;
    m_changed = false;

    m_setPin->setInverted( m_srInv );   // Set
    m_resetPin->setInverted( m_srInv ); // Reset

    voltChanged();
    Circuit::self()->update();
}

void FlipFlopBase::voltChanged()
{
    updateClock();  // Update Clk to don't miss any clock changes

    bool set   = sPinState();
    bool reset = rPinState();

    if( set || reset) m_nextOutVal = (set? 1:0) + (reset? 2:0);
    else if( m_clkState == Clock_Allow ) calcOutput();

    scheduleOutPuts( this );
}

void FlipFlopBase::setSrInv( bool inv )
{
    if( m_srInv == inv ) return;
    m_srInv = inv;

    m_changed = true;
    if( !Simulator::self()->isRunning() ) updateStep();
}

void FlipFlopBase::usePinsRS( bool rs )
{
    m_useRS = rs;
    if( !rs ){
        m_setPin->removeConnector();
        m_resetPin->removeConnector();
    }
    m_setPin->setVisible( rs );
    m_resetPin->setVisible( rs );
}

bool FlipFlopBase::sPinState()
{
    if( m_useRS ) return m_setPin->getInpState();
    return false;
}

bool FlipFlopBase::rPinState()
{
    if( m_useRS ) return m_resetPin->getInpState();
    return false;
}
