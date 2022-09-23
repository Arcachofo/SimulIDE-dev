/***************************************************************************
 *   Copyright (C) 2022 by Santiago González                               *
 *                                                                         *
 ***( see copyright.txt file at root folder )*******************************/

#include <math.h>

#include "intmemcore.h"
#include "e_mcu.h"
#include "mcupin.h"
#include "simulator.h"
#include "circuit.h"

IntMemCore::IntMemCore( eMcu* mcu )
          : CpuBase( mcu )
          , eElement( mcu->getId()+"-core" )
{
    m_rwPin = NULL;
    //m_rePin = NULL;
    m_cshPin = NULL; // Enable Pin
    m_cslPin = NULL; // !Enable Pin
    m_clkPin = NULL; // Clock Pin

    m_propDelay = 10*1000; // 10 ns
    m_asynchro = false;
}
IntMemCore::~IntMemCore() {}

//void IntMemCore::initialize(){;}

/*void IntMemCore::stamp()
{
    m_nextOutVal = 0;
    m_outValue = 0;
    m_cs = false;

    enableOutputs( false );
    m_clkPin->changeCallBack( this );
}*/

void IntMemCore::reset()  // NO: Reset happens after initialize() in Pins.
{
    m_memState = mem_IDLE;
    write( true );

    m_nextOutVal = 0;
    m_outValue = 0;
    m_cs = false;

    //enableOutputs( false );
    //m_clkPin->changeCallBack( this );
}

void IntMemCore::extClock( bool clkState )
{
    bool CS = !clkState;
    bool csh = m_cshPin->getInpState();
    bool csl = !m_cslPin->getInpState();

    if( m_cshPin ) CS = CS && csh;
    if( m_cslPin ) CS = CS && csl;
    //if( m_clkPin ) CS = CS && !m_clkPin->getInpState();
    if( CS != m_cs ){
        m_cs = CS;
        //enableOutputs( CS );
    }
    if( !CS ) return;

    m_we = !m_rwPin->getInpState();
    m_addr = 0;
    for( uint i=0; i<m_addrPin.size(); ++i )        // Get Address
    {
        bool state = m_addrPin[i]->getInpState();
        if( state ) m_addr += pow( 2, i );
    }
    if( m_we ){                             // Write
        write( true );
        //Simulator::self()->addEvent( m_propDelay, this );
    }
    else{                                   // Read
        m_nextOutVal = m_mcu->readReg( m_addr );
        write( false );
        //sheduleOutPuts();
}   }

void IntMemCore::runEvent()
{
    if( m_write )
    {
        int value = 0;
        for( uint i=0; i<m_dataPin.size(); ++i )
        {
            bool state = m_dataPin[i]->getInpState();
            if( state ) value += pow( 2, i );
            m_dataPin[i]->setPinState( state? input_high:input_low ); // High-Low colors
        }
        m_mcu->writeReg( m_addr, value );
    }
    else runOutputs();
}

void IntMemCore::write( bool w )
{
    m_write = w;

    if( m_write )
    {
        int value = 0;
        for( uint i=0; i<m_dataPin.size(); ++i )
        {
            bool state = m_dataPin[i]->getInpState();
            if( state ) value += pow( 2, i );
            m_dataPin[i]->setPinState( state? input_high:input_low ); // High-Low colors
        }
        m_mcu->writeReg( m_addr, value );
    }
    else sheduleOutPuts();

    for( IoPin* pin : m_dataPin )
    {
        pin->setPinMode( w ? input : output );
        if( m_asynchro ) pin->changeCallBack( this, w );
    }
    Simulator::self()->cancelEvents( this );
}

void IntMemCore::enableOutputs( bool en )
{
    for( uint i=0; i<m_dataPin.size(); ++i ) m_dataPin[i]->setStateZ( !en );
}

void IntMemCore::runOutputs()
{
    m_outValue = m_nextOutVal;

    for( uint i=0; i<m_dataPin.size(); ++i )
    {
        bool state = m_outValue & (1<<i);
        bool oldst = m_dataPin[i]->getOutState();

        if( state != oldst ) m_dataPin[i]->sheduleState( state, 0 );
    }
}

void IntMemCore::sheduleOutPuts()
{
    if( m_nextOutVal == m_outValue ) return;
    Simulator::self()->addEvent( m_propDelay, this );
}
