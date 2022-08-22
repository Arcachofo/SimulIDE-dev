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

#ifndef AVRSLEEP_H
#define AVRSLEEP_H

#include "mcusleep.h"
#include "mcutypes.h"

enum sleepMode_t{
    sleepIdle=0,
    sleepAdcNR,
    sleepPowDo,
    sleepPowSa,
    sleepRes0,
    sleepRes1,
    sleepStand,
    sleepExtSt,
    sleepNone
};

class MAINMODULE_EXPORT AvrSleep : public McuSleep
{
    public:
        AvrSleep( eMcu* mcu, QString name );
        ~AvrSleep();

        virtual void initialize() override;
        virtual void configureA( uint8_t newVal ) override;

        //virtual void sleep() override;

    protected:
        //sleepMode_t m_sleepMode;

        regBits_t m_SM;
        regBits_t m_SE;

        std::vector<Interrupt*> m_wakeUps;
};

/*class MAINMODULE_EXPORT AvrSleep00 : public AvrSleep
{
    public:
        AvrSleep00( eMcu* mcu, QString name );
        ~AvrSleep00();
}*/
#endif
