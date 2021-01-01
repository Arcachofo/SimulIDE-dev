/***************************************************************************
 *   Copyright (C) 2020 by santiago González                               *
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

#ifndef OSCOPECHANNEL_H
#define OSCOPECHANNEL_H

#include "datachannel.h"


class MAINMODULE_EXPORT OscopeChannel : public DataChannel
{
    public:

        OscopeChannel( QString id );
        ~OscopeChannel();

        virtual void initialize() override;
        virtual void updateStep() override;
        virtual void runEvent() override;
        virtual void voltChanged() override;

        virtual void fetchData(int ch , uint64_t edge );

    private:
        uint64_t m_totalP;
        uint64_t m_lastMax;
        uint64_t m_numMax;       // Number of Maximum found
        uint64_t m_nCycles;
        uint64_t m_period;
        uint64_t m_risEdge;
        uint64_t m_stepsPerS;

        double m_Vpos;
        double m_vTick;
        double m_lastValue;
        double m_freq;

        bool m_rising;
        bool m_falling;
        bool m_reading;
        //bool m_gotWave;
        bool m_runEvent;
};

#endif

