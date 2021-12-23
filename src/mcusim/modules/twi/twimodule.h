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

#ifndef TWIMODULE_H
#define TWIMODULE_H

#include "e-clocked_device.h"
#include "avrtwicodes.h" // Using AVR states comes at hand

class eSource;

class MAINMODULE_EXPORT TwiModule : public eClockedDevice
{
    public:
        TwiModule( QString name );
        ~TwiModule();

        enum twiMode_t{
            TWI_OFF=0,
            TWI_MASTER,
            TWI_SLAVE
        };

        enum i2cState_t{
            I2C_IDLE=0,
            I2C_STOP,
            I2C_START,
            I2C_READ,
            I2C_WRITE,
            I2C_ACK,
            I2C_ENDACK,
            I2C_READACK
        };

        virtual void initialize() override;
        virtual void stamp() override;
        virtual void runEvent() override;
        virtual void voltChanged() override;

        virtual double freqKHz() { return m_freq/1e3; }
        virtual void setFreqKHz( double f );

        virtual void startWrite(){;} // Notify posible child class
        virtual void writeByte() { m_bitPtr = 7;}
        virtual void readByte();

        uint8_t byteReceived() { return m_rxReg; }

        void setSdaPin( IoPin* pin );
        void setSclPin( IoPin* pin );
        void setMode( twiMode_t mode );

        void masterStart() { m_i2cState = I2C_START; }
        void masterWrite( uint8_t data, bool isAddr, bool write );
        void masterRead( bool ack );
        void masterStop();

        virtual void I2Cstop() { m_i2cState = I2C_STOP; }

    protected:
        inline void setSCL( bool st );
        inline void setSDA( bool st );
        inline void getSdaState();
        inline void sheduleSDA( bool state );
        inline void sheduleSCL( bool state );
        inline void keepClocking();
        inline void readBit();
        inline void writeBit();
        inline void waitACK();
        inline void ACK();

        virtual void setTwiState( twiState_t state ) { m_twiState = state; }

        uint m_address;           // Device Address
        int  m_addrBits;

        double m_freq;
        uint64_t m_clockPeriod;   // TWI Clock half period in ps

        bool m_sheduleSDA;
        bool m_sheduleSCL;
        bool m_nextSCL;
        bool m_lastSDA;
        bool m_nextSDA;
        bool m_sdaState;
        bool m_toggleScl;
        bool m_isAddr;
        bool m_write;
        bool m_sendACK;
        bool m_addrMatch;
        bool m_genCall;

        int m_bitPtr;       // Bit Pointer

        uint8_t m_txReg;    // Byte to Send
        uint8_t m_rxReg;    // Byte Received

        twiMode_t  m_mode;
        twiState_t m_twiState;
        twiState_t m_nextState;
        i2cState_t m_i2cState;   // Current State of i2c
        i2cState_t m_lastState;  // Last State of i2c

        IoPin* m_sda;
        IoPin* m_scl;
};

#endif
