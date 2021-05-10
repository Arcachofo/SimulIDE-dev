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
#include "e-source.h"
#include "simulator.h"

TwiModule::TwiModule( QString name )
         : eClockedDevice( name )
{
    m_twiState = TWI_NO_STATE;

    m_sda = NULL;
    m_scl = NULL;
}

TwiModule::~TwiModule( )
{
}

void TwiModule::initialize()
{
    m_sheduleSDA = false;

    m_addressBits = 7;
    m_toggleScl = false;
}

void TwiModule::stamp()      // Called at Simulation Start
{
    //if( m_input.size() == 0 ) return;
    //if( !m_enabled ) return;

    /*SDA_PIN->setState( false );
    SDA_PIN->setImp( high_imp );
    SCL_PIN->setImp( high_imp );
*/
    eClockedDevice::stamp();   // Initialize Base Class ( Clock pin is managed in eClockedDevice )

    eNode* enode = m_sda->getPin()->getEnode(); // Register for SDA voltage changes
    if( enode ) enode->voltChangedCallback( this );
}

void TwiModule::keepClocking()
{
    m_toggleScl = true;
    Simulator::self()->addEvent( m_clockPeriod/2, this );
}

void TwiModule::runEvent()
{
    if( m_sheduleSDA )
    {
        m_sda->setState( m_nextSDA );
        m_sheduleSDA = false;
    }
    /// Master step:
    if( m_state == I2C_IDLE ) return;

    clkState_t sclState = getClockState();
    bool clkLow = ((sclState == Clock_Low) | (sclState == Clock_Falling));

    if( m_toggleScl )
    {
        m_scl->setState( clkLow ); // High if is LOW, LOW if is HIGH
        m_toggleScl = false;
        return;
    }
    getSdaState();        // State of SDA pin

    Simulator::self()->addEvent( m_clockPeriod, this ); //

    twiState_t twiState;
    switch( m_state )
    {
        case I2C_IDLE: return;

        case I2C_START :               // Send Start Condition
        {
            if     ( m_sdaState ) m_sda->setState( false ); // Step 1: SDA is High, Lower it
            else if( !clkLow )                   // Step 2: SDA Already Low, Lower Clock
            {
                //if( m_comp ) m_comp->inStateChanged( TWI_MSG+TWI_COND_START ); // Set TWINT
                setTwiState( TWI_START );
                keepClocking();
            }
        }break;

        case I2C_WRITE :              // We are Writting data
        {
            if( clkLow ) writeBit();     // Set SDA while clk is Low
            keepClocking();
        }break;

        case I2C_READ:               // We are Reading data
        {
            if( !clkLow )               // Read bit while clk is high
            {
                readBit();
                if( m_bitPtr == 8 ) readByte();
            }
            keepClocking();
        }break;

        case I2C_ACK:                  // Send ACK
        {
            if( clkLow )
            {
                if( m_masterACK ) m_sda->setState( false );
                m_state = I2C_ENDACK;
            }
            keepClocking();
        }break;

        case I2C_ENDACK:              // We sent ACK, release SDA
        {
            if( clkLow )
            {
                m_sda->setState( true );
                if( /*m_comp &&*/ m_lastState == I2C_READ )
                {
                    twiState = m_masterACK ? TWI_MRX_DATA_ACK : TWI_MRX_DATA_NACK ;
                    setTwiState( twiState );
                    //m_comp->inStateChanged( TWI_MSG+TWI_COND_READ );
                    //m_comp->inStateChanged( TWI_MSG+TWI_COND_ACK+m_masterACK ); // ACK/NACK sent
                }
                m_state = I2C_IDLE;
            }
            else keepClocking();
        }break;

        case I2C_READACK:            // Read ACK
        {
            if( m_isAddr ) // ACK after sendind Slave address
                twiState = m_sdaState ? TWI_MTX_DATA_NACK : TWI_MTX_DATA_ACK;

            else           // ACK after sendind data
                twiState = m_sdaState ? TWI_MTX_ADR_NACK : TWI_MTX_ADR_ACK;

            setTwiState( twiState );
            //if( m_comp ) m_comp->inStateChanged( TWI_MSG+TWI_COND_ACK+ack );

            m_state = I2C_IDLE;
            keepClocking();
        }break;

        case I2C_STOP:           // Send Stop Condition
        {
            if     (  m_sdaState && clkLow )  m_sda->setState( false ); // Step 1: Lower SDA
            else if( !m_sdaState && clkLow )  keepClocking();  // Step 2: Raise Clock
            else if( !m_sdaState && !clkLow ) m_sda->setState( true );  // Step 3: Raise SDA
            else if(  m_sdaState && !clkLow )                  // Step 4: Operation Finished
            {
                m_state = I2C_IDLE;
                setTwiState( TWI_NO_STATE );
                //if( m_comp ) m_comp->inStateChanged( 128+I2C_STOPPED ); // Set TWINT ( set to 0 )
            }
        }
    }
}

void TwiModule::voltChanged() // Used by slave
{
    if( !m_enabled ) return;

    int  sclState = getClockState(); // Get Clk to don't miss any clock changes
    getSdaState();        // State of SDA pin

    if(( sclState == Clock_High )&&( m_state != I2C_ACK ))
    {
        if( m_lastSDA && !m_sdaState ) {     // We are in a Start Condition
            m_bitPtr = 0;
            m_rxReg = 0;
            m_state = I2C_START;
        }
        else if( m_sdaState && !m_lastSDA ) {   // We are in a Stop Condition
           m_state = I2C_STOP;// I2Cstop();
        }
    }
    else if( sclState == Clock_Rising )  // We are in a SCL Rissing edge
    {
        if( m_state == I2C_START )       // Get Transaction Info
        {                                // TODO add support for 10 bits
            readBit();
            if( m_bitPtr > m_addressBits )
            {
                bool rw = m_rxReg % 2;                 //Last bit is R/W
                m_rxReg >>= 1;
                if( m_rxReg == m_address ) {            // Address match
                    if( rw ) {                       // Master is Reading
                        m_state = I2C_READ;
                        writeByte();
                    } else {                        // Master is Writting
                        m_state = I2C_WRITE;
                        m_bitPtr = 0;
                    }
                    ACK();
                } else {
                    m_state = I2C_STOP;
                    m_rxReg = 0;
                }
            }
        }else if( m_state == I2C_WRITE ){
            readBit();
            if( m_bitPtr == 8 ) readByte();
        }
        else if( m_state == I2C_READACK )      // We wait for Master ACK
        {
            if( !m_sdaState ) {                  // ACK: Continue Sending
                m_state = m_lastState;
                writeByte();
            } else m_state = I2C_IDLE;
        }
    }
    else if( sclState == Clock_Falling )
    {
        if( m_state == I2C_ACK ) {            // Send ACK
            sheduleSDA( false );
            m_state = I2C_ENDACK;
        }
        else if( m_state == I2C_ENDACK ) {    // We sent ACK, release SDA
            m_state = m_lastState;
            bool releaseSda = true;
            if( m_state == I2C_READ ) releaseSda = m_txReg>>m_bitPtr & 1; // Keep Sending
            sheduleSDA( releaseSda );
            m_rxReg = 0;
        }
        if( m_state == I2C_READ ) writeBit();
    }
    m_lastSDA = m_sdaState;
}

void TwiModule::setTwiState( twiState_t state )
{
    m_twiState = state;
    twiState.emitValue( state );
}

void TwiModule::getSdaState()
{
    double volt = m_sda->getVolt();
    if     ( volt > m_inputHighV ) m_sdaState = true;
    else if( volt < m_inputLowV )  m_sdaState = false;

    m_sda->getPin()->setPinState( m_sdaState? input_high:input_low ); // High : Low colors
}

void TwiModule::sheduleSDA( bool state )
{
    m_sheduleSDA = true;
    m_nextSDA = state;
    Simulator::self()->addEvent( m_clockPeriod/2, this );
}

void TwiModule::readBit()
{
    if( m_bitPtr > 0 ) m_rxReg <<= 1;
    m_rxReg += m_sdaState;            //Read one bit from sda
    m_bitPtr++;
}

void TwiModule::writeBit()
{
    if( m_bitPtr < 0 ) { waitACK(); return; }

    bool bit = m_txReg>>m_bitPtr & 1;
    m_bitPtr--;

    if( bit != m_sdaState ) m_sda->setState( bit ); // Save some events
}

void TwiModule::readByte()
{
    m_bitPtr = 0;
    ACK();
}

void TwiModule::writeByte() { m_bitPtr = 7;}

void TwiModule::waitACK()
{
    m_sda->setState( true );
    m_lastState = m_state;
    m_state = I2C_READACK;
}

void TwiModule::ACK()
{
    m_lastState = m_state;
    m_state = I2C_ACK;
}

void TwiModule::masterStart( uint8_t addr )
{
    m_state = I2C_START;
}

void TwiModule::masterWrite( uint8_t data , bool isAddr )
{
    m_isAddr = isAddr;

    m_state = I2C_WRITE;
    m_txReg = data;
    writeByte();
}

void TwiModule::masterRead( bool ack )
{
    m_masterACK = ack;

    m_sda->setState( true );
    m_bitPtr = 0;
    m_rxReg = 0;
    m_state = I2C_READ;
}
