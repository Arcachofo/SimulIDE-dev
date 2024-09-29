/***************************************************************************
 *   Copyright (C) 2021 by Santiago González                               *
 *                                                                         *
 ***( see copyright.txt file at root folder )*******************************/

#include "picvref.h"
#include "datautils.h"
#include "e_mcu.h"
#include "mcupin.h"

PicVref::PicVref( eMcu* mcu, QString name )
       : McuVref( mcu, name )
{
}
PicVref::~PicVref(){}

void PicVref::setup()
{
    m_VREN = getRegBits("VREN", m_mcu );
    m_VROE = getRegBits("VROE", m_mcu );
    m_VRR  = getRegBits("VRR" , m_mcu );
    m_VR   = getRegBits("VR0,VR1,VR2,VR3", m_mcu );
}

void PicVref::initialize()
{
    m_vref = m_mcu->vdd()/4; //setMode( 0 );
}

void PicVref::configureA( uint8_t newVRCON )
{
    m_enabled = getRegBitsBool( newVRCON, m_VREN );

    bool     vrr = getRegBitsBool( newVRCON, m_VRR );
    uint8_t mode = getRegBitsVal( newVRCON, m_VR );
    if( mode != m_mode || vrr != m_vrr )
    {
        m_mode = mode;
        m_vrr = vrr;

        if( m_enabled ){
            double vdd = m_mcu->vdd();
            if( vrr ) m_vref = vdd*mode/24; /// TODO: get Vdd or use VrefP
            else      m_vref = vdd/4+vdd*mode/32;
        }
        else m_vref = 0;
    }
    bool vroe = getRegBitsBool( newVRCON, m_VROE );
    if( vroe != m_vroe )    // VDD-┬-8R-R-..16 Stages..-R-8R-┬-GND
    {                       // VrP-┘                     -VRR┴-VrN
        m_vroe = vroe;      /// TODO: Add VrefP/VrefN option to ladder
        if( m_pinOut ){
            double vddAdmit = 0;
            double gndAdmit = 0;
            if( vroe && m_enabled )
            {
                vddAdmit = 8+16-m_mode;
                if( !vrr ) gndAdmit = 8;
                gndAdmit += m_mode;
            }
            m_pinOut->setExtraSource( vddAdmit, gndAdmit );
        }
    }
    if( !m_callBacks.isEmpty() )
    { for( McuModule* mod : m_callBacks ) mod->callBackDoub( m_vref ); }
}

/*void PicVref::setMode( uint8_t mode )
{
    m_mode = mode;
}*/

//-------------------------------------------------------------
// Type 0: 16f1826 FVR  ---------------------------------------

PicVrefE::PicVrefE( eMcu* mcu, QString name )
        : McuVref( mcu, name )
{
}
PicVrefE::~PicVrefE(){}

void PicVrefE::setup()
{
    m_FVREN  = getRegBits("FVREN", m_mcu );
    m_ADFVR  = getRegBits("ADFVR0,ADFVR1", m_mcu );   // ADC Vref
    m_CDAFVR = getRegBits("CDAFVR0,CDAFVR1", m_mcu ); // DAC Vref
}

void PicVrefE::configureA( uint8_t newFVRCON )
{
    m_enabled = getRegBitsVal( newFVRCON, m_FVREN );

    double vdd = m_mcu->vdd();
    uint8_t adfvr = getRegBitsVal( newFVRCON, m_ADFVR );
    switch( adfvr ) {
        case 0: m_adcVref = 0.000; break;
        case 1: m_adcVref = 1.024; break;
        case 2: m_adcVref = 2.048; break;
        case 3: m_adcVref = 4.096; break;
    }
    if( m_adcVref > vdd ) m_adcVref = vdd;

    uint8_t cdafvr = getRegBitsVal( newFVRCON, m_CDAFVR );
    switch( cdafvr ) {
        case 0: m_dacVref = 0.000; break;
        case 1: m_dacVref = 1.024; break;
        case 2: m_dacVref = 2.048; break;
        case 3: m_dacVref = 4.096; break;
    }
    if( m_dacVref > vdd ) m_dacVref = vdd;

    if( !m_callBacks.isEmpty() )
    { for( McuModule* mod : m_callBacks ) mod->callBack(); }
}

double PicVrefE::getAdcVref()
{
    if( m_enabled ) return m_adcVref;
    else            return 0;
}

double PicVrefE::getDacVref()
{
    if( m_enabled ) return m_dacVref;
    else            return 0;
}
