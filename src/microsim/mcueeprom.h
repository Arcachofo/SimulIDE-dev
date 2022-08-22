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

#ifndef MCUEEPROM_H
#define MCUEEPROM_H

#include "mcumodule.h"
#include "e-element.h"

class MAINMODULE_EXPORT McuEeprom : public McuModule, public eElement
{
        friend class McuCreator;

    public:
        McuEeprom( eMcu* mcu, QString name );
        virtual ~McuEeprom();

        virtual void initialize() override;

        virtual void readEeprom();
        virtual void writeEeprom();

        virtual void addrWriteL( uint8_t val );
        virtual void addrWriteH( uint8_t val );

    protected:

        uint8_t* m_addressL; // Actual ram for counter Low address byte
        uint8_t* m_addressH; // Actual ram for counter High address byte

        uint32_t m_address;
        uint8_t* m_dataReg;
};

#endif
