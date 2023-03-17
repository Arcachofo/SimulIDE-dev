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

UsartModule::UsartModule( eMcu* mcu, QString name )
{
    m_sender   = new UartTx( this, mcu, name+"Tx" );
    m_receiver = new UartRx( this, mcu, name+"Rx" );

    m_mode = 0xFF; // Force first mode change.
    m_monitor = NULL;

    m_stopBits = 1;
    m_dataBits = 8;
    m_dataMask = 0xFF;
    m_parity   = parNONE;

    m_serialMon = false;
}
UsartModule::~UsartModule( )
{
    delete m_sender;
    delete m_receiver;
    if( m_monitor ) m_monitor->close();
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
    m_sender->processData( data );
}

void UsartModule::frameSent( uint8_t data )
{
    if( m_monitor ) m_monitor->printOut( data );
}

void UsartModule::byteReceived( uint8_t data )
{
    if( m_monitor ) m_monitor->printIn( data );
}

void UsartModule::openMonitor( QString id, int num )
{
    if( !m_monitor )
        m_monitor = new SerialMonitor( CircuitWidget::self(), this );

    if( num > 0 ) id.append(" - Uart"+QString::number(num) );
    m_monitor->setWindowTitle( id );
    m_monitor->show();
    m_serialMon = true;
}

void UsartModule::monitorClosed()
{
    m_serialMon = false;
}

void UsartModule::uartIn( uint8_t value ) // Data sent from external source (Serial Monitor)
{
    m_receiver->queueData( value );
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
    m_runHardware = false;
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


