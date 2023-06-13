/***************************************************************************
 *   Copyright (C) 2020 by Santiago González                               *
 *                                                                         *
 ***( see copyright.txt file at root folder )*******************************/

#include "mcupin.h"
#include "mcuport.h"
#include "datautils.h"
#include "mcuinterrupts.h"
#include "simulator.h"
#include "circuit.h"

McuPin::McuPin( McuPort* port, int i, QString id, Component* mcu )
      : IoPin( 0, QPoint(0,0), id, 0, mcu )
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
    m_changeCB = true;  // Always call VoltChanged()
    m_pinState = input_low;

    setOutHighV( 5 );
    initialize();
}
McuPin::~McuPin() {}

void McuPin::initialize()
{
    m_outCtrl = false;
    m_dirCtrl = false;
    m_isAnalog = false;
    //m_portState = false;

    IoPin::initialize();
}

void McuPin::stamp()
{
    IoPin::stamp();
    if( !m_dirCtrl ) setDirection( m_outMask );
    setPullup( m_puMask );
    if( !m_outCtrl && m_outMask ) IoPin::setOutState( true );
    update();
}

void McuPin::voltChanged()
{
    bool oldState = m_inpState;
    bool newState = IoPin::getInpState();

    if( m_extInt )
    {
        bool raise = true;
        bool trigger = false;
        switch( m_extIntTrigger ) { // Trigger pinLow without pin change at simulation start
            case pinLow:     trigger = (Simulator::self()->circTime() == 0); raise = !newState;
            case pinChange:  trigger |= (oldState != newState); break;
            case pinFalling: trigger = (oldState && !newState); break;
            case pinRising:  trigger = (!oldState && newState); break;
            case pinDisabled:                                   break;
        }
        if( trigger ) m_extInt->raise( raise );
    }
    if( oldState != newState )
    {
        uint8_t val = newState ? m_pinMask : 0;
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
{ if( m_outCtrl ) IoPin::setOutState( state ); }

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

void McuPin::setPullup( bool up )
{
    IoPin::setPullup( up );
    if( up == m_puMask ) return;
    m_puMask = up;

    if( m_pinMode < output && !this->isConnected() ) // Input Pin not connected
    {
        m_inpState = up;
        uint8_t val = up ? m_pinMask : 0;
        m_port->pinChanged( m_pinMask, val );
        if     ( m_pinMode == openCo ) setPinState( up? open_high  : open_low  ); // High : Low colors
        else if( m_pinMode == input  ) setPinState( up? input_high : input_low ); // High : Low colors
    }
}

void McuPin::setExtraSource( double vddAdmit, double gndAdmit ) // Comparator Vref out to Pin for example
{
    m_vddAdmEx = vddAdmit;
    m_gndAdmEx = gndAdmit;
    updtState();
}

void McuPin::ConfExtInt( uint8_t bits )
{
    if( !m_extInt ) return;
    m_extIntTrigger = (extIntTrig_t)getRegBitsVal( bits, m_extIntBits );
    m_extInt->setAutoClear( m_extIntTrigger != pinLow );
}

void McuPin::setExtInt( uint mode )
{
    m_extIntTrigger = (extIntTrig_t)mode;
}

// ---- Script Engine -------------------
#include "angelscript.h"
void McuPin::registerScript( asIScriptEngine* engine )
{
    engine->RegisterObjectType("McuPin", 0, asOBJ_REF | asOBJ_NOCOUNT );

    engine->RegisterObjectMethod("McuPin", "void setDirection( bool o )"
                                   , asMETHODPR( McuPin, setDirection, (bool), void)
                                   , asCALL_THISCALL );

    engine->RegisterObjectMethod("McuPin", "void setPortState( bool s )"
                                   , asMETHODPR( McuPin, setPortState, (bool), void)
                                   , asCALL_THISCALL );

    engine->RegisterObjectMethod("McuPin", "void controlPin( bool outCtrl, bool dirCtrl )"
                                   , asMETHODPR( McuPin, controlPin, (bool,bool), void)
                                   , asCALL_THISCALL );

    engine->RegisterObjectMethod("McuPin", "void setExtInt( uint mode )"
                                   , asMETHODPR( McuPin, setExtInt, (uint), void)
                                   , asCALL_THISCALL );

    // IoPin ------------
    engine->RegisterObjectMethod("McuPin", "void setPinMode(uint m)"
                                   , asMETHODPR( McuPin, setPinMode, (uint), void)
                                   , asCALL_THISCALL );

    engine->RegisterObjectMethod("McuPin", "bool getInpState()"
                                   , asMETHODPR( McuPin, getInpState, (), bool)
                                   , asCALL_THISCALL );

    engine->RegisterObjectMethod("McuPin", "void setOutState(bool s)"
                                   , asMETHODPR( McuPin, setOutState, (bool), void)
                                   , asCALL_THISCALL );

    engine->RegisterObjectMethod("McuPin", "double getVoltage()"
                                   , asMETHODPR( McuPin, getVoltage, (), double)
                                   , asCALL_THISCALL );

    engine->RegisterObjectMethod("McuPin", "void setVoltage(double v)"
                                   , asMETHODPR( McuPin, setVoltage, (double), void)
                                   , asCALL_THISCALL );

    engine->RegisterObjectMethod("McuPin", "void changeCallBack(eElement@ p, bool s)"
                                   , asMETHODPR( McuPin, changeCallBack, (eElement*, bool), void)
                                   , asCALL_THISCALL );
}
