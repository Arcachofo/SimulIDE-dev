/***************************************************************************
 *   Copyright (C) 2020 by Santiago González                               *
 *                                                                         *
 ***( see copyright.txt file at root folder )*******************************/

#include "avrusart.h"
#include "usarttx.h"
#include "usartrx.h"
#include "e_mcu.h"
#include "iopin.h"
#include "serialmon.h"
#include "datautils.h"
#include "regwatcher.h"

AvrUsart::AvrUsart( eMcu* mcu,  QString name, int number )
        : McuUsart( mcu, name, number )
{
    QString n = m_name.right(1);
    bool ok = false;
    n.toInt( &ok );
    if( !ok ) n = "";
    m_UCSRnA = mcu->getReg( "UCSR"+n+"A" );
    m_UCSRnB = mcu->getReg( "UCSR"+n+"B" );
    m_u2xn   = getRegBits( "U2X"+n, mcu );

    m_bit9Tx = getRegBits( "TXB8"+n, mcu );
    m_bit9Rx = getRegBits( "RXB8"+n, mcu );

    m_txEn = getRegBits( "TXEN"+n, mcu );
    m_rxEn = getRegBits( "RXEN"+n, mcu );

    if( n == "" ) m_modeRB = getRegBits( "UMSEL", mcu ); // atmega8
    else          m_modeRB = getRegBits( "UMSEL"+n+"0,UMSEL"+n+"1", mcu );
    m_pariRB = getRegBits( "UPM"+n+"0,UPM"+n+"1", mcu );
    m_stopRB = getRegBits( "USBS"+n, mcu );
    m_UCSZ01 = getRegBits( "UCSZ"+n+"0,UCSZ"+n+"1", mcu );
    m_UCSZ2  = getRegBits( "UCSZ"+n+"2", mcu );

    m_UBRRnL = mcu->getReg( "UBRR"+n+"L" );
    watchRegNames( "UBRR"+n+"L", R_WRITE, this, &AvrUsart::setUBRRnL, mcu );

    if( n == "" ) m_UBRRnH = NULL; // atmega8
    else{         m_UBRRnH = mcu->getReg( "UBRR"+n+"H" );
        watchRegNames( "UBRR"+n+"H", R_WRITE, this, &AvrUsart::setUBRRnH, mcu );
    }
    m_UDRIE = getRegBits( "UDRIE"+n, mcu );
    m_UDRE  = getRegBits( "UDRE"+n, mcu );
    m_TXC   = getRegBits( "TXC"+n, mcu );
    m_RXC   = getRegBits( "RXC"+n, mcu );
    m_FE    = getRegBits( "FE"+n, mcu );
    m_DOR   = getRegBits( "DOR"+n, mcu );

    if( n == "" ) m_UPE = getRegBits( "PE", mcu );
    else          m_UPE = getRegBits( "UPE"+n, mcu );
}
AvrUsart::~AvrUsart(){}

void AvrUsart::configureA( uint8_t newUCSRnA )
{
    bool speedx2 = getRegBitsBool( newUCSRnA, m_u2xn ); // Double Speed?
    if( speedx2 == m_speedx2 ) return;
    m_speedx2 = speedx2;
    setBaurrate();
}

void AvrUsart::configureB( uint8_t newUCSRnB ) // UCSRnB changed
{
    m_ucsz2 =  getRegBitsVal( newUCSRnB, m_UCSZ2 ) <<2;
    m_dataBits = m_ucsz01+m_ucsz2+5;

    uint8_t txEn = getRegBitsVal( newUCSRnB, m_txEn );
    if( txEn != m_sender->isEnabled() )
    {
        if( txEn ){
            m_sender->getPin()->controlPin( true, true );
            m_sender->getPin()->setPinMode( output );
        }
        else m_sender->getPin()->controlPin( false, false );
        m_sender->enable( txEn );
    }

    uint8_t rxEn = getRegBitsVal( newUCSRnB, m_rxEn );
    if( rxEn != m_receiver->isEnabled() )
    {
        if( rxEn )
        {
            m_receiver->getPin()->controlPin( true, true );
            m_receiver->getPin()->setPinMode( input );
        }
        else m_receiver->getPin()->controlPin( false, false );
        m_receiver->enable( rxEn );
    }
    if( getRegBitsBool( newUCSRnB, m_UDRIE ) ) // Buffer empty Interrupt enabled?
    {
        if( getRegBitsBool( *m_UCSRnA, m_UDRE ) )  // Buffer is empty?
            bufferEmpty();                         // Trigger Buffer empty Interrupt
    }
}

void AvrUsart::configureC( uint8_t newUCSRnC ) // UCSRnC changed
{
    if( !m_UBRRnH && !(newUCSRnC & (1<<7)) ) // atmega8 Writting to UBBRH
    {
        setUBRRnH( newUCSRnC & 0x0F );
        return;
    }
    // clockPol = getRegBitsVal( val, UCPOLn );

    m_mode     = getRegBitsVal( newUCSRnC, m_modeRB );    // UMSELn1, UMSELn0
    m_stopBits = getRegBitsVal( newUCSRnC, m_stopRB )+1;  // UPMn1, UPMno
    m_ucsz01   = getRegBitsVal( newUCSRnC, m_UCSZ01 );
    m_dataBits = m_ucsz01+m_ucsz2+5;

    uint8_t par = getRegBitsVal( newUCSRnC, m_pariRB );
    if( par > 0 ) m_parity = (parity_t)(par-1);
    else          m_parity = parNONE;

    /*if( sm0 )  // modes 2 and 3
    {
        if( !sm0 ) // Mode 2
        {
            /// setPeriod(  m_mcu->psCycle() );// Fixed baudrate 32 or 64
        }
    }*/
}

void AvrUsart::setUBRRnL( uint8_t v )
{
    if( *m_UBRRnL == v ) return;
    *m_UBRRnL = v;
    setBaurrate();
}

void AvrUsart::setUBRRnH( uint8_t v )
{
    if( m_UBRRHval == v ) return;
    m_UBRRHval = v;
    setBaurrate();
}

void AvrUsart::setBaurrate( uint8_t )
{
    uint16_t ubrr = *m_UBRRnL | (m_UBRRHval & 0x0F)<<8 ;

    uint64_t period = 16*(ubrr+1)*m_mcu->psCycle();
    if( m_speedx2 ) period /= 2;

    setPeriod( period );
}

uint8_t AvrUsart::getBit9Tx()
{
    return getRegBitsVal( *m_UCSRnB, m_bit9Tx );
}

void AvrUsart::setBit9Rx( uint8_t bit )
{
    *m_UCSRnB &= ~m_bit9Rx.mask;
    if( bit ) *m_UCSRnB |= m_bit9Rx.mask;
}

void AvrUsart::sendByte(  uint8_t data ) // Buffer is being written
{
    if( !m_sender->isEnabled() ) return;

    if( getRegBitsBool( *m_UCSRnA, m_UDRE ) )  // Buffer is empty?
    {
        m_interrupt->clearFlag();//clearRegBits( m_UDRE ); // Transmit buffer now full: Clear UDREn bit
        m_sender->processData( data );
}   }

void AvrUsart::frameSent( uint8_t data )
{
    if( m_monitor ) m_monitor->printOut( data );

    if( getRegBitsBool( *m_UCSRnA, m_UDRE ) ) // Frame sent & Buffer is empty
        m_sender->raiseInt();                 // Raise USART Transmit Complete
    else
        m_sender->startTransmission();        // Buffer contains data, send it
}

void AvrUsart::overrunError()
{
    setRegBits( m_DOR );
}

void AvrUsart::parityError()
{
    setRegBits( m_UPE );
}

void AvrUsart::frameError()
{
    setRegBits( m_FE );
}
