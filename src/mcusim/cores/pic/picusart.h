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

#ifndef PICUSART_H
#define PICUSART_H

#include "mcuuart.h"
#include "mcutypes.h"

class McuTimer;

class MAINMODULE_EXPORT PicUsart : public McuUsart
{
    public:
        PicUsart( eMcu* mcu, QString name, int number );
        ~PicUsart();

        virtual void configureA( uint8_t newTXSTA ) override;
        virtual void configureB( uint8_t newRCSTA ) override;
        virtual uint8_t getBit9Tx() override;
        virtual void setBit9Rx( uint8_t bit ) override;

        virtual void sendByte( uint8_t data ) override;
        virtual void bufferEmpty() override;
        virtual void frameSent( uint8_t data ) override;

        virtual void overrunError() override;
        virtual void parityError() override;
        virtual void frameError() override;

        void setSPBRGL(  uint8_t val );
        void setSPBRGH(  uint8_t val );
        void setBaurrate( uint8_t val=0 );

    private:
        bool m_enabled;

        bool m_speedx2;

        uint8_t*  m_PIR1;
        uint8_t*  m_TXSTA;
        uint8_t*  m_RCSTA;
        uint8_t*  m_SPBRG;
        uint8_t*  m_SPBRGL;
        uint8_t*  m_SPBRGH;

        regBits_t m_bit9Tx;
        regBits_t m_bit9Rx;

        regBits_t m_txEn;
        regBits_t m_rxEn;

        regBits_t m_TRMT;
        regBits_t m_TXIF;
        regBits_t m_RCIF;
        regBits_t m_BRGH;
        regBits_t m_SPEN;
        regBits_t m_TX9;
        regBits_t m_RX9;
        regBits_t m_FERR;
        regBits_t m_OERR;
};

#endif
