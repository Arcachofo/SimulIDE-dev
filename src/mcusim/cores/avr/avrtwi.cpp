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

#include "avrtwi.h"
#include "mcupin.h"
#include "e_mcu.h"

AvrTwi::AvrTwi( eMcu* mcu, QString name )
      : McuTwi( mcu, name )
{
    m_TWCR = mcu->getReg( "TWCR" );
    //m_TWSR = mcu->getReg( "TWSR" );

    m_TWEN  = mcu->getRegBits( "TWEN" );
    m_TWWC  = mcu->getRegBits( "TWWC" );
    m_TWSTO = mcu->getRegBits( "TWSTO" );
    m_TWSTA = mcu->getRegBits( "TWSTA" );
    m_TWEA  = mcu->getRegBits( "TWEA" );
    m_TWINT = mcu->getRegBits( "TWINT" );
}
AvrTwi::~AvrTwi(){}

void AvrTwi::initialize()
{
    McuTwi::initialize();
    //*m_TWSR= 0xF8; /// TODO: reset value is overriden
    m_bitRate = 0;
}

void AvrTwi::configureA( uint8_t newTWCR ) // TWCR is being written
{
    bool oldEn  = getRegBitsVal( *m_TWCR, m_TWEN );
    bool enable = getRegBitsVal( newTWCR, m_TWEN );

    if( oldEn && !enable )                 /// Disable TWI
    {
        setMode( TWI_OFF );
        m_sda->controlPin( false, false ); // Release control of MCU PIns
        m_scl->controlPin( false, false );
    }
    if( !enable ) return;           // Not enabled, do nothing

    if( !oldEn )                           /// Enable TWI if it was disabled
    {
        m_sda->setPinMode( open_col );
        m_sda->controlPin( true, true ); // Get control of MCU PIns

        m_scl->setPinMode( open_col );
        m_scl->controlPin( true, true );
    }

    bool clearTwint = getRegBitsVal( newTWCR, m_TWINT );
    if( clearTwint )                       /// Writting 1 to TWINT clears the flag
    {
        m_mcu->m_regOverride = newTWCR & ~m_TWINT.mask; // Clear TWINT flag
    }

    bool oldStop = getRegBitsVal( *m_TWCR, m_TWSTO );
    bool newStop = getRegBitsVal( newTWCR, m_TWSTO );

    if( newStop && !oldStop )              /// Generate Stop Condition
    {
        if( m_mode == TWI_MASTER ) // Master: Stop if I2C was started
        {
            if( getStaus() < TWI_NO_STATE ) I2Cstop();
        }
        else setMode( TWI_SLAVE ); // Slave: Stop Cond restarts Slave mode (can be used to recover from an error condition)
    }

    bool oldStart = getRegBitsVal( *m_TWCR, m_TWSTA );
    bool newStart = getRegBitsVal( newTWCR, m_TWSTA );

    /// TODO if Stop and Start at same time, then Start Condition should be sheduled
    if( newStart && !oldStart )            /// Generate Start Condition
    {
        if( m_mode != TWI_MASTER ) setMode( TWI_MASTER );
        masterStart();
    }
    bool ack = getRegBitsVal( newTWCR, m_TWEA );
    if( ack && !clearTwint && !newStop && !newStart )
    {
        if( m_mode != TWI_SLAVE ) setMode( TWI_SLAVE );
        return;
    }

    if( m_mode == TWI_MASTER )
    {
        bool data = clearTwint && !newStop && !newStart; // No start or stop and TWINT cleared, receive data
        if( !data ) return;

        if( (m_twiState == TWI_MRX_ADR_ACK)    // We sent Slave Address + R and received ACK
         || (m_twiState == TWI_MRX_DATA_ACK) ) // We sent data and received ACK
        {
            masterRead( ack );  // Read a byte and send ACK/NACK
        }
    }
    else if( m_mode == TWI_SLAVE )
    {
        bool data = clearTwint && !newStop; // No stop and TWINT cleared, receive data
        if( !data ) return;
        m_sendACK = ack;
    }
}

void AvrTwi::configureB( uint8_t val ) // TWBR is being written
{
    m_bitRate = val;
    updateFreq();
}

void AvrTwi::writeAddrReg( uint8_t newTWAR ) // TWAR is being written
{
    m_genCall = newTWAR & 1;
    m_address = newTWAR >> 1;
}

void AvrTwi::writeStatus( uint8_t newTWSR ) // TWSR Status Register is being written
{
    newTWSR &= 0b00000011;
    m_prescaler = m_prescList[newTWSR];
    updateFreq();

    m_mcu->m_regOverride = newTWSR | (*m_statReg & 0b11111100); // Preserve Status bits
}

void AvrTwi::writeTwiReg(uint8_t newTWDR ) // TWDR is being written
{
    if( m_mode != TWI_MASTER ) return;

    bool twint = getRegBitsVal( *m_TWCR, m_TWINT ); // Check if TWINT is set
    if( !twint )                  // If not, the access will be discarded
    {
        setRegBits( m_TWWC ); // set Write Collision bit TWWC
        return;
    }
    bool write = false;
    bool isAddr = (getStaus() == TWI_START
                || getStaus() == TWI_REP_START); // We just sent Start, so this must be slave address

    if( isAddr ) write = ((newTWDR & 1) == 0);       // Sendind address for Read or Write?

    masterWrite( newTWDR, isAddr, write );       /// Write data or address to Slave
}

void AvrTwi::setTwiState( twiState_t state )  // Set new AVR Status value
{
    TwiModule::setTwiState( state );

    *m_statReg &= 0b00000111;      // Clear old state
    *m_statReg |= state;           // Write new state
    interrupt.emitValue(1);

    if( (state == TWI_NO_STATE) && (m_i2cState == I2C_STOP) ) // Stop Condition sent
    {
        clearRegBits( m_TWSTO ); // Clear TWSTO bit
    }
    else if( (state == TWI_MRX_DATA_ACK) || (state == TWI_MRX_DATA_NACK) ) // Data received
    {
        *m_dataReg = m_rxReg; // Save data received into TWDR
    }
}

void AvrTwi::updateFreq()
{
    double freq = m_mcu->freqMHz()*1e6/(16+2*m_bitRate*m_prescaler);
    if( freq != m_freq ) setFreqKHz( freq/1e3 );
}
