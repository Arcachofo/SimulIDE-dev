/***************************************************************************
 *   Copyright (C) 2023 by Santiago González                               *
 *                                                                         *
 ***( see copyright.txt file at root folder )*******************************/

#include "mcuintosc.h"
#include "simulator.h"
#include "mcupin.h"
#include "e_mcu.h"
#include "mcu.h"

McuIntOsc::McuIntOsc( eMcu* mcu, QString name )
         : McuModule( mcu, name )
         , eElement( name )
{
    m_clkInIO  = true;
    m_clkOutIO = true;
    m_clkOut   = false;
    m_extClock = false;

    m_multiplier = 1;

    m_clkOutPin = nullptr;
    m_clkInPin  = nullptr;

    m_clkPin[0] = nullptr;
    m_clkPin[1] = nullptr;
}
McuIntOsc::~McuIntOsc(){}

void McuIntOsc::stamp()
{
    if( m_clkInPin ) m_clkInPin->controlPin( !m_clkInIO, !m_clkInIO );

    if( m_clkOutPin )
    {
        bool ctrlO = !m_clkOutIO || m_clkOut;
        m_clkOutPin->controlPin( ctrlO, ctrlO );

        if( m_clkOut )
        {
            m_clkOutPin->setPinMode( output );
            m_psInst = m_mcu->psInst()/2;
            Simulator::self()->addEvent( m_psInst, this );
        }
    }
}

void McuIntOsc::runEvent()
{
    m_clkOutPin->toggleOutState();
    Simulator::self()->addEvent( m_psInst, this );
}

void McuIntOsc::enableExtOsc( bool en ) // From Mcu, AVR or PIC with no cfg word
{
    m_extClock = en;
    /// if( m_mcu->cfgWord() ) return;     // Controlled by Config word.
    if( m_clkPin[0] == nullptr ) return;

    for( int i=0; i<2; ++i )
        if( m_clkPin[i] ){
            m_clkPin[i]->controlPin( en, en );
            m_clkPin[i]->setUnused( en );
        }

    if( en ){
        for( int i=0; i<2; ++i )
            if( m_clkPin[i] ) m_clkPin[i]->setPinMode( input );
    }
}

void McuIntOsc::setClockOut( bool clkOut )
{
    m_clkOut = clkOut;
    if( m_clkOutPin ) m_clkOutPin->setUnused( !m_clkOutIO && !clkOut );
}

void McuIntOsc::configPins( bool inIo, bool outIo, bool clkOut )
{
    m_clkInIO  = inIo;
    m_clkOutIO = outIo;
    m_extClock = !m_clkInIO || !m_clkOutIO;
    if( m_clkInPin ) m_clkInPin->setUnused( !inIo );

    setClockOut( clkOut );
}

void McuIntOsc::setPin( int n, McuPin* p )
{
    if     ( n == 0 ) m_clkPin[0] = p;
    else if( n == 1 ) m_clkPin[1] = m_clkInPin = p;
    else if( n == 2 ) m_clkOutPin = p;
}
