/***************************************************************************
 *   Copyright (C) 2021 by Santiago González                               *
 *                                                                         *
 ***( see copyright.txt file at root folder )*******************************/

#include "picport.h"
#include "picpin.h"
#include "mcu.h"
#include "e_mcu.h"
#include "datautils.h"

PicPort::PicPort( eMcu* mcu, QString name )
       : McuPort( mcu, name )
{
    m_sleepMode = 0xFF;
}
PicPort::~PicPort(){}

void PicPort::configureA( uint8_t newANSEL ) // Analog pins
{
    for( int i=0; i<m_numPins; ++i )
        m_pins[i]->setAnalog( newANSEL & 1<<i );
}

void PicPort::outChanged( uint8_t val )
{
    McuPort::outChanged( val );
    if( m_interrupt ) m_interrupt->clearFlag(); // Clear interrupt fag
}

void PicPort::readPort( uint8_t )
{
    m_mcu->m_regOverride = m_pinState;
    if( m_interrupt ) m_interrupt->clearFlag(); // Clear interrupt fag
}

McuPin* PicPort::createPin( int i, QString id , Component* mcu )
{
    return new PicPin( this, i, id, mcu );
}
