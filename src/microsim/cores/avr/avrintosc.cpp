/***************************************************************************
 *   Copyright (C) 2024 by Coethium                                        *
 *                                                                         *
 ***( see copyright.txt file at root folder )*******************************/

//#include <QDebug>

#include "avrintosc.h"
#include "e_mcu.h"
#include "mcu.h"
#include "datautils.h"

/*
  Logic behind is based on the usage of the internal RC Osc as set by Fuses by the constructor.

  Default values must be set in resources :
    - *regs.xml   : CLKPR reset= "binary-value"
    - *perifs.xml : intosc configregsA="CLKPR"
    - *.mcu       : frequency="factory frequency"

  The user is free to choose custom frequency on UI.
  It doesn't conform to others Clock Source (as fast PLL).
  In theses two cases, or when defaults values are not set, the meaning of CLKPR does not conform to the specs.

  TODO ? : adding fuses config
*/

/// Problem:
/// CKDIV8 fuse is programmed by default, setting div factor to 8
/// So the frequency set in properties widget is always divided by 8

AvrIntOsc::AvrIntOsc( eMcu* mcu, QString name )
         : McuIntOsc( mcu, name )
{
    m_CLKPS = getRegBits("CLKPS0,CLKPS1,CLKPS2,CLKPS3", mcu );
}
AvrIntOsc::~AvrIntOsc(){}

void AvrIntOsc::reset()
{
    m_prIndex = 0;
}

void AvrIntOsc::configureA( uint8_t newCLKPR ) // it's faster to calculate bit shift than using prescalers list from XML
{
    /// TODO: CLKPR can be modified only within 4 clock after CKSEL sets, not really important

    uint8_t prIndex = getRegBitsVal( newCLKPR, m_CLKPS );
    if( m_prIndex != prIndex )
    {
        m_prIndex = prIndex;
        freqChanged();
    }
}

bool AvrIntOsc::freqChanged()
{
    double freq = m_mcu->component()->uiFreq(); // Frequency set in UI
    m_intOscFreq = freq/(1 << m_prIndex);
    m_mcu->setFreq( m_intOscFreq );
    return true;
}
