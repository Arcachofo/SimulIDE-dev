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

#ifndef USARTRX_H
#define USARTRX_H

#include <queue>

#include "usartmodule.h"


class MAINMODULE_EXPORT UartRx : public UartTR
{
    public:
        UartRx( UsartModule* usart, eMcu* mcu, QString name );
        ~UartRx();

        enum error{
            frameError =1<<12,
            dataOverrun=1<<13,
            parityError=1<<14
        };

        virtual void enable( uint8_t en ) override;
        virtual void voltChanged() override;
        virtual void runEvent() override;
        virtual void processData( uint8_t ) override;
        virtual uint8_t getData() override;

        void queueData( uint8_t data );

    protected:
        void readBit();
        void byteReceived( uint16_t frame );

        bool m_startHigh;

        uint16_t m_fifo[2];
        uint8_t  m_fifoP;

        std::queue<uint8_t> m_inBuffer;
};

#endif
