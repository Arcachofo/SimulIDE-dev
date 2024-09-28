/***************************************************************************
 *   Copyright (C) 2020 by Santiago González                               *
 *                                                                         *
 ***( see copyright.txt file at root folder )*******************************/

#include "avrocunit.h"
#include "datautils.h"
#include "mcupin.h"
#include "mcuocm.h"
#include "e_mcu.h"

AvrOcUnit::AvrOcUnit( eMcu* mcu, QString name )
         : McuOcUnit( mcu, name )
{
}
AvrOcUnit::~AvrOcUnit( ){}

void AvrOcUnit::initialize()
{
    McuOcUnit::initialize();
    m_OCRXmask = 0xFFFF;
}

void AvrOcUnit::configure( uint8_t val ) // COMNX0,COMNX1
{
    m_mode = getRegBitsVal( val, m_configBitsA );

    bool enabled = m_mode > 0;
    if( m_enabled == enabled ) return;
    m_enabled = enabled;

    m_ocPin->controlPin( enabled, false );
    if( enabled ) m_ocPin->setOutState( false );  // OC Pin connected

    if( m_ocm ) m_ocm->setOcActive( this, enabled );
    else        m_ctrlPin = enabled;
}

void AvrOcUnit::setPinSate( bool state, uint64_t time )
{
    if( m_ctrlPin )
    {
        m_ocPin->scheduleState( state, time );
        if( m_ocPinInv ) m_ocPinInv->scheduleState( !state, time );
    }
    else if( m_ocm ) m_ocm->setState( this, state );
}

void AvrOcUnit::ocrWriteL( uint8_t val )
{
    val &= m_OCRXmask;
    McuOcUnit::ocrWriteL( val );
    m_mcu->m_regOverride = val;
}

void AvrOcUnit::ocrWriteH( uint8_t val )
{
    val &= (m_OCRXmask>>8);
    McuOcUnit::ocrWriteH( val );
    m_mcu->m_regOverride = val;
}

void AvrOcUnit::setOcrMask( uint16_t mask )
{
    m_OCRXmask = mask;

    *m_ocRegL &= mask;
    m_comMatch = *m_ocRegL;
    if( m_ocRegH )
    {
        *m_ocRegH &= (mask>>8);
        m_comMatch |= (uint16_t)*m_ocRegH<<8;
    }
}
