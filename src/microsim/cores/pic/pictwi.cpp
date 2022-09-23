/***************************************************************************
 *   Copyright (C) 2021 by Santiago González                               *
 *                                                                         *
 ***( see copyright.txt file at root folder )*******************************/

#include "pictwi.h"
#include "mcupin.h"
#include "e_mcu.h"
#include "mcuinterrupts.h"
#include "datautils.h"

PicTwi::PicTwi( eMcu* mcu, QString name )
      : McuTwi( mcu, name )
{
    /*m_TWCR = mcu->getReg( "TWCR" );
    //m_TWSR = mcu->getReg( "TWSR" );

    m_TWEN  = getRegBits( "TWEN", mcu );
    m_TWWC  = getRegBits( "TWWC", mcu );
    m_TWSTO = getRegBits( "TWSTO", mcu );
    m_TWSTA = getRegBits( "TWSTA", mcu );
    m_TWEA  = getRegBits( "TWEA", mcu );
    m_TWINT = getRegBits( "TWINT", mcu );*/
}
PicTwi::~PicTwi(){}

void PicTwi::initialize()
{
    McuTwi::initialize();
    m_bitRate = 0;
}

void PicTwi::configureA( uint8_t newTWCR )
{
    //bool oldEn  = getRegBitsBool( *m_TWCR, m_TWEN );
    //bool enable = getRegBitsBool( newTWCR, m_TWEN );


}

void PicTwi::configureB( uint8_t val )
{
}

void PicTwi::writeAddrReg( uint8_t newTWAR )
{
    //m_genCall = newTWAR & 1;
    //m_address = newTWAR >> 1;
}

void PicTwi::writeStatus( uint8_t newTWSR )
{
    newTWSR &= 0b00000011;
    uint8_t prescaler = m_prescList[newTWSR];
    if( m_prescaler != prescaler ) { m_prescaler = prescaler; updateFreq(); }
    /// Done by masking //m_mcu->m_regOverride = newTWSR | (*m_statReg & 0b11111100); // Preserve Status bits
}

void PicTwi::writeTwiReg(uint8_t newTWDR ) // TWDR is being written
{
    if( m_mode != TWI_MASTER ) return;


    //masterWrite( newTWDR, isAddr, write );       /// Write data or address to Slave
}

void PicTwi::setTwiState( twiState_t state )  // Set new AVR Status value
{
    TwiModule::setTwiState( state );


}

void PicTwi::updateFreq()
{
    //double freq = m_mcu->freqMHz()*1e6/(16+2*m_bitRate*m_prescaler);
    //setFreqKHz( freq/1e3 );
}
