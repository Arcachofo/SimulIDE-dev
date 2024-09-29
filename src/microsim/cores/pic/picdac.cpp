/***************************************************************************
 *   Copyright (C) 2023 by Santiago González                               *
 *                                                                         *
 ***( see copyright.txt file at root folder )*******************************/

#include <QDebug>

#include "picdac.h"
#include "e_mcu.h"
#include "mcupin.h"
#include "picvref.h"
#include "datautils.h"

PicDac::PicDac( eMcu* mcu, QString name )
      : McuDac( mcu, name )
{
}
PicDac::~PicDac(){}

void PicDac::setup()
{
    m_dacReg = nullptr;
    m_outPin = nullptr;

    m_DACEN  = getRegBits("DACEN", m_mcu );
    m_DACLPS = getRegBits("DACLPS", m_mcu );
    m_DACOE  = getRegBits("DACOE", m_mcu );
    m_DACPSS = getRegBits("DACPSS0,DACPSS1", m_mcu );
    m_DACNSS = getRegBits("DACNSS", m_mcu );

    m_DACR = getRegBits("DACR0,DACR1,DACR2,DACR3,DACR4", m_mcu );

    m_fvr = (PicVrefE*)m_mcu->vrefModule();
}

void PicDac::initialize()
{
    McuDac::initialize();

    if( m_pins.size() > 0 ) m_pRefPin = m_pins.at(0);
    else qDebug() << "PicDac::configureA: Error: NULL VREF+ Pin";

    if( m_pins.size() > 1 ) m_nRefPin = m_pins.at(1);
    else qDebug() << "PicDac::configureA: Error: NULL VREF- Pin";

    if( m_pins.size() > 2 ) m_outPin = m_pins.at(2);
    else qDebug() << "PicDac::configureA: Error: NULL Out Pin";
}

void PicDac::voltChanged()
{
    if( m_usePinP && m_pRefPin ) m_vRefP = m_pRefPin->getVoltage();
    if( m_usePinN && m_nRefPin ) m_vRefN = m_nRefPin->getVoltage();
    updtOutVolt();
}

void PicDac::configureA( uint8_t newDACCON0 ) //
{
    m_enabled = getRegBitsBool( newDACCON0, m_DACEN );
    m_daclps  = getRegBitsBool( newDACCON0, m_DACLPS );

    m_outVoltEn = getRegBitsBool( newDACCON0, m_DACOE );
    if( m_outPin )
    {
        if( m_outVoltEn ) m_outPin->setPinMode( output );
        m_outPin->controlPin( m_outVoltEn, m_outVoltEn );
    }

    m_useFVR  = false;
    m_usePinP = false;
    m_vRefP = 0;
    uint8_t dacPss = getRegBitsVal( newDACCON0, m_DACPSS );
    switch( dacPss ) {
        case 0: m_vRefP = m_mcu->vdd(); break; // VDD
        case 1: m_usePinP = true;       break; // VREF+
        case 2: m_useFVR  = true;       break; // FVR Buffer2 output
        case 3:                         break; // Reserved, do not use
    }
    m_usePinN = false;
    m_vRefN = 0;
    uint8_t dacNss = getRegBitsVal( newDACCON0, m_DACNSS );
    switch( dacNss ) {
        case 0:                    break; // VSS
        case 1: m_usePinN = true;  break; // VREF-
    }
    if( m_pRefPin ) m_pRefPin->changeCallBack( this, m_usePinP );
    if( m_nRefPin ) m_nRefPin->changeCallBack( this, m_usePinN );

    voltChanged();
}

void PicDac::outRegChanged( uint8_t val ) // DACON1 is written
{
    m_outVal = getRegBitsVal( val, m_DACR );
    updtOutVolt();
}

void PicDac::updtOutVolt()
{
    if( m_useFVR ) m_vRefP = m_fvr->getDacVref();

    if( m_enabled ) m_outVolt = ( m_vRefP-m_vRefN )*(double)m_outVal/32 + m_vRefN;
    else            m_outVolt = m_daclps ? m_vRefP : m_vRefN;

    if( m_outPin && m_outVoltEn ) m_outPin->setVoltage( m_outVolt );
}

void PicDac::callBack() // Called by FVR
{
    updtOutVolt();
}
