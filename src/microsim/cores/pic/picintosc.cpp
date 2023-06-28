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


PicIntOsc* PicIntOsc::createIntOsc( eMcu* mcu, QString name, QString type )
{
    if     ( type == ""   ) return new PicIntOsc( mcu, name );   // 16F876
    else if( type == "00" ) return new PicIntOsc00( mcu, name ); //
    else if( type == "01" ) return new PicIntOsc01( mcu, name ); // 16F627
    else if( type == "02" ) return new PicIntOsc02( mcu, name ); // 16F1825
    return NULL;
}

PicIntOsc::PicIntOsc( eMcu* mcu, QString name )
         : McuIntOsc( mcu, name )
{
    m_cfgWordCtrl = true;

    m_intOscFreq = 4*1e6; // 4 MHz (default)
}
PicIntOsc::~PicIntOsc(){}

void PicIntOsc::stamp()
{
    m_configFreq = m_mcu->freq();
    m_psCycle = m_mcu->psCycle()/2;
    if( m_cfgWordCtrl ) McuIntOsc::stamp();
}

void PicIntOsc::setPin( int n, McuPin* p )
{
    if     ( n == 0 ) m_clkOutPin = m_clkPin[0] = p; // RA6 16F886
    else if( n == 1 ) m_clkInPin  = m_clkPin[1] = p; // RA7 16F886
    else if( n == 2 ) m_clkOutPin = p;               // Ktal2 16F876
}

// -------------------------------------------

PicIntOsc00::PicIntOsc00( eMcu* mcu, QString name )
           : PicIntOsc( mcu, name )
{
    m_SCS  = getRegBits( "SCS", mcu );
    m_IRCF = getRegBits( "IRCF0,IRCF1,IRCF2", mcu );
}
PicIntOsc00::~PicIntOsc00(){}

void PicIntOsc00::configureA( uint8_t newOSCCON )
{
    uint8_t ircf = getRegBitsVal( newOSCCON, m_IRCF );
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

// -------------------------------------------

PicIntOsc01::PicIntOsc01( eMcu* mcu, QString name )
           : PicIntOsc( mcu, name )
{
    m_OSCF = getRegBits( "OSCF", mcu );
}
PicIntOsc01::~PicIntOsc01(){}

void PicIntOsc01::configureA( uint8_t newOSCCON )
{
    bool oscf = getRegBitsBool(  newOSCCON, m_OSCF );
    m_intOscFreq = oscf ? 4*1e6 : 48*1e3;

    double freq = m_clkInIO ? m_intOscFreq : m_mcu->component()->extFreq(); // Not controlled by CONFIG1 or controlled and set to INTOSC
    m_mcu->setFreq( freq );
    m_psCycle = m_mcu->psCycle()/2;
}

// -------------------------------------------

PicIntOsc02::PicIntOsc02( eMcu* mcu, QString name )
           : PicIntOsc( mcu, name )
{
    m_SCS  = getRegBits( "SCS0,SCS1", mcu );
    m_IRCF = getRegBits( "IRCF0,IRCF1,IRCF2,IRCF3", mcu );
}
PicIntOsc02::~PicIntOsc02(){}

void PicIntOsc02::configureA( uint8_t newOSCCON )
{
    uint8_t ircf = getRegBitsVal(  newOSCCON, m_IRCF );
    switch( ircf ) {
        case  0: // Fallthrough
        case  1: m_intOscFreq = 31*1e3;    break; // 31  kHz
        case  2: // Fallthrough
        case  3: m_intOscFreq = 31.25*1e3; break; // 31.25 kHz
        case  5: m_intOscFreq = 125*1e3;   break; // 125 kHz
        case  6: m_intOscFreq = 250*1e3;   break; // 250 kHz
        case  7: m_intOscFreq = 500*1e3;   break; // 500 kHz
        case  8: m_intOscFreq = 250*1e3;   break; // 250 kHz
        case  9: m_intOscFreq = 500*1e3;   break; // 500 kHz
        case 10: m_intOscFreq = 500*1e3;   break; // 500 kHz
        case 11: m_intOscFreq = 1*1e6;     break; // 1 MHz
        case 12: m_intOscFreq = 2*1e6;     break; // 2 MHz
        case 13: m_intOscFreq = 4*1e6;     break; // 4 MHz (default)
        case 14: m_intOscFreq = 8*1e6;     break; /// 8 MHz or 32 MHz HF(see Section 5.2.2.1 “HFINTOSC”)
        case 15: m_intOscFreq = 16*1e6;    break; // 16 MHz
    }
    uint8_t scs = getRegBitsVal( newOSCCON, m_SCS );
    m_cfgWordCtrl = scs == 0;

    bool intOsc = !m_cfgWordCtrl || m_clkInIO; // Not controlled by CONFIG1 or controlled and set to INTOSC
    double freq = intOsc ? m_intOscFreq : m_mcu->component()->extFreq();
    m_mcu->setFreq( freq );
    m_psCycle = m_mcu->psCycle()/2;
}
