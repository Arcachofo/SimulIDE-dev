/***************************************************************************
 *   Copyright (C) 2020 by santiago González                               *
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

#include "pictimer.h"
#include "e_mcu.h"
#include "simulator.h"
#include "datautils.h"

McuTimer* PicTimer::createTimer( eMcu* mcu, QString name ) // Static
{
    QString n = name.toLower();
    if     ( n == "timer0" ) return new PicTimer0( mcu, name );
    else if( n == "timer1" ) return new PicTimer1( mcu, name );
    else if( n == "timer2" ) return new PicTimer2( mcu, name );
    //else                     return new PicTimer16bit( mcu, name );

    return NULL;
}

PicTimer::PicTimer(  eMcu* mcu, QString name )
        : McuTimer( mcu, name )
{
}
PicTimer::~PicTimer(){}

void PicTimer::initialize()
{
    McuTimer::initialize();

    m_ovfMatch  = m_maxCount;
    m_ovfPeriod = m_ovfMatch + 1;
}

void PicTimer::addOcUnit( McuOcUnit* ocUnit )
{
}

McuOcUnit* PicTimer::getOcUnit( QString name )
{
    return NULL;
}

void PicTimer::configureA( uint8_t val )
{
}

void PicTimer::configureB( uint8_t val )
{
}

void PicTimer::configureClock()
{
    m_prescaler = m_prescList.at( m_mode );
    m_clkSrc = clkMCU;
}

void PicTimer::configureExtClock()
{
    m_prescaler = 1;
    m_clkSrc = clkEXT;
    /// if     ( m_mode == 6 ) m_clkEdge = Clock_Falling;
    /// else if( m_mode == 7 ) m_clkEdge = Clock_Rising;
}

void PicTimer::configureOcUnits( bool disable )
{

}

//--------------------------------------------------
// TIMER 8 Bit--------------------------------------

PicTimer8bit::PicTimer8bit( eMcu* mcu, QString name )
            : PicTimer( mcu, name )
{
    m_maxCount = 0xFF;
}
PicTimer8bit::~PicTimer8bit(){}


//--------------------------------------------------
// TIMER 0 -----------------------------------------

PicTimer0::PicTimer0( eMcu* mcu, QString name)
         : PicTimer8bit( mcu, name )
{
    m_T0CS = getRegBits( "T0CS", mcu );
    m_T0SE = getRegBits( "T0SE", mcu );
    m_PSA  = getRegBits( "PSA", mcu );
    m_PS   = getRegBits( "PS0,PS1,PS2", mcu );
}
PicTimer0::~PicTimer0(){}

void PicTimer0::initialize()
{
    PicTimer::initialize();

    m_running = true;
    Simulator::self()->cancelEvents( this );
    sheduleEvents();
}

void PicTimer0::configureA( uint8_t NewOPTION )
{
    uint8_t ps = getRegBitsVal( NewOPTION, m_PS );

    if( getRegBitsBool( NewOPTION, m_PSA ) )
         m_prescaler = 1;                    // Prescaler asigned to Watchdog
    else m_prescaler = m_prescList.at( ps ); // Prescaler asigned to TIMER0

    uint8_t mode = getRegBitsVal( NewOPTION, m_T0CS );
    if( mode != m_mode )
    {
        m_mode = mode;
        if( mode ) m_clkSrc = clkEXT;
        else       m_clkSrc = clkMCU;
        enable( mode==0 );
    }

    /*uint8_t clkEdge = getRegBitsVal( NewOPTION, m_T0SE );
    if     ( clkEdge == 1 ) m_clkEdge = Clock_Falling;
    else if( clkEdge == 0 ) m_clkEdge = Clock_Rising;*/

    Simulator::self()->cancelEvents( this );
    sheduleEvents();
}

//--------------------------------------------------
// TIMER 2 -----------------------------------------

PicTimer2::PicTimer2( eMcu* mcu, QString name)
         : PicTimer8bit( mcu, name )
{
    m_ps = 0;

    m_PR2 = mcu->getReg( "PR2" );

    m_TMR2ON = getRegBits( "TMR2ON", mcu );
    m_T2CKPS = getRegBits( "T2CKPS0,T2CKPS1", mcu );
}
PicTimer2::~PicTimer2(){}

void PicTimer2::configureA( uint8_t NewT2CON )
{
    m_ps = getRegBitsVal( NewT2CON, m_T2CKPS );
    m_prescaler = m_prescList.at( m_ps ) * (*m_PR2+1);

    bool en = getRegBitsBool( NewT2CON, m_TMR2ON );
    if( en != m_running ) enable( en );
}

void PicTimer2::configureB( uint8_t NewPR2 )
{
    m_prescaler = m_prescList.at( m_ps ) * ( NewPR2+1);
}

//--------------------------------------------------
// TIMER 16 Bit-------------------------------------


PicTimer16bit::PicTimer16bit( eMcu* mcu, QString name )
             : PicTimer( mcu, name )
{
    m_maxCount = 0xFFFF;

    QString num = name.right(1);
}
PicTimer16bit::~PicTimer16bit(){}

//--------------------------------------------------
// TIMER 1 -----------------------------------------

PicTimer1::PicTimer1( eMcu* mcu, QString name)
         : PicTimer16bit( mcu, name )
{
    m_T1CKPS = getRegBits( "T1CKPS0,T1CKPS1", mcu );
    m_T1OSCEN = getRegBits( "T1OSCEN", mcu );

    m_TMR1CS = getRegBits( "TMR1CS", mcu );
    m_TMR1ON = getRegBits( "TMR1ON", mcu );
}
PicTimer1::~PicTimer1(){}

void PicTimer1::configureA( uint8_t NewT1CON )
{
    uint8_t ps = getRegBitsVal( NewT1CON, m_T1CKPS );
    m_prescaler = m_prescList.at( ps );

    m_t1Osc = getRegBitsBool( NewT1CON, m_T1OSCEN );

    uint8_t mode = getRegBitsVal( NewT1CON, m_TMR1CS );
    if( mode != m_mode )
    {
        m_mode = mode;
        if( mode ) m_clkSrc = clkEXT;
        else       m_clkSrc = clkMCU;
    }

    bool en = getRegBitsBool( NewT1CON, m_TMR1ON );
    if( en != m_running ) enable( en && !mode );  /// TODO ext Clock
}

void PicTimer1::sheduleEvents()
{
    if( m_running && m_t1Osc )
    {
        uint64_t circTime = Simulator::self()->circTime();
        m_scale = 30517578; // Sim cycs per Timer tick for 32.768 KHz

        uint32_t ovfPeriod = m_ovfPeriod;
        if( m_countVal > m_ovfPeriod ) ovfPeriod += m_maxCount;

        uint64_t cycles = (ovfPeriod-m_countVal)*m_scale; // cycles in ps
        m_ovfCycle = circTime + cycles;// In simulation time (ps)

        Simulator::self()->addEvent( cycles, this );
    }
    else McuTimer::sheduleEvents();
}
