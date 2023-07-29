/***************************************************************************
 *   Copyright (C) 2021 by Santiago González                               *
 *                                                                         *
 ***( see copyright.txt file at root folder )*******************************/

#include <QtMath>

#include "iopin.h"
#include "simulator.h"
#include "circuit.h"

eNode IoPin::m_gndEnode("");

IoPin::IoPin( int angle, const QPoint pos, QString id, int index, Component* parent, pinMode_t mode )
     : Pin( angle, pos, id, index, parent )
     , eElement( id )
{
    m_outState = false;
    m_stateZ   = false;
    m_skipStamp = false;

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

    m_steps = 0;
    m_timeRis = 3750; // picoseconds
    m_timeFal = 3750;

    m_pinMode = undef_mode;
    setPinMode( mode );
}
IoPin::~IoPin(){}

void IoPin::initialize()
{
    m_step = 0;
    m_steps = Simulator::self()->slopeSteps();
    m_inpState = false;
    m_outState = false;
    m_nextState = false;
    updateStep();
}

void IoPin::stamp()
{
    if( m_skipStamp ) return;

    ePin::setEnodeComp( &m_gndEnode );
    ePin::createCurrent();
    setPinMode( m_pinMode );
    stampAll();
}

void IoPin::runEvent()
{
    if( m_step == m_steps )
    {
        m_step = 0;
        IoPin::setOutState( m_nextState );
    }else{
        bool nextState = m_inverted ? !m_nextState : m_nextState;

        if( m_pinMode == openCo )
        {
            double step = nextState ? m_step : m_steps-m_step;
            double delta =  qPow( 1e4*step/m_steps, 2 );
            m_gndAdmit = 1/(m_outputImp+delta);
            updtState();
        }else{
            double delta = m_step;
            if( m_step == 0 ) delta = 1e-5;
            if( nextState ) stampVolt( m_outLowV+delta*(m_outHighV-m_outLowV)/m_steps ); // L to H
            else            stampVolt( m_outHighV-delta*(m_outHighV-m_outLowV)/m_steps );// H to L
        }
        int time = nextState ? m_timeRis : m_timeFal;
        Simulator::self()->addEvent( time/m_steps, this );
        m_step++;
    }
}

void IoPin::sheduleState( bool state, uint64_t time )
{
    if( m_nextState == state ) return;
    m_nextState = state;

    if( m_step )
    {
        Simulator::self()->cancelEvents( this );
        m_step = m_steps-m_step;
    }
    if     ( time )    Simulator::self()->addEvent( time, this );
    else if( m_steps ) IoPin::runEvent();
    else               IoPin::setOutState( m_nextState );
}

void IoPin::startLH()
{
    m_step = 0;
    stampVolt( m_outLowV+(m_outLowV+m_outHighV)/100 );

}
void IoPin::startHL()
{
    m_step = 0;
    stampVolt( m_outHighV-(m_outLowV+m_outHighV)/100 );
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
            m_admit = 1/m_outputImp;
            ePin::stampAdmitance( m_admit );
            break;
        case openCo:
            m_vddAdmit = cero_doub;
            break;
        case source:
            m_vddAdmit = 1/cero_doub;
            m_gndAdmit = cero_doub;
            m_outState = true;
            break;
    }
    if( m_pinMode > input ) IoPin::setOutState( m_outState );
    else                    updtState();
}

void IoPin::updtState()
{
    if( m_pinMode > openCo ) return;

    double vddAdmit = m_vddAdmit + m_vddAdmEx;
    double gndAdmit = m_gndAdmit + m_gndAdmEx;
    double Rth      = 1/(vddAdmit+gndAdmit);

    m_outVolt = m_outHighV*vddAdmit*Rth;
    IoPin::setImp( Rth );
}

bool IoPin::getInpState()
{
    double volt = getVoltage();

    if     ( volt > m_inpHighV ) m_inpState = true;
    else if( volt < m_inpLowV )  m_inpState = false;

    if( m_pinMode == openCo )
    {
        pinState_t low = m_outState ? driven_low : open_low;
        setPinState( m_inpState? open_high  : low ); // High : Low colors
    }
    else if( m_pinMode == input  ) setPinState( m_inpState? input_high : input_low  ); // High : Low colors

    return m_inverted ? !m_inpState : m_inpState;
}

void IoPin::setOutState( bool high ) // Set Output to Hight or Low
{
    m_outState = m_nextState = high;
    if( m_pinMode < openCo || m_stateZ ) return;

    if( m_inverted ) high = !high;

    if( m_pinMode == openCo )
    {
        if( high ){
            m_gndAdmit = 1/1e8;
            setPinState( open_high ); // Z-high colors
         }else{
            m_gndAdmit = 1/m_outputImp;
            setPinState( open_low );  // Z-Low colors
        }
        updtState();
    }else{
        if( high ){
            m_outVolt = m_outHighV;
            setPinState( out_high ); // High colors
        }else{
            m_outVolt = m_outLowV;
            setPinState( out_low ); // Low colors
        }
        stampVolt( m_outVolt );
}   }

void IoPin::setStateZ( bool z )
{
    m_stateZ = z;
    if( z ){
        m_outVolt = m_outLowV;
        setImp( m_openImp );
        setPinState( undef_state );
    }else {
        pinMode_t pm = m_pinMode; // Force old pinMode
        m_pinMode = undef_mode;
        setPinMode( pm );
}   }

void IoPin::setPullup( bool up )
{
    if( up ) m_vddAdmEx = 1/1e5; // Activate pullup
    else     m_vddAdmEx = 0;     // Deactivate pullup

    if( m_pinMode < output ) updtState();
}

void IoPin::setImp( double imp )
{
    m_admit = 1/imp;
    stampAll();
}

void IoPin::setInputImp( double imp )
{
    m_inputImp = imp;
    if( m_pinMode == input )
    {
        m_gndAdmit = 1/m_inputImp;
        updtState();
    }
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

    if( m_pinMode > input ) setOutState( m_outState );
    update();
}

void IoPin::stampAll()
{
    ePin::stampAdmitance( m_admit );
    stampVolt( m_outVolt );
}

// ---- Script Engine -------------------
#include "angelscript.h"
void IoPin::registerScript( asIScriptEngine* engine )
{
    engine->RegisterObjectType("IoPin", 0, asOBJ_REF | asOBJ_NOCOUNT );

    engine->RegisterObjectMethod("IoPin", "void setPinMode(uint m)"
                                   , asMETHODPR( IoPin, setPinMode, (uint), void)
                                   , asCALL_THISCALL );

    engine->RegisterObjectMethod("IoPin", "bool getInpState()"
                                   , asMETHODPR( IoPin, getInpState, (), bool)
                                   , asCALL_THISCALL );

    engine->RegisterObjectMethod("IoPin", "void setOutState(bool s)"
                                   , asMETHODPR( IoPin, setOutState, (bool), void)
                                   , asCALL_THISCALL );

    engine->RegisterObjectMethod("IoPin", "double getVoltage()"
                                   , asMETHODPR( IoPin, getVoltage, (), double)
                                   , asCALL_THISCALL );

    engine->RegisterObjectMethod("IoPin", "void setVoltage(double v)"
                                   , asMETHODPR( IoPin, setVoltage, (double), void)
                                   , asCALL_THISCALL );

    engine->RegisterObjectMethod("IoPin", "void changeCallBack(eElement@ p, bool s)"
                                   , asMETHODPR( IoPin, changeCallBack, (eElement*, bool), void)
                                   , asCALL_THISCALL );
}
