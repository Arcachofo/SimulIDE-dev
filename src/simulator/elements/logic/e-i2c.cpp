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

#include "e-i2c.h"
#include "simulator.h"
//#include <QDebug>

eI2C::eI2C( QString id )
    : eLogicDevice( id )
{
    m_comp = 0l;
    m_address = 0x00;
    m_enabled = true;
    m_master  = false;

    m_freq = 100000;
}
eI2C::~eI2C() { }

void eI2C::initialize()
{
    eLogicDevice::initialize();

    double stepsPerS = 1e12;
    m_stepsPe = stepsPerS/m_freq/2;
    m_propDelay = m_stepsPe/3;
    
    m_txReg  = 0;
    m_rxReg  = 0;
    m_bitPtr = 0;

    m_lastSDA    = false;
    m_toggleScl  = false;
    m_lowerSda   = false;
    m_releaseSda = false;

    m_state = I2C_IDLE;
    m_addressBits = 7;
}

void eI2C::stamp()                    // Called at Simulation Start
{
    if( m_input.size() == 0 ) return;

    SDA_PIN->setOut( false );
    SDA_PIN->setImp( high_imp );
    SCL_PIN->setOut( false );
    SCL_PIN->setImp( high_imp );

    if( !m_enabled ) return;

    eLogicDevice::stamp();   // Initialize Base Class ( Clock pin is managed in eLogicDevice )

    eNode* enode = SDA_PIN->getEpin(0)->getEnode(); // Register for SDA voltage changes
    if( enode ) enode->voltChangedCallback( this );
}

void eI2C::runEvent()       // We are in Mater mode, controlling Clock
{
    if( !m_enabled ) return;

    bool clkLow = SCL_PIN->getVolt() < m_inputLowV;
    if( m_toggleScl )
    {
        SCL_PIN->setOut( false );
        if( clkLow ) SCL_PIN->setTimedImp( high_imp );
        else         SCL_PIN->setTimedImp( m_outImp );

        m_toggleScl = false;
        return;
    }
    m_SDA = eLogicDevice::getInputState( 0 );       // State of SDA pin

    if( m_lowerSda )                 // Used by Slave to send ACK
    {
        SDA_PIN->setTimedImp( m_outImp );
        Simulator::self()->addEvent( 0, NULL );
        m_lowerSda = false;
        return;
    }
    else if( m_releaseSda )          // Used by Slave to end ACK
    {
        SDA_PIN->setTimedImp( high_imp );
        Simulator::self()->addEvent( 0, NULL );
        m_releaseSda = false;
        return;
    }

    Simulator::self()->addEvent( m_stepsPe, this ); // 1 us
    if( m_state == I2C_IDLE ) return;

    if( m_state == I2C_STARTED )                    // Send Start Condition
    {
        if( m_SDA )                            // Step 1: SDA is High, Lower it
        {
            SDA_PIN->setOut( false );
            SDA_PIN->setTimedImp( m_outImp );
        }
        else if( !clkLow )                    // Step 2: SDA Already Low, Lower Clock
        {
            m_toggleScl = true;
            Simulator::self()->addEvent( m_propDelay, this );

            if( m_comp ) m_comp->inStateChanged( 128 ); // Set TWINT
        }
    }
    else if( m_state == I2C_WRITTING )               // We are Writting data
    {
        m_toggleScl = true;                        // Keep Clocking
        Simulator::self()->addEvent( m_propDelay, this );

        if( clkLow ) writeBit();                      // Set SDA while clk is Low
    }
    else if( m_state == I2C_READING )                // We are Reading data
    {
        m_toggleScl = true;                        // Keep Clocking
        Simulator::self()->addEvent( m_propDelay, this );

        if( !clkLow )                                // Read bit while clk is high
        {
            readBit();
            if( m_bitPtr == 8 )
            {
                readByte();
                if( m_comp ) m_comp->inStateChanged( 128+I2C_READING );
            }
        }
    }
    else if( m_state == I2C_WAITACK )                // Read ACK
    {
        int ack = 257;                               //  ACK
        if( m_SDA ) ack = 256;                       // NACK
        if( m_comp ) m_comp->inStateChanged( ack );

        m_state = I2C_ACK;
        m_toggleScl = true;                        // Lower Clock afther ack
        Simulator::self()->addEvent( m_propDelay, this );
    }
    else if( m_state == I2C_STOPPED )                // Send Stop Condition
    {
        if     ( m_SDA  && clkLow )         // Step 1: Lower SDA
        {
            SDA_PIN->setOut( false );
            SDA_PIN->setTimedImp( m_outImp );
        }
        else if( !m_SDA && clkLow )        // Step 2: Raise Clock
        {
            m_toggleScl = true;
            Simulator::self()->addEvent( m_propDelay, this );
        }
        else if( !m_SDA && !clkLow)        // Step 3: Raise SDA
        {
            SDA_PIN->setOut( false );
            SDA_PIN->setTimedImp( high_imp );
        }
        else if( m_SDA && !clkLow )        // Step 4: Operation Finished
        {
            m_state = I2C_IDLE;
            if( m_comp ) m_comp->inStateChanged( 128+I2C_STOPPED ); // Set TWINT ( set to 0 )
        }
    }
}

void eI2C::voltChanged()            // Some Pin Changed State, Manage it
{
    if( !m_enabled ) return;
    if( m_master ) return;

    int sclState = eLogicDevice::getClockState(); // Get Clk to don't miss any clock changes

    m_SDA = eLogicDevice::getInputState( 0 );        // State of SDA pin

    if(( sclState == Clock_High )&&( m_state != I2C_ACK ))
    {
        if( !m_SDA  && m_lastSDA ) {     // We are in a Start Condition
            m_bitPtr = 0;
            m_rxReg = 0;
            m_state = I2C_STARTED;
        }
        else if( m_SDA && !m_lastSDA ) {   // We are in a Stop Condition
           slaveStop();
        }
    }
    else if( sclState == Clock_Rising )  // We are in a SCL Rissing edge
    {
        if( m_state == I2C_STARTED )             // Get Transaction Info
        {                                // TODO add support for 10 bits
            readBit();
            if( m_bitPtr > m_addressBits ) {
                bool rw = m_rxReg % 2;                 //Last bit is R/W
                m_rxReg >>= 1;
       //qDebug() << "eI2C::stepSlave address" << m_rxReg << m_address;
                if( m_rxReg == m_address ) {            // Address match
                    if( rw ) {                       // Master is Reading
                        m_state = I2C_READING;
                        writeByte();
                    } else {                        // Master is Writting
                        m_state = I2C_WRITTING;
                        m_bitPtr = 0;
                        startWrite();
                    }
                    ACK();
                    //qDebug() << "eI2C::stepSlave Reading" << rw;
                } else {
                    m_state = I2C_STOPPED;
                    m_rxReg = 0;
                }
            }
        }else if( m_state == I2C_WRITTING ){
            readBit();
            if( m_bitPtr == 8 ) readByte();
        }
        else if( m_state == I2C_WAITACK )      // We wait for Master ACK
        {
            if( !m_SDA ) {                      // ACK: Continue Sending
                m_state = m_lastState;
                writeByte();
            } else m_state = I2C_IDLE;
        }
    }
    else if( sclState == Clock_Falling )
    {
        if( m_state == I2C_ACK ) {                           // Send ACK
            m_lowerSda = true;
            Simulator::self()->addEvent( m_propDelay, this );

            m_state = I2C_ENDACK;
        }
        else if( m_state == I2C_ENDACK ) {   // We sent ACK, release SDA
            m_releaseSda = true;
            Simulator::self()->addEvent( m_propDelay, this );

            m_state = m_lastState;
            m_rxReg = 0;
        }
        if( m_state == I2C_READING ) writeBit();
    }
    m_lastSDA = m_SDA;
}

void eI2C::masterStart( uint8_t addr )
{
    //qDebug() << "eI2C::masterStart"<<addr;

    SDA_PIN->setOut( false );
    SDA_PIN->setImp( high_imp );
    SCL_PIN->setOut( false );
    SCL_PIN->setImp( high_imp );

    m_state = I2C_STARTED;
}

void eI2C::masterWrite( uint8_t data )
{
    //qDebug() << "eI2C::masterWrite"<<data;
    m_state = I2C_WRITTING;
    m_txReg = data;
    writeByte();
}

void eI2C::masterRead()
{
    //qDebug() << "eI2C::masterRead";

    SDA_PIN->setOut( false );
    SDA_PIN->setTimedImp( high_imp );

    m_bitPtr = 0;
    m_state = I2C_READING;
}

void eI2C::masterStop()
{
    //qDebug() << "eI2C::masterStop";
    m_state = I2C_STOPPED;
}

void eI2C::slaveStop()
{
    m_state = I2C_STOPPED;
    //qDebug() << "eI2C::stepSlave I2C_STOPPED\n";
}

void eI2C::readBit()
{
    if( m_bitPtr > 0 ) m_rxReg <<= 1;
    m_rxReg += m_SDA;                            //Read one bit from sda
    m_bitPtr++;
}

void eI2C::writeBit()
{
    if( m_bitPtr < 0 ) { waitACK(); return; }

    bool bit = m_txReg>>m_bitPtr & 1;

    SDA_PIN->setOut( false );
    if( bit ) SDA_PIN->setTimedImp( high_imp );
    else      SDA_PIN->setTimedImp( m_outImp );

    if( !m_master ) Simulator::self()->addEvent( 0, NULL );

    m_bitPtr--;
}

void eI2C::readByte()
{
    //qDebug() << "eI2C::readByte()"<<m_rxReg;
    m_bitPtr = 0;
    ACK();
}

void eI2C::writeByte()
{
    m_bitPtr = 7;
}

void eI2C::ACK()
{
    m_lastState = m_state;
    m_state = I2C_ACK;
}

void eI2C::waitACK()
{
    SDA_PIN->setOut( false );
    SDA_PIN->setTimedImp( high_imp ); // Raise SDA
    if( !m_master ) Simulator::self()->addEvent( 0, NULL );
    
    m_lastState = m_state;
    m_state = I2C_WAITACK;
}

void eI2C::setEnabled( bool en )
{
    if( m_enabled == en ) return;
    m_enabled = en;
    updatePins();
}

void eI2C::setMaster( bool m )
{
    if( m_master == m ) return;
    m_master = m;
    Simulator::self()->cancelEvents( this );
    updatePins();
}

void eI2C::updatePins()
{
    if( m_enabled )
    {
        SDA_PIN->setOut( false );
        SDA_PIN->setImp( high_imp );
        SCL_PIN->setOut( false );
        SCL_PIN->setImp( high_imp );

        if( m_master )
        {
            eNode* enode = SCL_PIN->getEpin(0)->getEnode();
            if( enode ) enode->remFromChangedCallback(this);

            Simulator::self()->addEvent( m_stepsPe, this );
        }
        else
        {
            eNode* enode = SCL_PIN->getEpin(0)->getEnode();
            if( enode ) enode->voltChangedCallback( this );
        }
    }
    else m_state = I2C_IDLE;
}

void eI2C::setAddress( int address )
{
    m_address = address;
}

void eI2C::setFreq( double f )
{
    m_freq = f;
    double stepsPerS = 1e12;
    m_stepsPe = stepsPerS/m_freq/2;
    m_propDelay = m_stepsPe/3;
    //qDebug() << "eI2C::setFreq" << f<< m_stepsPe;
}

void eI2C::createPins()  // Usually Called by Subcircuit to create ePins
{
    createClockPin();            // Clock pin is managed in eLogicDevice
    eLogicDevice::createPins( 1, 0 );        // Create input Pin for SDA
}
