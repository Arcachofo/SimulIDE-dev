/***************************************************************************
 *   Copyright (C) 2021 by Santiago González                               *
 *                                                                         *
 ***( see copyright.txt file at root folder )*******************************/

#include "mcuadc.h"
#include "mcupin.h"
#include "e_mcu.h"
#include "mcuinterrupts.h"
#include "simulator.h"

McuAdc::McuAdc( eMcu* mcu, QString name )
      : McuPrescaled( mcu, name )
      , eElement( mcu->getId()+"-"+name )
{
    m_ADCL = NULL;
    m_ADCH = NULL;

    m_fixedVref = 0;
}
McuAdc::~McuAdc(){}

void McuAdc::initialize()
{
    m_channel = 0;
    m_chOffset = 0;
    m_enabled = false;
    m_converting = false;
    m_leftAdjust = false;
}

void McuAdc::runEvent()
{
    if( m_leftAdjust ) m_adcValue <<= 6;

    if( m_ADCL ) *m_ADCL = m_adcValue & 0x00FF;
    if( m_ADCH ) *m_ADCH = (m_adcValue & 0xFF00) >> 8;

    m_interrupt->raise();
    m_converting = false;
    endConversion();
}

void McuAdc::startConversion()
{
    if( !m_enabled ) return;
    m_converting = true;

    updtVref();

    double volt = m_adcPin[m_channel+m_chOffset]->getVoltage() - m_vRefN;
    if( volt < 0       ) volt = 0;
    if( volt > m_vRefP ) volt = m_vRefP;

    m_adcValue = (double)m_maxValue*volt/(m_vRefP-m_vRefN);
    if( m_adcValue > m_maxValue ) m_adcValue = m_maxValue;

    Simulator::self()->addEvent( m_convTime, this );
}

void McuAdc::updtVref()
{
    m_vRefP = 5;
    m_vRefN = 0;
}

void McuAdc::specialConv()
{
    qDebug() << "McuAdc::specialConv Error";
}
