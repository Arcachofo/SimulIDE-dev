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

#ifndef AVREEPROM_H
#define AVREEPROM_H

#include "mcueeprom.h"

class MAINMODULE_EXPORT AvrEeprom : public McuEeprom
{
        friend class McuCreator;

    public:
        AvrEeprom( eMcu* mcu, QString name );
        ~AvrEeprom();

        virtual void initialize() override;
        virtual void runEvent() override;

        virtual void configureA( uint8_t newEECR ) override;

        virtual void writeEeprom() override;

    private:
        uint64_t m_nextCycle;

        uint8_t m_mode;

        uint8_t* m_EECR;

        regBits_t m_EEPM;
        regBits_t m_EEMPE;
        regBits_t m_EEPE;
        regBits_t m_EERE;
};

#endif
