/***************************************************************************
 *   Copyright (C) 2020 by Santiago González                               *
 *                                                                         *
 ***( see copyright.txt file at root folder )*******************************/

#include "picusart.h"
#include "usarttx.h"
#include "usartrx.h"
#include "e_mcu.h"
#include "iopin.h"
#include "serialmon.h"
#include "datautils.h"
#include "regwatcher.h"

PicUsart::PicUsart( eMcu* mcu,  QString name, int number )
        : McuUsart( mcu, name, number )
{
    m_enabled = false;
    m_SPBRG  = NULL;
    m_SPBRGL = NULL;
    m_SPBRGH = NULL;

    m_PIR1  = mcu->getReg( "PIR1" );
    m_TXSTA = mcu->getReg( "TXSTA" );
    m_RCSTA = mcu->getReg( "RCSTA" );

    if( mcu->regExist( "SPBRG") ){
        m_SPBRGL = mcu->getReg( "SPBRG" );
        watchRegNames( "SPBRG",  R_WRITE, this, &PicUsart::setSPBRGL, mcu );
    }
    if( mcu->regExist( "SPBRGL") ){
        m_SPBRGL = mcu->getReg( "SPBRGL" );
        watchRegNames( "SPBRGL", R_WRITE, this, &PicUsart::setSPBRGL, mcu );
    }
    if( mcu->regExist( "SPBRGH") ){
        m_SPBRGH = mcu->getReg( "SPBRGH" );
        watchRegNames( "SPBRGH", R_WRITE, this, &PicUsart::setSPBRGH, mcu );
    }

    m_SPEN = getRegBits( "SPEN", mcu );
    m_BRGH = getRegBits( "BRGH", mcu );

    m_bit9Tx = getRegBits( "TX9D", mcu );
    m_bit9Rx = getRegBits( "RX9D", mcu );

    m_txEn = getRegBits( "TXEN", mcu );
    m_rxEn = getRegBits( "CREN", mcu );
    m_TX9  = getRegBits( "TX9", mcu );
    m_RX9  = getRegBits( "RX9", mcu );
    m_TXIF = getRegBits( "TXIF", mcu );
    m_TRMT = getRegBits( "TMRT", mcu );
    m_RCIF = getRegBits( "RCIF", mcu );
    m_FERR = getRegBits( "FERR", mcu );
    m_OERR = getRegBits( "OERR", mcu );
    m_SYNC = getRegBits( "SYNC", mcu );

}
PicUsart::~PicUsart(){}

void PicUsart::configureA( uint8_t newTXSTA ) // TXSTA changed
{
    // clockPol = getRegBitsVal( val, CSRC );

    m_sync = getRegBitsBool( newTXSTA, m_SYNC );

    bool txEn = getRegBitsBool( newTXSTA, m_txEn );
    //if( m_sync );
    //else
    {
        if( txEn != m_sender->isEnabled() )
        {
            setRegBits( m_TXIF );
            setRegBits( m_TRMT );
            if( txEn ){
                m_sender->getPin()->controlPin( true, true );
                m_sender->getPin()->setPinMode( output );
            }
            else m_sender->getPin()->controlPin( false, false );
            m_sender->enable( txEn );
        }
    }
    m_dataBits = getRegBitsVal( newTXSTA, m_TX9 )+8;

    m_speedx2 = getRegBitsBool( newTXSTA, m_BRGH ); // Double Speed?
    setBaurrate();
}

void PicUsart::configureB( uint8_t newRCSTA ) // RCSTA changed
{
    m_enabled = getRegBitsVal( newRCSTA, m_SPEN );

    bool rxEn = getRegBitsVal( newRCSTA, m_rxEn );
    if( rxEn != m_receiver->isEnabled() ) m_receiver->enable( rxEn );
}

void PicUsart::setSPBRGL(  uint8_t val )
{
    *m_SPBRGL = val;
    setBaurrate();
}

void PicUsart::setSPBRGH(  uint8_t val )
{
    *m_SPBRGH = val;
    setBaurrate();
}

void PicUsart::setBaurrate( uint8_t )
{
    uint64_t mult = 16;
    if( m_speedx2 ) mult = 4;
    uint16_t SPBRG = *m_SPBRGL;
    if( m_SPBRGH ) SPBRG |= (uint16_t)*m_SPBRGH << 8;
    setPeriod( mult*(SPBRG+1)*m_mcu->psCycle() ); // period in picoseconds
}

uint8_t PicUsart::getBit9Tx()
{
    return getRegBitsVal( *m_TXSTA, m_bit9Tx );
}

void PicUsart::setBit9Rx( uint8_t bit )
{
    if( bit ) setRegBits( m_bit9Rx );
    else      clearRegBits( m_bit9Rx );
}

void PicUsart::sendByte(  uint8_t data )
{
    if( getRegBitsBool( *m_PIR1, m_TXIF ) )  // TXREG is empty
    {
        m_interrupt->clearFlag();            // TXREG full: Clear TXIF bit
        m_sender->processData( data );
}   }

void PicUsart::frameSent( uint8_t data ) // TSR is empty
{
    if( m_monitor ) m_monitor->printOut( data );

    if( getRegBitsBool( *m_PIR1, m_TXIF ) )  // TXREG is empty, no data to load
        clearRegBits( m_TRMT );              // Clear TMRT bit
    else
        m_sender->startTransmission();       // TXREG contains data, send it
}

void PicUsart::bufferEmpty()
{
    m_interrupt->raise(); // USART Data Register Empty Interrupt
    setRegBits( m_TRMT ); // Set TMRT bit
}

void PicUsart::overrunError()
{
    setRegBits( m_OERR );
}

void PicUsart::parityError()
{
    //setRegBits( );
}

void PicUsart::frameError()
{
    setRegBits( m_FERR );
}
