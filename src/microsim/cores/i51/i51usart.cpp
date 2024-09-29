/***************************************************************************
 *   Copyright (C) 2020 by Santiago González                               *
 *                                                                         *
 ***( see copyright.txt file at root folder )*******************************/

#include "i51usart.h"
#include "usarttx.h"
#include "usartrx.h"
#include "mcutimer.h"
#include "e_mcu.h"
#include "datautils.h"

#define SCON *m_scon

I51Usart::I51Usart( eMcu* mcu, QString name, int number )
        : McuUsart( mcu, name, number )
{
}
I51Usart::~I51Usart(){}

void I51Usart::setup()
{
    m_receiver->setFifoSize( 1 );
    m_stopBits = 1;
    m_dataMask = 0xFF;
    m_parity   = parNONE;

    m_timer1 = m_mcu->getTimer( "TIMER1" );

    m_scon = m_mcu->getReg( "SCON" );

    m_SM     = getRegBits( "SM1,SM0", m_mcu );
    m_bit9Tx = getRegBits( "TB8", m_mcu );
    m_bit9Rx = getRegBits( "RB8", m_mcu );
    m_SM2    = getRegBits( "SM2", m_mcu );

    m_SMOD = getRegBits( "SMOD", m_mcu );
}

void I51Usart::reset()
{
    m_sender->enable( true ); // Sender always enabled
    m_mode = 0xFF;
    m_smodDiv = false;
    m_smodVal = 0;
    m_counter = 0;
}

void I51Usart::configureA( uint8_t newSCON ) //SCON
{
    uint8_t mode = getRegBitsVal( newSCON, m_SM );
    bool     sm2 = getRegBitsBool( newSCON, m_SM2 );

    if( mode == m_mode ){
        if( mode == 2 || mode == 3 ) m_receiver->ignoreData( sm2 );
        return;
    }
    m_mode = mode;

    setSynchronous( mode == 0 );

    bool useTimer = false;

    switch( mode )
    {
        case 0:             // Synchronous 8 bit
            m_uartSync->setPeriod( m_mcu->psInst() );// Fixed baudrate 32 or 64
            m_uartSync->setClkOffset( 200*1e3 );     // 200 ns first clock edge
            sm2 = 0;
            break;
        case 1:             // Asynchronous Timer1 8 bits
            useTimer = true;
            m_dataBits = 8;
            sm2 = 0;
            break;
        case 2:             // Asynchronous MCU Clock 9 bits
            setPeriod( m_mcu->psInst() );// Fixed baudrate 32 or 64
            m_dataBits = 9;
            break;
        case 3:             // Asynchronous Timer1 9 bits
            useTimer = true;
            m_dataBits = 9;
            break;
    }
    m_receiver->ignoreData( sm2 );

    m_timer1->getInterrupt()->callBack( this, useTimer );
    if( useTimer ) setPeriod( 0 );
}

void I51Usart::configureB( uint8_t newPCON )
{
    m_smodVal = getRegBitsVal( newPCON, m_SMOD );
}

void I51Usart::sendByte( uint8_t data )
{
    if( m_mcu->state() == mcuStopped ) return;

    if( m_synchronous ) m_uartSync->sendSyncData( data );
    else                m_sender->processData( data );
}

void I51Usart::callBack()
{
    if( ++m_counter != 16 ) return;

    m_counter = 0;
    if( (m_smodVal & 1) == 0 ) // half speed
    {
        m_smodDiv = !m_smodDiv;
        if( m_smodDiv ) return;
    }
    m_sender->runEvent();
    m_receiver->runEvent();
}

void I51Usart::readByte( uint8_t ) // Reading SBUF
{
    if( m_mcu->isCpuRead() )
    {
        m_stopBitError = false;
        uint8_t data = m_receiver->getData(); // This calls I51Usart::setRxFlags()

        if( m_mode == 1 && m_stopBitError ) return; // Ignore frame
        m_mcu->m_regOverride = data;
    }
}

void I51Usart::setRxFlags( uint16_t frame )
{
    m_stopBitError = (frame & frameError) != 0;

    //writeRegBits( m_FE , frame & frameError );  // frameError
    //writeRegBits( m_DOR, frame & dataOverrun ); // overrun error
    //writeRegBits( m_UPE, frame & parityError ); // parityError
}
