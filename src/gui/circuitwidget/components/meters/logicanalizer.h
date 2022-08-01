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
    public:
        LAnalizer( QObject* parent, QString type, QString id );
        ~LAnalizer();

 static Component* construct( QObject* parent, QString type, QString id );
 static LibraryItem* libraryItem();

        virtual void updateStep() override;

        virtual QString timPos() override;
        virtual void setTimPos( QString tp ) override;

        virtual QString volDiv() override;
        virtual void setVolDiv( QString vd ) override;

        virtual void setTimeDiv( uint64_t td ) override;

        int64_t timePos(){ return m_timePos; }
        void setTimePos( int64_t tp );
        virtual void moveTimePos( int64_t delta ) override;

        double voltDiv(){ return m_voltDiv; }
        void setVoltDiv( double vd );

        virtual void setTrigger( int ch ) override;

        double threshold() { return m_threshold; }
        void setThreshold( double thr ) { m_threshold = thr; }

        virtual void setConds( QString conds ) override;

        virtual void setTunnels( QString tunnels ) override;

        virtual void expand( bool e ) override;

    private:
        double m_voltDiv;
        double m_threshold;

        int m_updtCount;

        int64_t m_timePos;

        LaWidget*  m_laWidget;
        DataLaWidget* m_dataWidget;
};

#endif
