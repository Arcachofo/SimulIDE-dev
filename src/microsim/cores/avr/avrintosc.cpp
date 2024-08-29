/***************************************************************************
 *   Copyright (C) 2024 by Coethium                                        *
 *                                                                         *
 ***( see copyright.txt file at root folder )*******************************/

#include "avrintosc.h"
#include "e_mcu.h" 
//#include "mcudataspace.h"
//#include "datautils.h"

#include <QDebug>

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

AvrIntOsc::AvrIntOsc( eMcu* mcu, QString name )
    : McuIntOsc( mcu, name )
{      
    regInfo_t regInfo = mcu->regInfo()->value("CLKPR");
    m_prIndex = regInfo.resetVal;
    m_intOscFreq = m_mcu->freq()*(1 << m_prIndex); // RC Osc
}

AvrIntOsc::~AvrIntOsc(){}


// it's faster to calculate bit shift than using prescalers list from XML
void AvrIntOsc::configureA(uint8_t newCLKPR)
{
    double freq = m_mcu->freq();
    
    if (freq != m_intOscFreq/(1 << m_prIndex)) {
        // frequency was modified in UI
        // we adjust internal frequency, considering user knows what he does
        // before effectively setting new CLKPR
        
        m_intOscFreq = freq*(1 << m_prIndex);        
    }

    if (newCLKPR >= 0x80) return; // ignore CKSEL bit. TODO: CLKPR can be modified only within 4 clock after CKSEL sets, not really important
    
    m_prIndex = (newCLKPR & 0x0F);
    m_mcu->setFreq(m_intOscFreq/(1 << m_prIndex), true);
    m_psInst = m_mcu->psInst()/2; //update UI event tick
}


