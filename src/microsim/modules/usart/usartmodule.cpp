/***************************************************************************
 *   Copyright (C) 2020 by Santiago González                               *
 *                                                                         *
 ***( see copyright.txt file at root folder )*******************************/

#include <math.h>

#include "usartmodule.h"
#include "usarttx.h"
#include "usartrx.h"
#include "e_mcu.h"
#include "mcuinterrupts.h"
#include "circuitwidget.h"
#include "serialmon.h"
#include "datautils.h"
#include "iopin.h"
#include "simulator.h"

UsartModule::UsartModule( eMcu* mcu, QString name )
{
    m_sender   = new UartTx( this, mcu, name+"Tx" );
    m_receiver = new UartRx( this, mcu, name+"Rx" );
    m_uartSync = nullptr;

    m_mode = 0xFF; // Force first mode change.
    m_monitor = nullptr;

    m_stopBits = 1;
    m_dataBits = 8;
    m_dataMask = 0xFF;
    m_parity   = parNONE;

    m_synchronous = false;
    m_serialMon   = false;
}
UsartModule::~UsartModule( )
{
    delete m_sender;
    delete m_receiver;
    if( m_monitor ) m_monitor->close();
}

void UsartModule::setSynchronous( bool s )
{
    m_synchronous = s;
    if( !s ) return;
    if( !m_uartSync ) m_uartSync = new UartSync( this, "uartSync");

    m_uartSync->m_syncTxPin  = m_receiver->getPin();
    m_uartSync->m_syncClkPin = m_sender->getPin();
}

void UsartModule::setBaudRate( int br )
{
    m_baudRate = br;
    setPeriod( 1e12/br );
}

void UsartModule::setDataBits( uint8_t b )
{
    m_dataBits = b;
    m_dataMask = pow( 2, b )-1;
}

void UsartModule::setPeriod( uint64_t period )
{
    m_sender->setPeriod( period );
    m_receiver->setPeriod( period );
}

void UsartModule::sendByte( uint8_t data )  // Buffer is being written
{
    if( m_synchronous ) m_uartSync->sendSyncData( data );
    else                m_sender->processData( data );
}

void UsartModule::frameSent( uint8_t data )
{
    if( m_monitor ) m_monitor->printOut( data );
}

void UsartModule::byteReceived( uint8_t data )
{
    if( m_monitor ) m_monitor->printIn( data );
}

void UsartModule::openMonitor( QString id, int num, bool send )
{
    if( !m_monitor )
        m_monitor = new SerialMonitor( CircuitWidget::self(), this, send );

    if( num > 0 ) id.append(" - Uart"+QString::number(num) );
    m_monitor->setWindowTitle( id );
    m_monitor->show();
    m_serialMon = true;
}

void UsartModule::setMonitorTittle( QString t )
{
    if( m_monitor ) m_monitor->setWindowTitle( t );
}

void UsartModule::monitorClosed()
{
    m_serialMon = false;
}

//---------------------------------------
//---------------------------------------


UartSync::UartSync( UsartModule* usart, /*eMcu* mcu,*/ QString name )
        : eElement( name )
{
    m_usart = usart;

    m_syncTxPin  = nullptr;
    m_syncClkPin = nullptr;
    m_syncClkOffset = 10;
}
UartSync::~UartSync(){;}


void UartSync::runEvent() // Used in Sybchronous mode
{
    if( m_clkState )
    {
        if( ++m_currentBit <= 8 ) //sendSyncBit(); // Still transmitting
        {
            m_syncTxPin->setOutState( m_frame & 1 );
            m_frame >>= 1;
        }else{
            m_syncTxPin->setOutState( true );
            m_usart->frameSent( m_syncData );
            return;
        }
    }
    Simulator::self()->addEvent( m_syncPeriod/2, this ); // Shedule next bit
    m_clkState = !m_clkState;
    m_syncClkPin->scheduleState( m_clkState, m_syncClkOffset );
}

void UartSync::sendSyncData( uint8_t data )
{
    if( !m_syncTxPin || !m_syncClkPin || !m_syncPeriod ) return;
    m_frame = m_syncData = data;
    m_currentBit = 0;
    m_clkState = true;
    runEvent();
}

//---------------------------------------
//---------------------------------------


UartTR::UartTR( UsartModule* usart, eMcu* mcu, QString name )
      : McuModule( mcu, name )
      , eElement( name )
{
    m_usart = usart;

    m_state = usartSTOPPED;
    m_enabled = false;
}
UartTR::~UartTR( ){}

void UartTR::initialize()
{
    m_enabled = false;
}

void UartTR::configureA( uint8_t val ) // Select Pin
{
    uint8_t pinNum = getRegBitsVal( val, m_configBitsA );
    if( pinNum < m_pinList.size() ) m_ioPin = m_pinList.at( pinNum );
}

bool UartTR::getParity( uint16_t data )
{
    bool parity = false;
    for( int i=0; i<mDATABITS; ++i )
    {
        parity ^= data & 1;
        data >>= 1;
    }
    if( mPARITY == parODD ) parity ^= 1;
    return parity;
}

void UartTR::raiseInt( uint8_t data )
{
    m_data = data;
    if( m_interrupt ) m_interrupt->raise( /*data*/ );
}

void UartTR::setPins( QList<IoPin*> pinList )
{
    m_pinList = pinList;
    m_ioPin = pinList.at(0);
}
