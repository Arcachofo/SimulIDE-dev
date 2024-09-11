/***************************************************************************
 *   Copyright (C) 2020 by Santiago González                               *
 *                                                                         *
 ***( see copyright.txt file at root folder )*******************************/

#include "mcuocunit.h"
#include "mcupin.h"
#include "e_mcu.h"
#include "simulator.h"

McuOcUnit::McuOcUnit( eMcu* mcu, QString name )
         : McuModule( mcu, name )
         , eElement( mcu->getId()+"-"+name )
{
    m_timer = nullptr;
    m_ocPin = nullptr;
    m_ocm   = nullptr;
    m_ocPinInv = nullptr;
}
McuOcUnit::~McuOcUnit( ){}

void McuOcUnit::initialize()
{
    m_comMatch = 0;
    m_extMatch = 0;

    m_enabled = false;
    m_ctrlPin = false;
    m_mode = 0;

    m_comAct = ocNON;
    m_tovAct = ocNON;
    m_ocPin->controlPin( false, false );
}

void McuOcUnit::clockStep( uint16_t count )
{
    if( count == m_extMatch ) runEvent();
}

void McuOcUnit::runEvent()  // Compare match
{
    m_interrupt->raise();   // Trigger interrupt
    if( m_enabled ) drivePin( m_comAct, m_timer->psPerTick() );
}

void McuOcUnit::drivePin( ocAct_t act, uint64_t time )
{
    if( !act ) return;
    bool pinState = false;

    if     ( act == ocTOG ) pinState = !m_ocPin->getOutState();
    else if( act == ocCLR ) pinState = !m_pinSet;
    else if( act == ocSET ) pinState =  m_pinSet;

    setPinSate( pinState, time );
}

void McuOcUnit::setPinSate( bool state, uint64_t time )
{
    m_ocPin->scheduleState( state, time );
}

void McuOcUnit::sheduleEvents( uint32_t ovf, uint32_t countVal, int rot )
{
    ovf      <<= rot;  // Used by Pic CCP PWM mode: 8+2 bits (rot=2)
    countVal <<= rot;

    uint64_t match;

    if( m_timer->reverse() )
    {
        match = ovf-m_comMatch;
        m_pinSet = false;
    }else{
        match = m_comMatch;
        m_pinSet = true;
    }
    Simulator::self()->cancelEvents( this );

    if( m_timer->extClocked() ) m_extMatch = match; // Using external clock
    else{
        if( match <= ovf && match >= countVal ) // be sure next comp match is still ahead
        {
            uint64_t psPerTick  = m_timer->psPerTick();
            uint64_t timeOffset = m_timer->timeOffset();

            uint64_t time2ovf = (match-countVal)*psPerTick; // Time in ps
            if( timeOffset ) time2ovf -= psPerTick-timeOffset;

            Simulator::self()->addEvent( time2ovf, this );
        }
    }
}

void McuOcUnit::setOcActs( ocAct_t comAct, ocAct_t tovAct )
{
    m_comAct = comAct;
    m_tovAct = tovAct;
}

void McuOcUnit::ocrWriteL( uint8_t val )
{
    m_comMatch = (m_comMatch & 0xFF00) | val;
    if( m_timer->running() ) sheduleEvents( m_timer->ovfMatch(), m_timer->getCount() );
}

void McuOcUnit::ocrWriteH( uint8_t val )
{
    m_comMatch = (m_comMatch & 0x00FF) | (uint16_t)val<<8;
    //sheduleEvents( m_timer->ovfMatch(), m_timer->getCount() );
}
