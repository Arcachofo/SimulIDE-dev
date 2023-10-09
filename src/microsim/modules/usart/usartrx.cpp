/***************************************************************************
 *   Copyright (C) 2021 by Santiago González                               *
 *                                                                         *
 ***( see copyright.txt file at root folder )*******************************/

#include "usartrx.h"
#include "mcuinterrupts.h"
#include "iopin.h"
#include "simulator.h"
#include "connector.h"

UartRx::UartRx( UsartModule* usart, eMcu* mcu, QString name )
      : UartTR( usart, mcu, name )
{
    m_period = 0;
    m_ignoreData = false;
}
UartRx::~UartRx( ){}

void UartRx::enable( uint8_t en )
{
    bool enabled = en > 0;
    if( enabled == m_enabled ) return;
    m_enabled = enabled;

    if( enabled )  // Start reading
    {
        m_state = usartIDLE;
        m_framesize = 1+mDATABITS+mPARITY+mSTOPBITS;
        m_currentBit = 0;
        m_fifoP = 2;
        m_startHigh = m_ioPin->getInpState();
    }else{
        m_state = usartSTOPPED;
        Simulator::self()->cancelEvents( this );
    }

    m_ioPin->changeCallBack( this, enabled );  // Wait for start bit if enabled
    m_frame = 0;
}

void UartRx::voltChanged()
{
    if( !m_enabled || m_sleeping ) return;

    bool bit = m_ioPin->getInpState();

    /// if( m_state == usartRXEND ) rxEnd(); ???

    if     ( !m_startHigh &&  bit ) m_startHigh = true;
    else if(  m_startHigh && !bit )                     // Start bit detected
    {
        m_state = usartRECEIVE;
        m_ioPin->changeCallBack( this, false );
        if( m_period ) Simulator::self()->addEvent( m_period/2, this ); // Shedule reception
    }
}

void UartRx::runEvent()
{
    if( m_state == usartRECEIVE ) readBit();
}

void UartRx::readBit()
{
    bool bit = m_ioPin->getInpState();

    if( bit ){
        if( m_currentBit == 0 ){ rxEnd(); return; } // Start bit error
        m_frame += 1<<m_currentBit;                 // Get bit into frame
    }
    if( ++m_currentBit == m_framesize )
    {
        m_frame >>= 1;           // Remove Start bit
        byteReceived( m_frame );
        rxEnd();
    }
    else if( m_period ) Simulator::self()->addEvent( m_period, this );
}

void UartRx::rxEnd()
{
    m_currentBit = 0;
    m_frame = 0;

    m_state = usartIDLE;
    m_ioPin->changeCallBack( this, true ); // Wait for next start bit

    if( m_period ) Simulator::self()->cancelEvents( this );
}

void UartRx::byteReceived( uint16_t frame )
{
    if( mDATABITS == 9 && m_ignoreData && (frame & 1<<8) == 0 ) return; // Multi-proccesor data frame

    if( m_fifoP == 0 )                                  // Overrun error
    {
        m_fifoP = 1;                                    // Overwite FIFO
        frame |= dataOverrun;
    }
    if( (frame & 1<<(mDATABITS+mPARITY)) == 0 )         // Check Stop bit
        frame |= frameError;                            // Frame Error: wrong stop bit

    if( mPARITY > parNONE )                             // Check Parity bit
    {
        bool parity = getParity( frame );
        bool parityBit = frame & 1<<mDATABITS;
        if( parity != parityBit ) frame |= parityError; // Parity error
    }
    m_fifoP--;
    m_fifo[m_fifoP] = frame;

    if( m_fifoP == 1 ) setRxFlags();

    m_usart->byteReceived( frame & mDATAMASK );
}

uint8_t UartRx::getData()
{
    if( m_fifoP == 2 ) return 0;  // No data available

    uint16_t frame = m_fifo[1];
    uint8_t  data = frame & mDATAMASK;

    if( ++m_fifoP == 1 )
    {
        m_fifo[1] = m_fifo[0];    // Advance fifo
        setRxFlags();
    }
    else if( m_interrupt ) m_interrupt->clearFlag(); // Fifo empty

    return data;
}

void UartRx::setRxFlags()
{
    uint16_t frame = m_fifo[1];
    if( mDATABITS == 9 ) m_usart->setBit9Rx( ( frame & (1<<8) ) ? 1 : 0 );

    m_usart->setRxFlags( frame );         // New data in Rx Reg, set new flags
    if( m_interrupt ) m_interrupt->raise();
}

