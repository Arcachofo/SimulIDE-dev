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

#ifndef ECLOCKEDDEVICE_H
#define ECLOCKEDDEVICE_H

#include "component.h"

enum clkState_t{
    Clock_Low = 0,
    Clock_Rising,
    Clock_Allow,
    Clock_High,
    Clock_Falling,
};

class eElement;
class IoPin;

class MAINMODULE_EXPORT eClockedDevice
{
    public:
        eClockedDevice();
        ~eClockedDevice();

        void initState();
        void stamp( eElement* el );

        Component::trigger_t trigger() { return m_trigger; }
        virtual void setTrigger( Component::trigger_t trigger );

        void setClockPin( IoPin* clockPin ) { m_clockPin = clockPin; }

        bool clockInv();
        void setClockInv( bool inv );

        clkState_t getClockState();

        void remove();

    protected:
        bool m_clock;

        Component::trigger_t m_trigger;

        IoPin* m_clockPin;
};

#endif
