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

        virtual void configureA( uint8_t newUCSRnC ) override;
        virtual void configureB( uint8_t newUCSRnB ) override;
        virtual uint8_t getBit9() override;
        virtual void setBit9( uint8_t bit ) override;

        virtual void byteSent( uint8_t data ) override;

        void setUBRRnL( uint8_t val );
        void setUBRRnH( uint8_t val );

    private:
        void setBaurrate( uint16_t ubrr );

        uint8_t*  m_ucsrna;
        uint8_t*  m_ucsrnb;
        uint8_t*  m_ubrrnL;
        uint8_t*  m_ubrrnH;

        regBits_t m_bit9Tx;
        regBits_t m_bit9Rx;

        regBits_t m_txEn;
        regBits_t m_rxEn;

        regBits_t m_modeRB;
        regBits_t m_pariRB;
        regBits_t m_stopRB;
        regBits_t m_dataRB;
        regBits_t m_u2xn;
};

#endif

