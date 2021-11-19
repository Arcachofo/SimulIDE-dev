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

#ifndef AVRWDT_H
#define AVRWDT_H

#include "mcuwdt.h"

class eMcu;

class MAINMODULE_EXPORT AvrWdt : public McuWdt
{
        //friend class McuCreator;

    public:
        AvrWdt( eMcu* mcu, QString name );
        ~AvrWdt();

        virtual void initialize() override;
        virtual void runEvent() override;

        virtual void configureA( uint8_t newWDTCSR ) override;

        virtual void reset() override;

        virtual void callBack() override;

    private:
        void wdtEnable();

        bool m_allowChanges;
        bool m_disabled;

        uint8_t*  m_WDTCSR;

        regBits_t m_WDIF;
        regBits_t m_WDIE;
        regBits_t m_WDCE;
        regBits_t m_WDE;
        regBits_t m_WDP02;
        regBits_t m_WDP3;

        regBits_t m_WDRF;
};
#endif
