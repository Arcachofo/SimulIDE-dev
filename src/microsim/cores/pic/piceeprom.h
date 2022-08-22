/***************************************************************************
 *   Copyright (C) 2022 by santiago González                               *
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

#ifndef PICEEPROM_H
#define PICEEPROM_H

#include "mcueeprom.h"

class MAINMODULE_EXPORT PicEeprom : public McuEeprom
{
        friend class McuCreator;

    public:
        PicEeprom( eMcu* mcu, QString name );
        ~PicEeprom();

        virtual void initialize() override;
        virtual void runEvent() override;

        virtual void configureA( uint8_t newEECON1 ) override;
        virtual void configureB( uint8_t newEECON2 ) override;

    private:
        uint64_t m_nextCycle;
        bool m_writeEnable;

        uint8_t m_wrMask;

        regBits_t m_WRERR;
        regBits_t m_WREN;
        regBits_t m_WR;
        regBits_t m_RD;
};

#endif
