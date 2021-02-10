/***************************************************************************
 *   Copyright (C) 2017 by Santiago González                               *
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

#include "e-i2c_master.h"
#include "simulator.h"

eI2CMaster::eI2CMaster( QString id )
          : eI2C( id )
{
}
eI2CMaster::~eI2CMaster() { }

void eI2CMaster::stamp()                    // Called at Simulation Start
{
    m_toggleScl = false;

    if( m_input.size() == 0 ) return;
    if( !m_enabled ) return;

    SDA_PIN->setImp( high_imp );
    SCL_PIN->setImp( high_imp );

    //eLogicDevice::stamp();   // Initialize Base Class ( Clock pin is managed in eLogicDevice )
}

void eI2CMaster::setSDA( bool state )
{
    if( state ) SDA_PIN->setImp( high_imp );
    else        SDA_PIN->setImp( m_outImp );
}

void eI2CMaster::setSCL( bool state )
{
    if( state ) SCL_PIN->setImp( high_imp );
    else        SCL_PIN->setImp( m_outImp );
}

void eI2CMaster::keepClocking()
{
    m_toggleScl = true;
    Simulator::self()->addEvent( m_propDelay, this );
}

void eI2CMaster::runEvent()       // We are in Mater mode, controlling Clock
{
    if( !m_enabled ) return;

    bool clkLow = SCL_PIN->getVolt() < m_inputLowV;
    if( m_toggleScl )
    {
        setSCL( clkLow ); // High if is LOW, LOW if is HIGH
        m_toggleScl = false;
        return;
    }
    m_SDA = eLogicDevice::getInputState( 0 );       // State of SDA pin

    Simulator::self()->addEvent( m_stepsPe, this ); //
    if( m_state == I2C_IDLE ) return;

    if( m_state == I2C_STARTED )                    // Send Start Condition
    {
        if     ( m_SDA ) setSDA( false );     // Step 1: SDA is High, Lower it
        else if( !clkLow )                    // Step 2: SDA Already Low, Lower Clock
        {
            if( m_comp ) m_comp->inStateChanged( 128 ); // Set TWINT
            keepClocking();
        }
    }
    else if( m_state == I2C_WRITTING )              // We are Writting data
    {
        if( clkLow ) writeBit();                    // Set SDA while clk is Low
        keepClocking();
    }
    else if( m_state == I2C_READING )               // We are Reading data
    {
        if( !clkLow )                               // Read bit while clk is high
        {
            readBit();
            if( m_bitPtr == 8 )
            {
                readByte();
                if( m_comp ) m_comp->inStateChanged( 128+I2C_READING );
            }
        }
        keepClocking();
    }
    else if( m_state == I2C_WAITACK )                // Read ACK
    {
        int ack = 257;                               //  ACK
        if( m_SDA ) ack = 256;                       // NACK
        if( m_comp ) m_comp->inStateChanged( ack );
        m_state = I2C_ACK;
        keepClocking();
    }
    else if( m_state == I2C_STOPPED )                // Send Stop Condition
    {
        if     (  m_SDA && clkLow )  setSDA( false ); // Step 1: Lower SDA
        else if( !m_SDA && clkLow )  keepClocking();  // Step 2: Raise Clock
        else if( !m_SDA && !clkLow ) setSDA( true );  // Step 3: Raise SDA
        else if(  m_SDA && !clkLow )                  // Step 4: Operation Finished
        {
            m_state = I2C_IDLE;
            if( m_comp ) m_comp->inStateChanged( 128+I2C_STOPPED ); // Set TWINT ( set to 0 )
        }
    }
}

void eI2CMaster::masterStart( uint8_t addr )
{
    m_state = I2C_STARTED;
}

void eI2CMaster::masterWrite( uint8_t data )
{
    m_state = I2C_WRITTING;
    m_txReg = data;
    writeByte();
}

void eI2CMaster::masterRead()
{
    setSDA( true );
    m_bitPtr = 0;
    m_state = I2C_READING;
}

void eI2CMaster::updatePins()
{
    if( m_enabled )
    {
        setSDA( true );
        setSCL( true );
        Simulator::self()->addEvent( m_stepsPe, this );
    }
    else m_state = I2C_IDLE;
}
