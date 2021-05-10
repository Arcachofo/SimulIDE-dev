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

#include "e-element.h"

enum trigtType_t{
    Trig_None = 0,
    Trig_Clk,
    Trig_InEn,
};

enum clkState_t{
    Clock_Low = 0,
    Clock_Rising,
    Clock_Allow,
    Clock_High,
    Clock_Falling,
};

class eSource;

class MAINMODULE_EXPORT eClockedDevice : public eElement
{
    public:
        eClockedDevice( QString id );
        ~eClockedDevice();

        int eTrigger() { return m_etrigger; }
        virtual void seteTrigger( int trigger );

        void setClockPin( eSource* clockSource ) { m_clockSource = clockSource; }
        void createClockPin();

        bool clockInv();
        void setClockInv( bool inv );

        virtual void stamp() override;

        void callBack( bool en );

        clkState_t getClockState();

    protected:
        void createClockPin( ePin* epin );
        void createClockeSource( ePin* epin );

        bool m_clock;

        int m_etrigger;

        double m_inputImp;
        double m_inputHighV;
        double m_inputLowV;

        eSource* m_clockSource;
};

#endif
