/***************************************************************************
 *   Copyright (C) 2023 by Santiago González                               *
 *                                                                         *
 ***( see copyright.txt file at root folder )*******************************/

#include "picintosc.h"
#include "datautils.h"
#include "simulator.h"
#include "mcupin.h"
#include "e_mcu.h"
#include "mcu.h"

PicIntOsc::PicIntOsc( eMcu* mcu, QString name )
         : McuIntOsc( mcu, name )
{
    m_cfgWordCtrl = true;

    m_SCS  = getRegBits( "SCS", mcu );
    m_IRCF = getRegBits( "IRCF0,IRCF1,IRCF2,", mcu );
}
PicIntOsc::~PicIntOsc(){}

void PicIntOsc::stamp()
{
    m_configFreq = m_mcu->freq();
    m_psCycle = m_mcu->psCycle()/2;
    if( m_cfgWordCtrl ) McuIntOsc::stamp();
}

void PicIntOsc::configureA( uint8_t newOSCCON )
{
    uint8_t ircf = getRegBitsVal(  newOSCCON, m_IRCF );
    switch( ircf ) {
        case 0: m_intOscFreq = 31*1e3;  break; // 31  kHz (LFINTOSC)
        case 1: m_intOscFreq = 125*1e3; break; // 125 kHz
        case 2: m_intOscFreq = 250*1e3; break; // 250 kHz
        case 3: m_intOscFreq = 500*1e3; break; // 500 kHz
        case 4: m_intOscFreq = 1*1e6;   break; // 1 MHz
        case 5: m_intOscFreq = 2*1e6;   break; // 2 MHz
        case 6: m_intOscFreq = 4*1e6;   break; // 4 MHz (default)
        case 7: m_intOscFreq = 8*1e6;   break; // 8 MHz
    }
    m_cfgWordCtrl = !getRegBitsBool( newOSCCON, m_SCS );

    bool intOsc = !m_cfgWordCtrl || m_clkInIO; // Not controlled by CONFIG1 or controlled and set to INTOSC
    double freq = intOsc ? m_intOscFreq : m_mcu->component()->extFreq();
    m_mcu->setFreq( freq );
    m_psCycle = m_mcu->psCycle()/2;
    //QString msg = intOsc ? "Internal" : "External";
    //qDebug() << msg<< "Clock"<< freq << "Hz" <<"\n";
}

