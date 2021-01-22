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

#ifndef EI2C_H
#define EI2C_H

#include "e-logic_device.h"
#include "component.h"

#define SDA_PIN m_input[0]
#define SCL_PIN m_clockSource

enum i2cState_t{
    I2C_IDLE = 0,
    I2C_STARTED ,
    I2C_READING ,
    I2C_WRITTING,
    I2C_STOPPED ,
    I2C_ACK     ,
    I2C_ENDACK  ,
    I2C_WAITACK ,
};

class MAINMODULE_EXPORT eI2C : public eLogicDevice
{
    public:

        eI2C( QString id );
        ~eI2C();

        virtual void stamp() override;
        virtual void initialize() override;
        virtual void runEvent() override;
        virtual void voltChanged() override;

        virtual void startWrite(){;}
        virtual void writeByte();
        virtual void readByte();
        virtual void slaveStop();
        virtual void setEnabled( bool en );
        virtual void setMaster( bool m );
        virtual void setAddress( int address );
        virtual void setFreq( double f );
        virtual void setComponent( Component* comp ) { m_comp = comp; }

        virtual void masterStart( uint8_t addr );
        virtual void masterWrite( uint8_t data );
        virtual void masterRead();
        virtual void masterStop();

        virtual int byteReceived() { return m_rxReg; }

    protected:
        void setSDA( bool state );
        void setSCL( bool state );
        void readBit();
        void writeBit();
        void ACK();
        void waitACK();

        void updatePins();
        
        int m_address;           // Device Address
        int m_txReg;             // Byte to Send
        int m_rxReg;             // Byte Received
        int m_addressBits;       // Number of m_address bits
        int m_bitPtr;            // Bit Pointer

        i2cState_t m_state;      // Current State of i2c
        i2cState_t m_lastState;  // Last State of i2c

        double m_freq;
        double m_stepsPe;

        bool m_SDA;
        bool m_lastSDA;
        bool m_master;
        bool m_enabled;
        bool m_toggleScl;
        bool m_lowerSda;
        bool m_releaseSda;

        Component* m_comp;
};


#endif
