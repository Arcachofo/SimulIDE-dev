/***************************************************************************
 *   Copyright (C) 2020 by santiago González                               *
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

#ifndef USARTMODULE_H
#define USARTMODULE_H

#include<QHash>

#include "mcumodule.h"
#include "e-element.h"

#define mSTOPBITS m_usart->m_stopBits
#define mDATABITS m_usart->m_dataBits
#define mDATAMASK m_usart->m_dataMask
#define mPARITY   m_usart->m_parity

enum parity_t{
    parNONE=0,
    parEVEN=2,
    parODD=3,
};

class IoPin;
class UartTx;
class UartRx;
class SerialMonitor;

class MAINMODULE_EXPORT UsartModule
{
        friend class eMcu;
    public:
        UsartModule( eMcu* mcu, QString name );
        ~UsartModule();

        int baudRate() { return m_baudRate; }
        void setBaudRate( int br );

        virtual uint8_t getBit9Tx(){return 0;}
        virtual void setBit9Rx( uint8_t bit ){;}

        virtual void sendByte( uint8_t data );
        virtual void bufferEmpty(){;}
        virtual void frameSent( uint8_t data );
        virtual void readByte( uint8_t data ){;}
        virtual void byteReceived( uint8_t data );

        virtual void overrunError(){;}
        virtual void parityError(){;}
        virtual void frameError(){;}

        void openMonitor( QString id, int num=0 );
        void uartIn( uint8_t value );

        uint8_t m_mode;
        uint8_t m_stopBits;
        uint8_t m_dataBits;
        uint8_t m_dataMask;
        parity_t m_parity;

    protected:
        void setPeriod( uint64_t period );

        SerialMonitor* m_monitor;

        UartTx* m_sender;
        UartRx* m_receiver;

        bool m_sync;

        int m_baudRate;

        //bool m_running;   // is Uart running?
};

class Interrupt;

class MAINMODULE_EXPORT UartTR : public McuModule, public eElement
{
        friend class McuCreator;

    public:
        UartTR( UsartModule* usart, eMcu* mcu, QString name );
        ~UartTR();

        enum state_t{
            usartSTOPPED=0,
            usartIDLE,
            usartTRANSMIT,
            usartTXEND,
            usartRECEIVE,
            usartRXEND,
        };

        virtual void initialize() override;

        virtual void processData( uint8_t data )=0;
        virtual void enable( uint8_t en ){;}
        virtual uint8_t getData() { return  m_data; }

        virtual void configureA( uint8_t val ) override;

        bool isEnabled() { return m_enabled; }

        void setPeriod( uint64_t period ) { m_period = period; }
        bool getParity( uint16_t data );

        state_t state() { return m_state; }

        void setPins( QList<IoPin*> pinList );
        IoPin* getPin() { return m_ioPin; }

        void raiseInt( uint8_t data=0 );

    protected:
        UsartModule* m_usart;
        IoPin* m_ioPin;
        QList<IoPin*> m_pinList;

        uint8_t m_buffer;
        uint8_t m_data;
        uint16_t m_frame;
        uint8_t m_framesize;
        uint8_t m_currentBit;
        uint8_t m_bit9;
        state_t m_state;

        bool m_enabled;
        bool m_runHardware; // If m_ioPin is not connected don't run hardware

        uint64_t m_period; // Baudrate period
};

#endif
