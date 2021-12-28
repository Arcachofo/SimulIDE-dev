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

#ifndef RTCLOCK_H
#define RTCLOCK_H

#include <QTime>

#include "e-element.h"

class IoPin;

class MAINMODULE_EXPORT RtClock : public eElement
{
    public:
        RtClock( QString id );
        ~RtClock();

        virtual void initialize() override;
        virtual void runEvent() override;

        void enable( bool en );
        void setDisOut( bool d );
        void setFreq( uint64_t freq );
        void setPin( IoPin* pin ) { m_outpin = pin; }

        QTime m_time;
        QDate m_date;

    private:
        bool m_enabled;
        bool m_disOut;

        uint64_t m_tCount;
        uint64_t m_sCount;
        uint64_t m_toggle;
        uint64_t m_halfPeriod;
        uint64_t m_freqBase;

        IoPin* m_outpin;
};

#endif
