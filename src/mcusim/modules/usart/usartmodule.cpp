/***************************************************************************
 *   Copyright (C) 2020 by santiago González                               *
 *   santigoro@gmail.com                                                   *
 *                                                                         *
 *   This program is free software; you can redistribute it and/or modify  *
 *   it under the terms of the GNU General Public License as published by  *
 *   the Free Software Foundation; either version 3 of the License, or     *
 *   (at your option) any later version.                                   *
 *                                                                         *
 *   This program is distributed in the hope that it will be useful,       *
 *   but WITHOUT ANY WARRANTY; without even the implied warranty of        *
 *   MERCHANTABILITY or FITNESS FOR A PARTICULAR PURPOSE.  See the         *
 *   GNU General Public License for more details.                          *
 *                                                                         *
 *   You should have received a copy of the GNU General Public License     *
 *   along with this program; if not, see <http://www.gnu.org/licenses/>.  *
 *                                                                         *
 ***************************************************************************/

#include "usartmodule.h"
#include "usarttx.h"
#include "usartrx.h"
#include "mcuinterrupts.h"
#include "circuitwidget.h"
#include "serialmon.h"

UsartModule::UsartModule( eMcu* mcu, QString name )
{
    m_sender   = new UartTx( this, mcu, name+"Tx" );
    m_receiver = new UartRx( this, mcu, name+"Rx" );

    m_mode = 0xFF; // Force first mode change.
    m_monitor = NULL;
}
UsartModule::~UsartModule( )
{
    delete m_sender;
    delete m_receiver;
    if( m_monitor ) m_monitor->close();
}

void UsartModule::parityError()
{
}

void UsartModule::setPeriod( uint64_t period )
{
    m_sender->setPeriod( period );
    m_receiver->setPeriod( period );
}

void UsartModule::sendByte(  uint8_t data )  // Buffer is being written
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
}

void UsartModule::uartIn( uint8_t value )
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

bool UartTR::getParity( uint8_t data )
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
    m_interrupt->raise( data );
}

