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
    public:
        LAnalizer( QObject* parent, QString type, QString id );
        ~LAnalizer();

 static Component* construct( QObject* parent, QString type, QString id );
 static LibraryItem* libraryItem();

        double dataSize() { return m_dataSize/1e6; }
        void setDataSize( double ds ) { m_dataSize = ds*1e6; }

        virtual void updateStep() override;

        virtual void setTimeDiv( double td ) override;

        int timePos(){ return m_timePos; }
        void setTimePos( int tp );

        double voltDiv(){ return m_voltDiv; }
        void setVoltDiv( double vd );

        void setTrigger( int ch );

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
