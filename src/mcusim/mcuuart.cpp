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

#include "mcuuart.h"
#include "serialmon.h"
#include "usartrx.h"
#include "usarttx.h"

McuUsart::McuUsart( eMcu* mcu, QString name, int number )
        : McuModule( mcu, name )
        , UsartModule( mcu, name )
{
    m_number = number;
    m_mode = 0xFF; // Force first mode change.

    m_monitor = NULL;
}
McuUsart::~McuUsart( ){}

void McuUsart::sendByte(  uint8_t data )
{
    m_sender->processData( data );
}

void McuUsart::byteSent( uint8_t data )
{
    if( m_monitor ) m_monitor->printOut( data );
}

void McuUsart::byteReceived( uint8_t data )
{
    if( m_monitor ) m_monitor->printIn( data );
    m_receiver->raiseInt( data );
}

// ----------------------------------------

/*McuUsarts::McuUsarts( eMcu* mcu  )
{
    m_mcu = mcu;
}

McuUsarts::~McuUsarts(){}

void McuUsarts::remove()
{
    for( McuUsart* usart : m_usartList ) delete usart;
    m_usartList.clear();
}*/
