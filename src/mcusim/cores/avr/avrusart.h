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

#ifndef AVRUSART_H
#define AVRUSART_H

#include "mcuuart.h"
#include "mcutypes.h"

class McuTimer;

class MAINMODULE_EXPORT AvrUsart : public McuUsart
{
    public:
        AvrUsart( eMcu* mcu, QString name, int number );
        ~AvrUsart();

        virtual void configureA( uint8_t newUCSRnA ) override;
        virtual void configureB( uint8_t newUCSRnB ) override;
        virtual void configureC( uint8_t newUCSRnC ) override;
        virtual uint8_t getBit9Tx() override;
        virtual void setBit9Rx( uint8_t bit ) override;

        virtual void sendByte( uint8_t data ) override;
        virtual void frameSent( uint8_t data ) override;

        virtual void overrunError() override;
        virtual void parityError() override;
        virtual void frameError() override;

        void setBaurrate( uint8_t ubrr=0 );

    private:
        void setUBRRnL( uint8_t v );
        void setUBRRnH( uint8_t v );

        uint8_t*  m_UCSRnA;
        uint8_t*  m_UCSRnB;
        uint8_t*  m_UBRRnL;
        uint8_t*  m_UBRRnH;
        uint8_t  m_UBRRHval;

        regBits_t m_bit9Tx;
        regBits_t m_bit9Rx;

        regBits_t m_txEn;
        regBits_t m_rxEn;

        regBits_t m_modeRB;
        regBits_t m_pariRB;
        regBits_t m_stopRB;
        regBits_t m_dataRB;
        regBits_t m_u2xn;

        regBits_t m_UDRIE;
        regBits_t m_UDRE;
        regBits_t m_TXC;
        regBits_t m_RXC;
        regBits_t m_FE;
        regBits_t m_DOR;
        regBits_t m_UPE;
};

#endif
