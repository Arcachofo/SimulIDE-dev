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

#ifndef MCUUART_H
#define MCUUART_H

#include "e-element.h"
#include "regsignal.h"

class eMcu;
class McuPin;
class UsartM;

class MAINMODULE_EXPORT UartTR : public eElement
{
        friend class McuCreator;
        friend class UsartM;

    public:
        UartTR( UsartM* usart, QString name );
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

        RegSignal<uint8_t> on_dataEnd;

    protected:
        UsartM* m_usart;
        McuPin* m_ioPin;

        uint8_t* m_register;

        uint16_t m_frame;
        uint8_t m_framesize;
        uint8_t m_currentBit;
        uint8_t m_bit9;
        state_t m_state;

        bool m_enabled;

        uint64_t m_period; // Baudrate period
};

class MAINMODULE_EXPORT UartTx : public UartTR
{
    public:
        UartTx( UsartM* usart, QString name );
        ~UartTx();

        virtual void enable( uint8_t en ) override;
        virtual void runEvent() override;
        virtual void processData( uint8_t data ) override;

    protected:
        void sendBit();
};

class MAINMODULE_EXPORT UartRx : public UartTR
{
    public:
        UartRx( UsartM* usart, QString name );
        ~UartRx();

        virtual void enable( uint8_t en ) override;
        virtual void runEvent() override;
        virtual void processData( uint8_t data ) override;

    protected:
        void readBit();
};

#endif
