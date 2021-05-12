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
#include "oscope.h"


class MAINMODULE_EXPORT OscopeChannel : public DataChannel
{
        friend class Oscope;

    public:

        OscopeChannel( Oscope* oscope, QString id );
        ~OscopeChannel();

        virtual void initialize() override;
        virtual void updateStep() override;
        virtual void voltChanged() override;

        virtual void setFilter( double f );

    private:
        void updateValues();

        uint64_t m_totalP;
        uint64_t m_lastMax;
        uint64_t m_numMax;       // Number of Maximum found
        uint64_t m_nCycles;
        uint64_t m_period;
        uint64_t m_dataZero;

        int m_dataTime;

        double   m_subRate;
        uint64_t m_subSample;
        uint64_t m_subStep;

        double m_lastValue;
        double m_freq;
        double m_mid;

        bool m_rising;
        bool m_falling;
        bool m_connected;

        double m_maxVal;
        double m_minVal;
        double m_dispMax;
        double m_dispMin;
        double m_ampli;
        double m_filter;

        Oscope* m_oscope;
};

#endif

