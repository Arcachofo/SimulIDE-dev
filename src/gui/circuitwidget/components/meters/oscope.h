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

#ifndef OSCOPE_H
#define OSCOPE_H

#include "plotbase.h"

class LibraryItem;
class OscopeChannel;
class OscWidget;
class DataWidget;

class MAINMODULE_EXPORT Oscope : public PlotBase
{
        Q_OBJECT
    public:
        Oscope( QObject* parent, QString type, QString id );
        ~Oscope();

        static Component* construct( QObject* parent, QString type, QString id );
        static LibraryItem* libraryItem();

        double filter() { return m_filter; }
        void setFilter( double filter );

        int autoSC() { return m_auto; }
        void setAutoSC( int ch );

        QString hideCh();
        void setHideCh( QString hc );

        int tracks();
        void setTracks( int tracks );

        virtual QString timPos() override;
        virtual void setTimPos( QString tp ) override;

        virtual QString volDiv() override;
        virtual void setVolDiv( QString vd ) override;

        QString volPos();
        void setVolPos( QString vp );

        virtual void setTrigger( int ch ) override;

        virtual void updateStep() override;

        virtual void setTimeDiv( uint64_t td ) override;

        int64_t timePos( int ch ){ return m_timePos[ch]; }
        void setTimePos( int ch, int64_t tp );

        double voltDiv( int ch ){ return m_voltDiv[ch]; }
        void setVoltDiv( int ch, double vd );

        double voltPos( int ch ){ return m_voltPos[ch]; }
        void setVoltPos( int ch, double vp );

        void hideChannel( int ch, bool hide );

        virtual void setTunnels( QString tunnels ) override;

        virtual void expand( bool e ) override;

        DataWidget* dataW() { return m_dataWidget; }

    private:
        double m_filter;

        int m_auto;

        int64_t m_timePos[4];
        double  m_voltDiv[4];
        double  m_voltPos[4];
        bool    m_hideCh[4];

        OscWidget*  m_oscWidget;
        DataWidget* m_dataWidget;
};

#endif
