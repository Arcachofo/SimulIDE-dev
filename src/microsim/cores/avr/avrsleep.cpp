/***************************************************************************
 *   Copyright (C) 2022 by Santiago González                               *
 *                                                                         *
 ***( see copyright.txt file at root folder )*******************************/

//#include <QDebug>

#include "avrsleep.h"
#include "e_mcu.h"
#include "mcuwdt.h"
#include "datautils.h"

AvrSleep::AvrSleep( eMcu* mcu, QString name )
        : McuSleep( mcu, name )
{
}
AvrSleep::~AvrSleep(){}

void AvrSleep::setup()
{
    m_SM = getRegBits("SM0,SM1,SM2", m_mcu );
    m_SE = getRegBits("SE", m_mcu );

    //Interrupt* inte = nullptr;

    //m_wakeUps.emplace_back( m_mcu->watchDog()->getInterrupt() );
}

void AvrSleep::initialize()
{
    m_sleepMode = sleepNone;
}

void AvrSleep::configureA( uint8_t newVal )
{
    bool enabled = getRegBitsBool( newVal, m_SE );
    if( m_enabled != enabled )
    {
        m_enabled = enabled;
        if( !enabled )
        {
            for( Interrupt* inte : m_wakeUps )
            {
                if( inte ) inte->callBack( this, false);
            }
        }
    }
    sleepMode_t mode = (sleepMode_t)getRegBitsVal( newVal, m_SM );
    if( m_sleepMode != mode )
    {
        m_sleepMode = mode;
        switch( mode ) {
            case sleepIdle: break;
            case sleepAdcNR: break;
            case sleepPowDo: break;
            case sleepPowSa: break;
            case sleepStand: break;
            case sleepExtSt: break;
        default:
            break;
        }
    }
}


/*
//------------------------------------------------------
//-- AVR ADC Type 0 ------------------------------------

AvrSleep00::AvrSleep00( eMcu* mcu, QString name )
          : AvrSleep( mcu, name )
{
    m_SM = getRegBits( "SM0,SM1,SM2", mcu );
}
AvrSleep::~AvrSleep(){}

//------------------------------------------------------
//-- AVR ADC Type 1 ------------------------------------

AvrSleep00::AvrSleep00( eMcu* mcu, QString name )
          : AvrSleep( mcu, name )
{
    m_SM = getRegBits( "SM0,SM1", mcu );
}
AvrSleep::~AvrSleep(){}
*/
