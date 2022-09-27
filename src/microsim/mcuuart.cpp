/***************************************************************************
 *   Copyright (C) 2020 by Santiago González                               *
 *                                                                         *
 ***( see copyright.txt file at root folder )*******************************/

#include "mcuuart.h"
#include "serialmon.h"
#include "usartrx.h"
#include "usarttx.h"
#include "e_mcu.h"
#include "mcuinterrupts.h"

McuUsart::McuUsart( eMcu* mcu, QString name, int number )
        : McuModule( mcu, name )
        , UsartModule( mcu, mcu->getId()+"-"+name )
{
    m_number = number;
}
McuUsart::~McuUsart( ){}

void McuUsart::bufferEmpty()
{
    if( m_interrupt ) m_interrupt->raise(); // USART Data Register Empty Interrupt
}

void McuUsart::frameSent( uint8_t data )
{
    if( m_monitor ) m_monitor->printOut( data );
    m_sender->raiseInt();
}

void McuUsart::readByte( uint8_t )
{
    m_mcu->m_regOverride = m_receiver->getData();
}
