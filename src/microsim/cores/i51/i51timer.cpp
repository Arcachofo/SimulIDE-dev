/***************************************************************************
 *   Copyright (C) 2020 by Santiago González                               *
 *                                                                         *
 ***( see copyright.txt file at root folder )*******************************/

#include "i51timer.h"
#include "e_mcu.h"
#include "simulator.h"
#include "datautils.h"

#define COUNT_L m_countL[0]
#define COUNT_H m_countH[0]

I51Timer::I51Timer( eMcu* mcu, QString name)
        : McuTimer( mcu, name )
{
    QString n = m_name.right(1);
    m_TxM  = getRegBits( "T"+n+"M0,T"+n+"M1", mcu );
    m_CTx  = getRegBits( "C/T"+n, mcu );
    m_GATE = getRegBits( "GATE"+n, mcu );
}
I51Timer::~I51Timer(){}

void I51Timer::initialize()
{
    McuTimer::initialize();
    //configure( 0 );           // Defautl mode = 0
    m_ovfMatch = 0x1FFF;
    m_ovfPeriod = m_ovfMatch + 1;
    m_gate = 0;
}

void I51Timer::configureA( uint8_t newTMOD ) // TxM0,TxM1
{
    uint8_t mode = getRegBitsVal( newTMOD, m_TxM );

    if( mode != m_mode )
    {
        m_mode = mode;

        switch( mode )
        {
            case 0: m_ovfMatch = 0x1FFF; break; // 13 bits
            case 1: m_ovfMatch = 0xFFFF; break; // 16 bits
            case 2: m_ovfMatch = 0x00FF; break; // 8 bits
            case 3:                             // 8+8 bits
            {
                m_ovfMatch = 0x00FF;

                if( m_number == 0 )
                {
                }
                else if( m_number == 1 )
                {
                }
            }
        }
        m_ovfPeriod = m_ovfMatch+1;
    }

    bool extClock = getRegBitsVal( newTMOD, m_CTx );
    if( extClock != m_extClock )
    {
        m_extClock = extClock;
    }
    bool gate = getRegBitsVal( newTMOD, m_GATE );
    if( gate != m_gate )
    {
        m_gate = gate;
        /// TODO
    }
}

void I51Timer::updtCycles() // Recalculate ovf, comps, etc
{
    switch( m_mode )
    {
        case 0:  // 13 bits
        {
            m_countVal  = COUNT_H << 5;
            m_countVal |= COUNT_L & 0b00011111;
            m_countStart = 0;
        } break;
        case 1: // 16 bits
        {
            m_countVal  = COUNT_H << 8;
            m_countVal |= COUNT_L;
            m_countStart = 0;
        } break;
        case 2: // 8 bits
        {
            //m_ovfMatch = m_ovfPeriod-m_countH;
            m_countVal   = COUNT_H;
            m_countStart = COUNT_H;
        } break;
        case 3: // 8+8 bits
        {
            if     ( m_number == 0 ) m_countVal = COUNT_L;
            else if( m_number == 1 ) m_countVal = COUNT_H;
            m_countStart = 0;
        }
    }
    sheduleEvents();
}

void I51Timer::updtCount( uint8_t )     // Write counter values to Ram
{
    if( m_running ) // If no running, values were already written at timer stop.
    {
        if( m_mode == 1 ) // 16 bits
        {
            McuTimer::updtCount();
            return;
        }
        uint64_t timTime = m_ovfCycle-Simulator::self()->circTime(); // Next overflow time - current time
        uint16_t countVal = timTime/m_mcu->psCycle()/m_prescaler;

        if( m_mode == 0 )  // 13 bits
        {
            COUNT_L = countVal & 0b00011111;
            COUNT_H = (countVal>>5) & 0xFF;
        }
        else if( m_mode == 2 ) // 8 bits
        {
            COUNT_L = countVal & 0xFF;
            //if( m_countH ) m_countH[0] = (countVal>>8) & 0xFF;
        }
        else                 // 8+8 bits
        {
            if     ( m_number == 0 ) COUNT_L = countVal & 0xFF;
            else if( m_number == 1 ) COUNT_H = countVal & 0xFF;
        }
    }
}
