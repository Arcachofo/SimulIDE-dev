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
}
PicUsart::~PicUsart(){}

void PicUsart::setup()
{
    m_sleepMode = 0xFF;
    m_sender->setSleepMode( 0xFF );
    m_receiver->setSleepMode( 0xFF );

    m_enabled = false;
    m_SPBRG  = nullptr;
    m_SPBRGL = nullptr;
    m_SPBRGH = nullptr;

    m_PIR1  = m_mcu->getReg("PIR1");
    m_TXSTA = m_mcu->getReg("TXSTA");
    m_RCSTA = m_mcu->getReg("RCSTA");

    if( m_mcu->regExist("SPBRG") ){
        m_SPBRGL = m_mcu->getReg("SPBRG");
        watchRegNames( "SPBRG",  R_WRITE, this, &PicUsart::setSPBRGL, m_mcu );
    }
    if( m_mcu->regExist("SPBRGL") ){
        m_SPBRGL = m_mcu->getReg("SPBRGL");
        watchRegNames( "SPBRGL", R_WRITE, this, &PicUsart::setSPBRGL, m_mcu );
    }
    if( m_mcu->regExist("SPBRGH") ){
        m_SPBRGH = m_mcu->getReg("SPBRGH");
        watchRegNames("SPBRGH", R_WRITE, this, &PicUsart::setSPBRGH, m_mcu );
    }

    m_SPEN = getRegBits("SPEN", m_mcu );
    m_BRGH = getRegBits("BRGH", m_mcu );

    m_bit9Tx = getRegBits("TX9D", m_mcu );
    m_bit9Rx = getRegBits("RX9D", m_mcu );

    m_txEn = getRegBits("TXEN", m_mcu );
    m_rxEn = getRegBits("CREN", m_mcu );
    m_TX9  = getRegBits("TX9", m_mcu );
    m_RX9  = getRegBits("RX9", m_mcu );
    m_ADDEN = getRegBits("ADDEN", m_mcu );

    m_TXIF = getRegBits("TXIF", m_mcu );
    m_TRMT = getRegBits("TMRT", m_mcu );
    m_RCIF = getRegBits("RCIF", m_mcu );
    m_FERR = getRegBits("FERR", m_mcu );
    m_OERR = getRegBits("OERR", m_mcu );
    m_SYNC = getRegBits("SYNC", m_mcu );
}

void PicUsart::configureA( uint8_t newTXSTA ) // TXSTA changed
{
    // clockPol = getRegBitsVal( val, CSRC );

    m_synchronous = getRegBitsBool( newTXSTA, m_SYNC );

    bool txEn = getRegBitsBool( newTXSTA, m_txEn );
    if( m_synchronous );
    else{
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
    setDataBits( getRegBitsVal( newTXSTA, m_TX9 )+8 );

    m_speedx2 = getRegBitsBool( newTXSTA, m_BRGH ); // Double Speed?
    setBaurrate();
}

void PicUsart::configureB( uint8_t newRCSTA ) // RCSTA changed
{
    m_enabled = getRegBitsVal( newRCSTA, m_SPEN );

    bool rxEn = getRegBitsVal( newRCSTA, m_rxEn );
    if( rxEn != m_receiver->isEnabled() ) m_receiver->enable( rxEn );

    bool adden = getRegBitsBool( newRCSTA, m_ADDEN );
    m_receiver->ignoreData( adden );
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
    setPeriod( mult*(SPBRG+1)*m_mcu->psInst() ); // period in picoseconds
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

void PicUsart::setRxFlags( uint16_t frame )
{
    if( dataBits() == 9 ) setBit9Rx( ( frame & (1<<8) ) ? 1 : 0 );

    writeRegBits( m_FERR, frame & frameError );  // frameError
    writeRegBits( m_OERR, frame & dataOverrun ); // overrun error
    //writeRegBits(     , frame & parityError);  // parityError
}

void PicUsart::sleep( int mode )
{
    McuModule::sleep( mode );
    if( m_sleeping ) m_sender->pauseEvents();
    else             m_sender->resumeEvents();
}
