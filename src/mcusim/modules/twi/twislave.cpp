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

#include "twislave.h"
#include "simulator.h"
#include "e-source.h"

TwiSlave::TwiSlave( TwiModule* twi, QString name )
        : TwiTR( twi, name )
{
}
TwiSlave::~TwiSlave( ){}

void TwiSlave::initialize()
{
    m_addressBits = 7;
}

void TwiSlave::stamp()      // Called at Simulation Start
{
    //if( m_input.size() == 0 ) return;
    if( !m_enabled ) return;

    /*SDA_PIN->setState( false );
    SDA_PIN->setImp( high_imp );
    SCL_PIN->setImp( high_imp );
*/
    eClockedDevice::stamp();   // Initialize Base Class ( Clock pin is managed in eClockedDevice )

    eNode* enode = m_twi->sda()->getPin()->getEnode(); // Register for SDA voltage changes
    if( enode ) enode->voltChangedCallback( this );
}

void TwiSlave::setSDA( bool state )
{
    m_nextSDA = state;
    Simulator::self()->addEvent( m_clockPeriod/2, this );
}

void TwiSlave::runEvent()       // We are in Mater mode, controlling Clock
{
    if( !m_enabled ) return;

    m_twi->sda()->setState( m_nextSDA );
}

void TwiSlave::voltChanged()   // Some Pin Changed State, Manage it
{
    if( !m_enabled ) return;

    int sclState = eClockedDevice::getClockState(); // Get Clk to don't miss any clock changes

    m_sdaState = getSdaState();        // State of SDA pin

    if(( sclState == Clock_High )&&( m_state != I2C_ACK ))
    {
        if( m_lastSdaState && !m_sdaState ) {     // We are in a Start Condition
            m_bitPtr = 0;
            m_rxReg = 0;
            m_state = I2C_STARTED;
        }
        else if( m_sdaState && !m_lastSdaState ) {   // We are in a Stop Condition
           m_state = I2C_STOPPED;;// I2Cstop();
        }
    }
    else if( sclState == Clock_Rising )  // We are in a SCL Rissing edge
    {
        if( m_state == I2C_STARTED )             // Get Transaction Info
        {                                // TODO add support for 10 bits
            readBit();
            if( m_bitPtr > m_addressBits )
            {
                bool rw = m_rxReg % 2;                 //Last bit is R/W
                m_rxReg >>= 1;
                if( m_rxReg == m_address ) {            // Address match
                    if( rw ) {                       // Master is Reading
                        m_state = I2C_READING;
                        writeByte();
                    } else {                        // Master is Writting
                        m_state = I2C_WRITTING;
                        m_bitPtr = 0;
                        //startWrite();
                    }
                    ACK();
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
            if( !m_sdaState ) {                      // ACK: Continue Sending
                m_state = m_lastState;
                writeByte();
            } else m_state = I2C_IDLE;
        }
    }
    else if( sclState == Clock_Falling )
    {
        if( m_state == I2C_ACK ) {                           // Send ACK
            setSDA( false );
            m_state = I2C_ENDACK;
        }
        else if( m_state == I2C_ENDACK ) {   // We sent ACK, release SDA
            m_state = m_lastState;
            bool releaseSda = true;
            if( m_state == I2C_READING ) releaseSda = m_txReg>>m_bitPtr & 1; // Keep Sending
            setSDA( releaseSda );
            m_rxReg = 0;
        }
        if( m_state == I2C_READING ) writeBit();
    }
    m_lastSdaState = m_sdaState;
}
