/***************************************************************************
 *   Copyright (C) 2021 by Santiago González                               *
 *                                                                         *
 ***( see copyright.txt file at root folder )*******************************/

#include <QtMath>
#include <QMenu>

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
    m_userInvert = true;

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
    animate( Circuit::self()->animate() );
}
IoPin::~IoPin(){}

void IoPin::initialize()
{
    m_step = 0;
    m_steps = Simulator::self()->slopeSteps();

    m_inpState  = false;
    m_outState  = false;
    m_nextState = false;
}

void IoPin::stamp()
{
    if( m_skipStamp ) return;

    ePin::setEnodeComp( &m_gndEnode );
    ePin::createCurrent();
    setPinMode( m_pinMode );
    stampAll();
    updateStep();
}

void IoPin::updateStep()
{
    if( m_unused ) return;

    if( m_stateZ ) m_pinState = undef_state;
    else{
        bool state = getInpState();
        if( m_inverted ) state = !state;
        switch( m_pinMode ) // Pin colors in animation
        {
            case undef_mode: m_pinState = undef_state; break;
            case input:  m_pinState = state? input_high : input_low; break;
            case openCo:{
                    pinState_t low = m_outState ? driven_low : open_low;
                    m_pinState = state? open_high  : low ;
                }break;
            case output:
            case source: m_pinState = state? out_high : out_low; break;
        }
    }
    update();
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

void IoPin::scheduleState( bool state, uint64_t time )
{
    if( m_nextState == state ) return;
    m_nextState = state;

    if( m_step )
    {
        Simulator::self()->cancelEvents( this );
        m_step = m_steps-m_step;
    }
    if( time )
    {
        Simulator::self()->cancelEvents( this );
        Simulator::self()->addEvent( time, this );
    }
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

    if( !Simulator::self()->isRunning() ) IoPin::updateStep();
}

/*void IoPin::updtState()
{
    //if( m_pinMode > openCo ) return;

    double vddAdmit = m_vddAdmit + m_vddAdmEx;
    double gndAdmit = m_gndAdmit + m_gndAdmEx;
    m_admit         = vddAdmit+gndAdmit;

    m_outVolt = m_outHighV*vddAdmit/m_admit;
    stampAll();
}*/

bool IoPin::getInpState()
{
    double volt = getVoltage();

    if     ( volt > m_inpHighV ) m_inpState = true;
    else if( volt < m_inpLowV )  m_inpState = false;

    return m_inverted ? !m_inpState : m_inpState;
}

void IoPin::setOutState( bool high ) // Set Output to Hight or Low
{
    m_outState = m_nextState = high;
    if( m_pinMode < openCo || m_stateZ ) return;

    if( m_inverted ) high = !high;

    if( m_pinMode == openCo )
    {
        m_gndAdmit = high ? 1/1e8 : 1/m_outputImp;
        updtState();
    }else{
        m_outVolt = high ? m_outHighV : m_outLowV;
        stampVolt( m_outVolt );
}   }

void IoPin::setStateZ( bool z )
{
    m_stateZ = z;
    if( z ){
        m_outVolt = m_outLowV;
        setImpedance( m_openImp );
    }else {
        pinMode_t pm = m_pinMode; // Force old pinMode
        m_pinMode = undef_mode;
        setPinMode( pm );
}   }

double IoPin::getVoltage()
{
    if     ( m_enode )           return m_enode->getVolt();
    else if( m_pinMode > input ) return m_outVolt;
    else{
        double vddAdmit = m_vddAdmit + m_vddAdmEx;
        double voltage = m_outHighV*vddAdmit/m_admit;
        return voltage;
    }
    return 0;
}

void IoPin::setPullup( bool up )
{
    if( up ) m_vddAdmEx = 1/1e5; // Activate pullup
    else     m_vddAdmEx = 0;     // Deactivate pullup

    if( m_pinMode < output ) updtState();
}

void IoPin::setImpedance( double imp )
{
    m_admit = 1/imp;
    stampAll();
}

void IoPin::setInputImp( double imp )
{
    m_inputImp = imp;
    if( m_pinMode == input )
    {
        //m_gndAdmit = 1/m_inputImp;
        //updtState();
        m_admit = 1/m_inputImp;
        ePin::stampAdmitance( m_admit );
    }
}

void IoPin::setOutputImp( double imp )
{
    m_outputImp = imp;
    if( m_pinMode == output )
    {
        //m_vddAdmit = 1/m_outputImp;
        //updtState();
        m_admit = 1/m_outputImp;
        ePin::stampAdmitance( m_admit );
    }
}

void IoPin::setInverted( bool invert )
{
    if( invert == m_inverted ) return;
    m_inverted = invert;

    if( m_pinMode > input ) setOutState( m_outState );
    else                    ePin::stampAdmitance( m_admit );
    update();
}

void IoPin::stampAll()
{
    ePin::stampAdmitance( m_admit );
    stampVolt( m_outVolt );
}

// ---- Script Engine -------------------
#include "angelscript.h"
QStringList IoPin::registerScript( asIScriptEngine* engine )
{
    QStringList memberList;
    engine->RegisterObjectType("IoPin", 0, asOBJ_REF | asOBJ_NOCOUNT );

    memberList << "setPinMode( uint mode )";
    engine->RegisterObjectMethod("IoPin", "void setPinMode(uint m)"
                                   , asMETHODPR( IoPin, setPinMode, (uint), void)
                                   , asCALL_THISCALL );

    memberList << "getInpState()";
    engine->RegisterObjectMethod("IoPin", "bool getInpState()"
                                   , asMETHODPR( IoPin, getInpState, (), bool)
                                   , asCALL_THISCALL );

    memberList << "void setOutState( bool state )";
    engine->RegisterObjectMethod("IoPin", "void setOutState(bool s)"
                                   , asMETHODPR( IoPin, setOutState, (bool), void)
                                   , asCALL_THISCALL );

    memberList << "setStateZ( bool state )";
    engine->RegisterObjectMethod("IoPin", "void setStateZ( bool z )"
                                   , asMETHODPR( IoPin, setStateZ, (bool), void)
                                   , asCALL_THISCALL );

    memberList << "setOutStatFast( bool state )";
    engine->RegisterObjectMethod("IoPin", "void setOutStatFast(bool s)"
                                   , asMETHODPR( IoPin, setOutStatFast, (bool), void)
                                   , asCALL_THISCALL );

    memberList << "scheduleState( bool state, uint64 time )";
    engine->RegisterObjectMethod("IoPin", "void scheduleState( bool state, uint64 time )"
                                   , asMETHODPR( IoPin, scheduleState, (bool,uint64_t), void)
                                   , asCALL_THISCALL );

    memberList << "getVoltage()";
    engine->RegisterObjectMethod("IoPin", "double getVoltage()"
                                   , asMETHODPR( IoPin, getVoltage, (), double)
                                   , asCALL_THISCALL );

    memberList << "setVoltage( double voltage )";
    engine->RegisterObjectMethod("IoPin", "void setVoltage(double v)"
                                   , asMETHODPR( IoPin, setVoltage, (double), void)
                                   , asCALL_THISCALL );

    memberList << "setImpedance( double impedance )";
    engine->RegisterObjectMethod("IoPin", "void setImpedance( double imp )"
                                   , asMETHODPR( IoPin, setImpedance, (double), void)
                                   , asCALL_THISCALL );

    memberList << "changeCallBack( eElement@ e, bool call )";
    engine->RegisterObjectMethod("IoPin", "void changeCallBack(eElement@ p, bool s)"
                                   , asMETHODPR( IoPin, changeCallBack, (eElement*, bool), void)
                                   , asCALL_THISCALL );

    return memberList;
}
