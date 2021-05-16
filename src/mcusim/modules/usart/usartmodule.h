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

#include "e-element.h"
#include "regsignal.h"

#define mSTOPBITS m_usart->m_stopBits
#define mDATABITS m_usart->m_dataBits
#define mDATAMASK m_usart->m_dataMask
#define mPARITY   m_usart->m_parity

enum parity_t{
    parNONE=0,
    parEVEN,
    parODD,
};

class IoPin;
class UartTR;

class MAINMODULE_EXPORT UsartModule
{
    public:
        UsartModule( QString name );
        ~UsartModule();

        virtual void step( uint8_t ){;}

        virtual uint8_t getBit9(){return 0;}
        virtual void    setBit9( uint8_t bit ){;}

        virtual void dataAvailable( uint8_t data ){;}

        void parityError();

        uint8_t m_mode;
        uint8_t m_stopBits;
        uint8_t m_dataBits;
        uint8_t m_dataMask;
        parity_t m_parity;

    protected:
        void setPeriod( uint64_t period );

        UartTR* m_sender;
        UartTR* m_receiver;

        bool m_running;   // is Uart running?
};

class MAINMODULE_EXPORT UartTR : public eElement
{
    public:
        UartTR( UsartModule* usart, QString name );
        ~UartTR();

        enum state_t{
            usartSTOPPED=0,
            usartIDLE,
            usartTRANSMIT,
            usartTXEND,
            usartRECEIVE,
            usartRXEND,
        };

        virtual void processData( uint8_t data )=0;
        virtual void enable( uint8_t en ){;}

        void setPeriod( uint64_t period ) { m_period = period; }
        bool getParity( uint8_t data );

        state_t state() { return m_state; }

        void setPin( IoPin* pin ) { m_ioPin = pin; }

        RegSignal<uint8_t> on_dataEnd;

    protected:
        UsartModule* m_usart;
        IoPin* m_ioPin;

        uint16_t m_frame;
        uint8_t m_framesize;
        uint8_t m_currentBit;
        uint8_t m_bit9;
        state_t m_state;

        bool m_enabled;

        uint64_t m_period; // Baudrate period
};

#endif
