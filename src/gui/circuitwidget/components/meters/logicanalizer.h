/***************************************************************************
 *   Copyright (C) 2018 by santiago González                               *
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

#ifndef LANALIZER_H
#define LANALIZER_H

#include "plotbase.h"


class LibraryItem;
class LaChannel;
class LaWidget;
class DataLaWidget;

class MAINMODULE_EXPORT LAnalizer : public PlotBase
{
    Q_OBJECT

    //Q_PROPERTY( bool    Data_Log READ paOnCond WRITE setPaOnCond DESIGNABLE true USER true )
    //Q_PROPERTY( double  Log_us   READ dataSize WRITE setDataSize DESIGNABLE true USER true )
    Q_PROPERTY( double  vTick    READ voltDiv  WRITE setVoltDiv )
    Q_PROPERTY( int     Trigger  READ trigger  WRITE setTrigger )
    Q_PROPERTY( qint64  TimePos  READ timePos  WRITE setTimePos )
    Q_PROPERTY( double  Treshold READ threshold  WRITE setThreshold )

    public:

        LAnalizer( QObject* parent, QString type, QString id );
        ~LAnalizer();

 static Component* construct( QObject* parent, QString type, QString id );
 static LibraryItem* libraryItem();

        virtual QList<propGroup_t> propGroups() override;

        double dataSize() { return m_dataSize/1e6; }
        void setDataSize( double ds ) { m_dataSize = ds*1e6; }

        virtual void updateStep() override;

        virtual void setTimeDiv( uint64_t td ) override;

        int64_t timePos(){ return m_timePos; }
        void setTimePos( int64_t tp );

        double voltDiv(){ return m_voltDiv; }
        void setVoltDiv( double vd );

        int trigger() { return m_trigger; }
        void setTrigger( int ch );

        double threshold() { return m_threshold; }
        void setThreshold( double thr ) { m_threshold = thr; }

        virtual QStringList tunnels() override;
        virtual void setTunnels( QStringList tunnels ) override;

        virtual void setConds( QVector<int> conds ) override;
        virtual void setCond( int ch, int cond ) override;

        virtual void expand( bool e ) override;

        virtual void channelChanged( int ch, QString name ) override;

    private:
        double m_voltDiv;
        double m_threshold;

        int m_updtCount;

        int64_t m_timePos;

        LaChannel* m_channel[8];
        LaWidget*  m_laWidget;
        DataLaWidget* m_dataWidget;
};

#endif

