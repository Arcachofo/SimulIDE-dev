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

#ifndef DATACHANNEL_H
#define DATACHANNEL_H

#include "e-element.h"

#include <QVector>

enum cond_t{
    C_NONE=0,
    C_LOW,
    C_RISING,
    C_HIGH,
    C_FALLING
};

class PlotBase;

class MAINMODULE_EXPORT DataChannel : public eElement
{
        friend class PlotBase;
        friend class Oscope;
        friend class LAnalizer;
        friend class PlotDisplay;

    public:

        DataChannel( PlotBase* plotBase, QString id );
        ~DataChannel();

        virtual void stamp() override;

    protected:
        QVector<double> m_buffer;
        QVector<uint64_t> m_time;

        bool m_trigger;
        int m_trigIndex;

        uint64_t m_risEdge;

        int m_channel;
        int m_bufferCounter;

        cond_t m_cond;
        cond_t m_lastCond;

        QString m_chTunnel;

        PlotBase* m_plotBase;
};

#endif
