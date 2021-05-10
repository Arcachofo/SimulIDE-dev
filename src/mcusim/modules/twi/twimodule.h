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
#include "regsignal.h"
#include "avrtwicodes.h" // Using AVR states comes at hand

class eSource;

class MAINMODULE_EXPORT TwiModule : public eClockedDevice
{
    public:
        TwiModule( QString name );
        ~TwiModule();

        enum trState_t{
            I2C_IDLE=0,
            I2C_START,
            I2C_READ,
            I2C_WRITE,
            I2C_STOP,
            I2C_ACK,
            I2C_ENDACK,
            I2C_READACK
        };

        virtual void initialize() override;
        virtual void stamp() override;
        virtual void runEvent() override;
        virtual void voltChanged() override;

        void sheduleSDA( bool state );
        void getSdaState();

        void setTwiState( twiState_t state );

        virtual void writeByte();
        virtual void readByte();

        void masterStart( uint8_t addr );
        void masterWrite( uint8_t data, bool isAddr );
        void masterRead( bool ack );

        RegSignal<uint8_t> twiState; // Signal to propagate TWI state

    protected:
        void keepClocking();
        void readBit();
        void writeBit();
        void waitACK();
        void ACK();

        uint8_t m_address;           // Device Address
        int m_addressBits;

        uint64_t m_clockPeriod;   // TWI Clock period in ps

        bool m_enabled;
        bool m_sheduleSDA;
        bool m_lastSDA;
        bool m_nextSDA;
        bool m_sdaState;
        bool m_toggleScl;
        bool m_isAddr;
        bool m_masterACK;

        int m_bitPtr;       // Bit Pointer

        uint8_t m_txReg;    // Byte to Send
        uint8_t m_rxReg;    // Byte Received

        trState_t m_state;      // Current State of i2c
        trState_t m_lastState;  // Last State of i2c

        twiState_t m_twiState;

        eSource* m_sda;
        eSource* m_scl;
};

#endif
