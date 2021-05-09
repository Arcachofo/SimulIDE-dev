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

enum i2cState_t{
    I2C_IDLE=0,
    I2C_STARTED,
    I2C_READING,
    I2C_WRITTING,
    I2C_STOPPED,
    I2C_ACK,
    I2C_ENDACK,
    I2C_WAITACK
};

#include "e-clocked_device.h"
#include "regsignal.h"

class eSource;
class TwiTR;

class MAINMODULE_EXPORT TwiModule
{
    public:
        TwiModule(bool master, bool slave, QString name );
        ~TwiModule();

        eSource* sda() { return m_sda; }
        eSource* scl() { return m_scl; }

    protected:
        uint8_t* m_address;

        eSource* m_sda;
        eSource* m_scl;

        TwiTR* m_master;
        TwiTR* m_slave;
};

class MAINMODULE_EXPORT TwiTR : public eClockedDevice
{
    public:
        TwiTR( TwiModule* twi, QString name );
        ~TwiTR();

        virtual void writeByte();
        virtual void readByte();
        //virtual void I2Cstop();

        RegSignal<uint8_t> on_dataEnd;

    protected:
        virtual void setSDA( bool state );
        virtual void setSCL( bool state );

        bool getSdaState();
        void readBit();
        void writeBit();
        void waitACK();
        void ACK();

        TwiModule* m_twi;

        bool m_enabled;

        bool m_sdaState;
        bool m_lastSdaState;

        int m_bitPtr;       // Bit Pointer

        uint64_t m_clockPeriod;   // TWI Clock period in ps

        uint8_t m_txReg;    // Byte to Send
        uint8_t m_rxReg;    // Byte Received

        i2cState_t m_state;      // Current State of i2c
        i2cState_t m_lastState;  // Last State of i2c
};

#endif
