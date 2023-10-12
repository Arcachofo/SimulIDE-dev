/***************************************************************************
 *   Copyright (C) 2023 by Santiago González                               *
 *                                                                         *
 ***( see copyright.txt file at root folder )*******************************/

#include <QDebug>

#include "picdac.h"
#include "e_mcu.h"
#include "mcupin.h"
#include "datautils.h"

PicDac::PicDac( eMcu* mcu, QString name )
      : McuDac( mcu, name )
{
    m_dacReg = NULL;
    m_outPin = NULL;

    m_DACEN  = getRegBits( "DACEN", mcu );
    m_DACLPS = getRegBits( "DACLPS", mcu );
    m_DACOE  = getRegBits( "DACOE", mcu );
    m_DACPSS = getRegBits( "DACPSS0,DACPSS1", mcu );
    m_DACNSS = getRegBits( "DACNSS", mcu );

    m_DACR = getRegBits( "DACR", mcu );
}
PicDac::~PicDac(){}

void PicDac::initialize()
{
    McuDac::initialize();

    if( m_pins.size() == 0 ) return;
    m_pRefPin = m_pins.at(0);

    if( m_pins.size() > 1 )
    {
        m_nRefPin = m_pins.at(1);
        if( m_pins.size() > 2 ) m_outPin = m_pins.at(2);
    }
}

void PicDac::configureA( uint8_t newDACCON1 ) // ADCON0
{
    m_enabled   = getRegBitsBool( newDACCON1, m_DACEN );
    m_daclps    = getRegBitsBool( newDACCON1, m_DACLPS );
    m_outVoltEn = getRegBitsBool( newDACCON1, m_DACOE );

    m_vRefP = 0;
    uint8_t dacPss = getRegBitsVal( newDACCON1, m_DACPSS );
    switch( dacPss ) {
        case 0: m_vRefP = 5;                       break; // VDD
        case 1: {                                         // VREF+
            if( m_pRefPin ) m_vRefP = m_pRefPin->getVoltage();
            else qDebug() << "PicDac::configureA: Error: NULL VREF+ Pin";
        } break;
        case 2:                                    break; /// TODO: FVR Buffer2 output
        case 3:                                    break; // Reserved, do not use
    }
    m_vRefN = 0;
    uint8_t dacNss = getRegBitsVal( newDACCON1, m_DACNSS );
    switch( dacNss ) {
        case 0:                                    break; // VSS
        case 1: {                                         // VREF-
            if( m_nRefPin ) m_vRefN = m_nRefPin->getVoltage();
            else qDebug() << "PicDac::configureA: Error: NULL VREF- Pin";
        } break;
    }
    updtOutVolt();
}

void PicDac::outRegChanged( uint8_t val ) // ADCON1 is written
{
    m_outVal = getRegBitsVal( val, m_DACR );
    updtOutVolt();
}

void PicDac::updtOutVolt()
{
    if( m_enabled ) m_outVolt = ( m_vRefP-m_vRefN )*m_outVal/2e5 + m_vRefN;
    else            m_outVolt = m_daclps ? m_vRefP : m_vRefN;

    if( m_outPin ){
        if( m_outVoltEn ) m_outPin->setVoltage( m_outVolt );
    }
    else qDebug() << "PicDac::outRegChanged: Error: NULL Out Pin";

}
