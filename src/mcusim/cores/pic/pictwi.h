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

#ifndef AVRTWI_H
#define AVRTWI_H

#include "mcutwi.h"
//#include "mcutypes.h"

class eMcu;
//class McuPin;

class MAINMODULE_EXPORT PicTwi : public McuTwi
{
    public:
        PicTwi( eMcu* mcu, QString name );
        ~PicTwi();

        virtual void initialize() override;

        virtual void configureA( uint8_t newTWCR ) override;
        virtual void configureB( uint8_t val ) override;

        virtual void writeAddrReg( uint8_t newTWAR ) override;
        virtual void writeStatus( uint8_t newTWSR ) override;
        virtual void writeTwiReg( uint8_t newTWDR ) override;

    protected:
        virtual void setTwiState( twiState_t state ) override;
        uint8_t getStaus() { return *m_statReg &= 0b11111000; }
        virtual void updateFreq() override;

        uint8_t m_bitRate;

        uint8_t*  m_TWCR;
        //uint8_t*  m_TWSR;

        regBits_t m_TWEN;
        regBits_t m_TWWC;
        regBits_t m_TWSTO;
        regBits_t m_TWSTA;
        regBits_t m_TWEA;
        regBits_t m_TWINT;

};

#endif
