/***************************************************************************
 *   Copyright (C) 2020 by Santiago González                               *
 *                                                                         *
 ***( see copyright.txt file at root folder )*******************************/

#include "i51usart.h"
#include "usarttx.h"
#include "usartrx.h"
#include "mcutimer.h"
#include "e_mcu.h"
#include "i51interrupt.h"
#include "datautils.h"

#define SCON *m_scon

I51Usart::I51Usart( eMcu* mcu, QString name, int number )
        : McuUsart( mcu, name, number )
{
    m_stopBits = 1;
    m_dataMask = 0xFF;
    m_parity   = parNONE;

    //m_timerConnected = false;
    m_timer1 = mcu->getTimer( "TIMER1" );

    m_scon = mcu->getReg( "SCON" );
    m_bit9Tx = getRegBits( "TB8", mcu );
    m_bit9Rx = getRegBits( "RB8", mcu );
}
I51Usart::~I51Usart(){}

void I51Usart::configureA( uint8_t val ) //SCON
{
    uint8_t mode = val >> 6;
    if( mode == m_mode ) return;
    m_mode = mode;

    m_sender->enable( true );

    m_useTimer = false;

    switch( mode )
    {
        case 0:             // Synchronous
            /// TODO //setPeriod(  m_mcu->psCycle() );// Fixed baudrate 32 or 64
            m_dataBits = 8;
            break;
        case 1:             // Asynchronous Timer1
            m_useTimer = true;
            m_dataBits = 8;
            break;
        case 2:             // Asynchronous MCU Clock
            setPeriod(  m_mcu->psCycle() );// Fixed baudrate 32 or 64
            m_dataBits = 9;
            break;
        case 3:             // Asynchronous Timer1
            m_useTimer = true;
            m_dataBits = 9;
            break;
    }

    I51T1Int* t1Int = static_cast<I51T1Int*>( m_timer1->getInterrupt() ); //  .connect( this, &I51Usart::step );
    if( m_useTimer )
    {
        //if( !m_timerConnected )
        {
            //m_timerConnected = true;
            t1Int->setUsart( this );
        }
        setPeriod( 0 );
    }
    else t1Int->setUsart( NULL );
}

void I51Usart::step()
{
    if( !m_useTimer ) return;

    m_sender->runEvent();
    m_receiver->runEvent();
}

uint8_t I51Usart::getBit9()
{
    return getRegBitsVal( SCON, m_bit9Tx );
}

void I51Usart::setBit9( uint8_t bit )
{
    SCON &= ~m_bit9Rx.mask;
    if( bit ) SCON |= m_bit9Rx.mask;
}

