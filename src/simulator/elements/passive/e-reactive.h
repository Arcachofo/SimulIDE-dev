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

#ifndef EREACTIVE_H
#define EREACTIVE_H

#include "e-resistor.h"

class MAINMODULE_EXPORT eReactive : public eResistor
{
    public:
        eReactive( QString id );
        ~eReactive();

        virtual void stamp() override;
        virtual void voltChanged() override;
        virtual void runEvent() override;

        double initVolt() { return m_InitVolt; }
        void setInitVolt( double v ) { m_InitVolt = v; }

        double initCurr() { return -m_InitCurr; }
        void setInitCurr( double c ) { m_InitCurr = -c; }

        virtual void stepError(){;}

    protected:
        void updtReactStep();

        virtual double updtRes(){ return 0.0;}
        virtual double updtCurr(){ return 0.0;}

        double m_value; // Capacitance or Inductance

        double m_InitCurr;
        double m_curSource;

        double m_InitVolt;
        double m_volt;

        double m_tStep;

        uint64_t m_reacStep;
        uint64_t m_lastTime;
        uint64_t m_deltaTime;

        int m_autoStep;

        bool m_stepError;
        bool m_running;
};

#endif
