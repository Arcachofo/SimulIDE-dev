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
#include "e-element.h"

enum clkState_t{
    Clock_Low = 0,
    Clock_Rising=1,
    Clock_Allow=1,
    Clock_High,
    Clock_Falling,
};

enum trigger_t {
    None = 0,
    Clock,
    InEnable
};

class eElement;
class IoPin;

class MAINMODULE_EXPORT eClockedDevice : public eElement
{
    public:
        eClockedDevice( QString id );
        ~eClockedDevice();

        virtual void initialize() override;
        virtual void stamp() override;

        trigger_t trigger() { return m_trigger; }
        virtual void setTrigger( trigger_t trigger );

        void setClockPin( IoPin* clockPin ) { m_clkPin = clockPin; }

        bool clockInv();
        void setClockInv( bool inv );

        void updateClock();

        void remove();

    protected:
        bool m_clock;

        clkState_t m_clkState;
        trigger_t  m_trigger;

        IoPin* m_clkPin;
};

#endif
