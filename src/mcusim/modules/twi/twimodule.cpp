/***************************************************************************
 *   Copyright (C) 2021 by santiago González                               *
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

#include "twimodule.h"
#include "twimaster.h"
#include "twislave.h"
#include "e-source.h"

TwiModule::TwiModule( bool master, bool slave, QString name )
{
    m_sda = NULL;
    m_scl = NULL;

    if( master ) m_master = new TwiMaster( this, name+"_Master" );
    else         m_master = NULL;

    if( slave ) m_slave = new TwiSlave( this, name+"_Slave" );
    else        m_slave = NULL;
}

TwiModule::~TwiModule( )
{
}


//--------------------------------------------------------
//--------------------------------------------------------

TwiTR::TwiTR( TwiModule* twi, QString name )
     : eClockedDevice( name )
{
    m_twi = twi;

    //m_state = usartSTOPPED;
    m_enabled = false;
}
TwiTR::~TwiTR( ){}


void TwiTR::setSDA( bool state )
{
    m_twi->sda()->setState( state );
}

void TwiTR::setSCL( bool state )
{
    m_twi->scl()->setState( state );
}

bool TwiTR::getSdaState()
{
    double volt = m_twi->sda()->getVolt();
    bool  state = m_sdaState;

    if     ( volt > m_inputHighV ) state = true;
    else if( volt < m_inputLowV )  state = false;

    m_twi->sda()->getPin()->setPinState( state? input_high:input_low ); // High : Low colors

    if( m_twi->sda()->isInverted() ) state = !state;
    m_sdaState = state;

    return state;
}

void TwiTR::readBit()
{
    if( m_bitPtr > 0 ) m_rxReg <<= 1;
    m_rxReg += m_sdaState;                  //Read one bit from sda
    m_bitPtr++;
}

void TwiTR::writeBit()
{
    if( m_bitPtr < 0 ) { waitACK(); return; }

    bool bit = m_txReg>>m_bitPtr & 1;
    m_bitPtr--;

    if( bit != m_sdaState ) setSDA( bit ); // Save some events
}

void TwiTR::readByte()
{
    m_bitPtr = 0;
    ACK();
}

void TwiTR::writeByte()
{
    m_bitPtr = 7;
}

void TwiTR::waitACK()
{
    setSDA( true );
    m_lastState = m_state;
    m_state = I2C_WAITACK;
}

void TwiTR::ACK()
{
    m_lastState = m_state;
    m_state = I2C_ACK;
}
