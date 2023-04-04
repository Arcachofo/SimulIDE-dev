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
    m_VREN = getRegBits( "VREN", mcu );
    m_VROE = getRegBits( "VROE", mcu );
    m_VRR  = getRegBits( "VRR", mcu );
    m_VR   = getRegBits( "VR0,VR1,VR2,VR3", mcu );
}
PicVref::~PicVref(){}

void PicVref::initialize()
{
    m_vref = 5.0/4; //setMode( 0 );
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
            if( vrr ) m_vref = 5.0*mode/24; /// TODO: get Vdd or use VrefP
            else      m_vref = 5.0/4+5.0*mode/32;
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
