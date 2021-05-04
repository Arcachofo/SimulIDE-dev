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

#include "e-logic_device.h"
#include "simulator.h"
#include "circuit.h"

eLogicDevice::eLogicDevice( QString id )
            : eElement( id )
{
    m_numInputs  = 0;
    m_numOutputs = 0;

    m_inputHighV = 2.5;
    m_inputLowV  = 2.5;
    m_outHighV   = 5;
    m_outLowV    = 0;
    m_outValue   = 0;

    m_inputImp = 1e9;
    m_outImp   = 40;

    m_rndPD = false;
    m_invInputs = false;
    m_inverted  = false;
    m_clock     = false;
    m_outEnable = true;

    m_clockSource = 0l;
    m_outEnSource = 0l;

    m_propDelay = 10*1000; // 10 ns
    m_timeLH = 3000;
    m_timeHL = 4000;

    m_etrigger = Trig_Clk;
}
eLogicDevice::~eLogicDevice()
{
    for( eSource* esource: m_output ) delete esource;
    for( eSource* esource: m_input )  delete esource;

    if( m_clockSource ) delete m_clockSource;
    if( m_outEnSource ) delete m_outEnSource;
}

void eLogicDevice::stamp()
{
    // Register for callBack when eNode volt change on clock or OE pins
    if( m_clockSource )
    {
        eNode* enode = m_clockSource->getEpin(0)->getEnode();
        if( enode ) enode->voltChangedCallback( this );
    }
    if( m_outEnSource )
    {
        eNode* enode = m_outEnSource->getEpin(0)->getEnode();
        if( enode ) enode->voltChangedCallback( this );
    }
}

void eLogicDevice::initialize()
{
    if( m_clockSource ) m_clock = false;
    
    for( int i=0; i<m_numOutputs; ++i ) eLogicDevice::setOut( i, false );

    m_outStep = 0;
    m_outValue = 0;
    m_nextOutVal = 0;
}

void eLogicDevice::runEvent()
{
    for( int i=0; i<m_numOutputs; ++i )
    {
        bool state = m_nextOutVal & (1<<i);
        bool oldst = m_outValue   & (1<<i);

        if( state != oldst )
        {
            /*if( m_outStep == 0 )
            {
                eNode* enode =  m_output[i]->getPin()->getEnode();
                if( enode ) enode->saveData();
            }
            else */
                m_output[i]->setState( state, true );
        }
    }
    /*if( m_outStep == 0 )
    {
        m_outStep = 1;
        Simulator::self()->addEvent( m_timeLH*1.25, this );
    }
    else*/
    {
        //m_outStep = 0;
        m_outValue = m_nextOutVal;
    }
}

void eLogicDevice::sheduleOutPuts()
{
    if( m_nextOutVal == m_outValue ) return;

    if( m_rndPD )Simulator::self()->addEvent( m_propDelay+(std::rand()%2), this );
    else         Simulator::self()->addEvent( m_propDelay, this );
}

bool eLogicDevice::outputEnabled()
{
    if( !m_outEnSource ) return true;

    double volt = m_outEnSource->getVolt();

    if     ( volt > m_inputHighV ) m_outEnable = false;   // Active Low
    else if( volt < m_inputLowV )  m_outEnable = true;

    m_outEnSource->getPin()->setPinState( m_outEnable? input_low:input_high ); // Low-High colors

    return m_outEnable;
}

void eLogicDevice::updateOutEnabled()
{
    if( m_outEnSource )
    {
        bool outEnPrev = m_outEnable;
        bool outEn = outputEnabled();              // Refresh m_outEnable

        if( outEnPrev != outEn ) setOutputEnabled( outEn );
    }
}

void eLogicDevice::setOutputEnabled( bool enabled )
{
    for( int i=0; i<m_numOutputs; ++i ) m_output[i]->setStateZ( !enabled );
    Simulator::self()->addEvent( 1, NULL );
}

void eLogicDevice::createClockPin()
{
    ePin* epin = new ePin( m_elmId+"-ePin-clock", 0 );

    createClockeSource( epin );
}

void eLogicDevice::createClockPin( ePin* epin )
{
    epin->setId( m_elmId+"-ePin-clock" );

    createClockeSource( epin );
}

void eLogicDevice::createClockeSource( ePin* epin )
{
    m_clockSource = new eSource( m_elmId+"-eSource-clock", epin, input );
    m_clockSource->setInputImp( m_inputImp );
}

void eLogicDevice::createOutEnablePin()
{
    ePin* epin = new ePin( m_elmId+"-ePin-outEnable", 0 );
    createOutEnableeSource( epin );
}

void eLogicDevice::createOutEnablePin( ePin* epin )
{
    epin->setId( m_elmId+"-ePin-outEnable" );
    createOutEnableeSource( epin );
}

void eLogicDevice::createOutEnableeSource( ePin* epin )
{
    m_outEnSource = new eSource( m_elmId+"-eSource-outEnable", epin, input );
    m_outEnSource->setInputImp( m_inputImp );
    epin->setInverted( true );
}

void eLogicDevice::createPins( int inputs, int outputs )
{
    setNumInps( inputs );
    setNumOuts( outputs );
}

void eLogicDevice::setInput( int n, eSource* input )
{
    if( m_numInputs < (n+1) )
    {
        m_numInputs = n+1;
        m_input.resize( m_numInputs );
        m_inputState.resize( m_numInputs );
    }
    m_input[n] = input;
}

void eLogicDevice::createInput( ePin* epin )
{
    int totalInps  = m_numInputs + 1;
    m_input.resize( totalInps );

    epin->setId( m_elmId+"-ePin-input"+m_numInputs );

    m_input[m_numInputs] = new eSource( m_elmId+"-eSource-input"+QString::number(m_numInputs), epin, input );
    //m_input[m_numInputs]->setInputImp( m_inputImp );

    m_inputState.resize( totalInps );
    m_inputState[m_numInputs] = false;
    
    m_numInputs = totalInps;
}

void eLogicDevice::createInputs( int inputs )
{
    int totalInps  = m_numInputs + inputs;
    m_input.resize( totalInps );

    for( int i=m_numInputs; i<totalInps; ++i )
    {
        ePin* epin = new ePin( m_elmId+"-ePin-input"+i, i );

        m_input[i] = new eSource( m_elmId+"-eSource-input"+i, epin, input );
        //m_input[i]->setInputImp( m_inputImp );
    }
    m_numInputs = totalInps;
}

void eLogicDevice::createOutput( ePin* epin )
{
    int totalOuts = m_numOutputs + 1;
    m_output.resize( totalOuts );

    epin->setId( m_elmId+"-ePin-output"+m_numOutputs );

    m_output[m_numOutputs] = new eSource( m_elmId+"-eSource-output"+m_numOutputs, epin, output );
    m_output[m_numOutputs]->setVoltHigh( m_outHighV );
    //m_output[m_numOutputs]->setOutputImp( m_outImp );

    m_numOutputs = totalOuts;
}

void eLogicDevice::createOutputs( int outputs )
{
    int totalOuts = m_numOutputs + outputs;
    m_output.resize( totalOuts );

    for( int i=m_numOutputs; i<totalOuts; ++i )
    {
        ePin* epin = new ePin( m_elmId+"-ePin-output"+i, i );

        m_output[i] = new eSource( m_elmId+"-eSource-output"+i, epin, output );
        m_output[i]->setVoltHigh( m_outHighV );
        //m_output[i]->setOutputImp( m_outImp );
    }
    m_numOutputs = totalOuts;
}

void eLogicDevice::deleteInputs( int inputs )
{
    if( m_numInputs-inputs < 0 ) inputs = m_numInputs;

    for( int i=m_numInputs-1; i>m_numInputs-inputs-1; i-- )
    {
        delete m_input[i];
        m_input.pop_back();
    }
    m_numInputs -= inputs;
}

void eLogicDevice::deleteOutputs( int outputs )
{
    if( m_numOutputs-outputs < 0 ) outputs = m_numOutputs;

    for( int i=m_numOutputs-1; i>m_numOutputs-outputs-1; i-- )
    {
        delete m_output[i];
        m_output.pop_back();
    }
    m_numOutputs -= outputs;
}

void eLogicDevice::setNumInps( int inputs )
{
    if     ( inputs > m_numInputs ) createInputs( inputs - m_numInputs );
    else if( inputs < m_numInputs ) deleteInputs( m_numInputs - inputs );
    else return;
    m_inputState.resize( inputs );
}

void eLogicDevice::setNumOuts( int outputs )
{
    if     ( outputs > m_numOutputs ) createOutputs( outputs - m_numOutputs );
    else if( outputs < m_numOutputs ) deleteOutputs( m_numOutputs - outputs );
}

void eLogicDevice::setOut( int num, bool out )
{
    m_output[num]->setState( out, true );
}

void eLogicDevice::setOutHighV( double volt )
{
    bool pauseSim = Simulator::self()->isRunning();
    if( pauseSim ) Simulator::self()->pauseSim();

    m_outHighV = volt;

    for( int i=0; i<m_numOutputs; ++i )
        m_output[i]->setVoltHigh( volt );

    if( pauseSim ) Simulator::self()->resumeSim();
}

void eLogicDevice::setOutLowV( double volt )
{
    bool pauseSim = Simulator::self()->isRunning();
    if( pauseSim ) Simulator::self()->pauseSim();

    m_outLowV = volt;

    for( int i=0; i<m_numOutputs; ++i )
        m_output[i]->setVoltLow( volt );

    if( pauseSim ) Simulator::self()->resumeSim();
}

void eLogicDevice::setInputImp( double imp )
{
    bool pauseSim = Simulator::self()->isRunning();
    if( pauseSim ) Simulator::self()->pauseSim();

    m_inputImp = imp;
    for( int i=0; i<m_numInputs; ++i )
        m_input[i]->setImp( imp );

    if( pauseSim ) Simulator::self()->resumeSim();
}

void eLogicDevice::setOutImp( double imp )
{
    bool pauseSim = Simulator::self()->isRunning();
    if( pauseSim ) Simulator::self()->pauseSim();

    if( m_outImp == imp ) return;

    m_outImp = imp;

    for( int i=0; i<m_numOutputs; ++i )
    {
        m_output[i]->setImp( imp );
    }
    if( pauseSim ) Simulator::self()->resumeSim();
}

void eLogicDevice::setInverted( bool inverted )
{
    bool pauseSim = Simulator::self()->isRunning();
    if( pauseSim ) Simulator::self()->pauseSim();

    m_inverted = inverted;
    
    for( int i=0; i<m_numOutputs; ++i )
    {
        m_output[i]->setInverted( inverted );
    }
    Circuit::self()->update();

    if( pauseSim ) Simulator::self()->resumeSim();
}

void eLogicDevice::setInvertInps( bool invert )
{
    bool pauseSim = Simulator::self()->isRunning();
    if( pauseSim ) Simulator::self()->pauseSim();

    m_invInputs = invert;
    for( int i=0; i<m_numInputs; ++i )
    {
        m_input[i]->setInverted( invert );
    }
    Circuit::self()->update();

    if( pauseSim ) Simulator::self()->resumeSim();
}

void eLogicDevice::seteTrigger( int trigger )
{
    //qDebug() << "eLogicDevice::seteTrigger"<<trigger;
    m_etrigger = trigger;
    m_clock = false;
}

void eLogicDevice::setClockInv( bool inv )     
{
    if( !m_clockSource ) return;

    bool pauseSim = Simulator::self()->isRunning();
    if( pauseSim ) Simulator::self()->pauseSim();

    m_clockSource->setInverted(inv);
    Circuit::self()->update();

    if( pauseSim ) Simulator::self()->resumeSim();
}

int eLogicDevice::getClockState()
{
    if( !m_clockSource ) return Clock_Allow;

    int cState = 0;

    bool clock = m_clock;
    double volt = m_clockSource->getVolt(); // Clock pin volt.

    if     ( volt > m_inputHighV ) clock = true;
    else if( volt < m_inputLowV )  clock = false;

    if( m_clockSource->isInverted() ) clock = !clock;

    if( m_etrigger == Trig_InEn )
    {
        if     (!clock ) cState = Clock_Low;
        else if( clock ) cState = Clock_Allow;
    }
    else if( m_etrigger == Trig_Clk )
    {
        if     (!m_clock &&  clock ) cState = Clock_Rising;
        else if( m_clock &&  clock ) cState = Clock_High;
        else if( m_clock && !clock ) cState = Clock_Falling;
    }
    else cState = Clock_Allow;
    m_clock = clock;

    m_clockSource->getPin()->setPinState( clock? input_high:input_low ); // High-Low colors

    return cState;
}

bool eLogicDevice::getInputState( int input )
{
    double volt = m_input[input]->getVolt();
    bool  state = m_inputState[input];

    if     ( volt > m_inputHighV ) state = true;
    else if( volt < m_inputLowV )  state = false;

    m_input[input]->getPin()->setPinState( state? input_high:input_low ); // High : Low colors

    if( m_input[input]->isInverted() ) state = !state;
    m_inputState[input] = state;
    
    return state;
}

bool eLogicDevice::getOutputState( int output )
{
    return m_output[output]->getState();
}

void eLogicDevice::setRiseTime( uint64_t time )
{
    m_timeLH = time;
    //for( int i=0; i<m_numOutputs; ++i ) m_output[i]->setRiseTime( time );
}

void eLogicDevice::setFallTime( uint64_t time )
{
    m_timeHL = time;
    //for( int i=0; i<m_numOutputs; ++i ) m_output[i]->setFallTime( time );
}

ePin* eLogicDevice::getEpin( QString pinName )
{
    if( pinName.contains("input") )
    {
        int pin = pinName.remove("input").toInt();

        return m_input[pin]->getEpin(0);
    }
    else if( pinName.contains("output") )
    {
        int pin = pinName.remove("output").toInt();

        return m_output[pin]->getEpin(0);
    }
    else if( pinName.contains("clock")
          || pinName.contains("inputEnable"))
    {
        return m_clockSource->getEpin(0);
    }
    else if( pinName.contains("outEnable") )
    {
        return m_outEnSource->getEpin(0);
    }
    
    return 0l;
}

