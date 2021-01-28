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
#include "plotbase.h"

class MAINMODULE_EXPORT DataChannel : public eElement
{
        friend class PlotBase;
        friend class Oscope;

    public:

        DataChannel( PlotBase* plotBase, QString id );
        ~DataChannel();

        virtual void initialize(){;}
        virtual void stamp() override;
        virtual void voltChanged(){;}

        virtual void setFilter( double f ){;}
        virtual void fetchData( uint64_t orig, uint64_t origAbs , uint64_t offset );

        //paCond chCond() { return m_chCond; }
        //bool chCondFlag() { return m_chCondFlag; }

        //QList<QPointF>* points() { return m_points; }

    protected:
        QList<QPointF> m_pointsA;
        QList<QPointF> m_pointsB;
        QList<QPointF>* m_points;

        QVector<double> m_buffer;
        QVector<uint64_t> m_time;

        int m_channel;
        int m_bufferCounter;

        double m_maxVal;
        double m_minVal;
        double m_dispMax;
        double m_dispMin;
        double m_ampli;
        double m_filter;

        paCond m_chCond;
        bool   m_chCondFlag;

        PlotBase* m_plotBase;
};

#endif
