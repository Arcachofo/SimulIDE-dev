/***************************************************************************
 *   Copyright (C) 2021 by Santiago González                               *
 *                                                                         *
 ***( see copyright.txt file at root folder )*******************************/

#include "picccpunit.h"
#include "picicunit.h"
#include "picocunit.h"
#include "datautils.h"
#include "mcupin.h"
#include "e_mcu.h"
#include "simulator.h"

PicCcpUnit::PicCcpUnit( eMcu* mcu, QString name, int type )
          : McuModule( mcu, name )
          , eElement( name )
{
    QString n = name.right(1); // name="CCP1" => n="1"
    QString e = name.contains("+") ? "+" : "";

    McuTimer* timer1 = mcu->getTimer( "TIMER1" );
    McuTimer* timer2 = mcu->getTimer( "TIMER2" );

    m_capUnit = new PicIcUnit( mcu, "IC"+n );
    m_capUnit->m_timer = timer1;

    m_comUnit = new PicOcUnit( mcu, "OC"+e+n );
    m_comUnit->m_timer = timer1;
    if( timer1 ) timer1->addOcUnit( m_comUnit );

    m_pwmUnit = PicOcUnit::createPwmUnit( mcu, "PWM"+e+n, type  );
    m_pwmUnit->m_timer = timer2;
    if( timer2 ) timer2->addOcUnit( m_pwmUnit );

    m_CCPxM = getRegBits( "CCP"+n+"M0,CCP"+n+"M1,CCP"+n+"M2,CCP"+n+"M3", mcu );

    m_mode = 0;
}
PicCcpUnit::~PicCcpUnit()
{
    delete m_capUnit;
}

void PicCcpUnit::initialize()
{
    m_mode = 0;
    m_ccpMode = ccpOFF;
}

void PicCcpUnit::ccprWriteL( uint8_t val )
{
    m_pwmUnit->ocrWriteL( val );
    m_comUnit->ocrWriteL( val );
}

void PicCcpUnit::ccprWriteH( uint8_t val )
{
    if( m_ccpMode == ccpPWM ) m_mcu->m_regOverride = *m_ccpRegH; // Read only
    else                      m_comUnit->ocrWriteH( val );
}

void PicCcpUnit::configureA( uint8_t CCPxCON ) //
{
    if( CCPxCON == m_mode ) return;
    m_mode = CCPxCON;

    uint8_t CCPxM = getRegBitsVal( CCPxCON, m_CCPxM );

    m_ccpMode = ccpOFF;
    m_capUnit->initialize();
    m_comUnit->initialize();
    m_pwmUnit->initialize();

    if     ( CCPxM == 0 ) return;
    if     ( CCPxM < 4  ) { m_ccpMode = ccpCOM; m_comUnit->configure( CCPxM ); } // Compare Mode (Enhanced):
    else if( CCPxM < 8  ) { m_ccpMode = ccpCAP; m_capUnit->configure( CCPxM ); } // Capture Mode:
    else if( CCPxM < 12 ) { m_ccpMode = ccpCOM; m_comUnit->configure( CCPxM ); } // Compare Mode:
    else                  { m_ccpMode = ccpPWM; m_pwmUnit->configure( CCPxCON );}// PWM Mode
}

void PicCcpUnit::setInterrupt( Interrupt* i )
{
    m_capUnit->m_interrupt = i;
    m_comUnit->m_interrupt = i;
    m_pwmUnit->m_interrupt = i;
}

void PicCcpUnit::setPin( McuPin* pin )
{
    m_capUnit->m_icPin = pin;
    m_comUnit->m_ocPin = pin;
    m_pwmUnit->m_ocPin = pin;
}
