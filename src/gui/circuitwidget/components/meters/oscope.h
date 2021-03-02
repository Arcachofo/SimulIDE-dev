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
    Q_PROPERTY( double Filter   READ filter  WRITE setFilter DESIGNABLE true USER true )
    Q_PROPERTY( int  AutoSC  READ autoSC  WRITE setAutoSC )
    Q_PROPERTY( int  Trigger READ trigger WRITE setTrigger )
    Q_PROPERTY( int  Tracks  READ tracks  WRITE setTracks )

    Q_PROPERTY( QStringList HideCh  READ hideCh  WRITE setHideCh )
    Q_PROPERTY( QStringList TimPos  READ timPos  WRITE setTimPos )
    Q_PROPERTY( QStringList VolDiv  READ volDiv  WRITE setVolDiv )
    Q_PROPERTY( QStringList VolPos  READ volPos  WRITE setVolPos )

    public:

        Oscope( QObject* parent, QString type, QString id );
        ~Oscope();

        static Component* construct( QObject* parent, QString type, QString id );
        static LibraryItem* libraryItem();

        virtual QList<propGroup_t> propGroups() override;

        double filter() { return m_filter; }
        void setFilter( double filter );

        int trigger() { return m_trigger; }
        void setTrigger( int ch );

        int autoSC() { return m_auto; }
        void setAutoSC( int ch );

        QStringList hideCh();
        void setHideCh( QStringList hc );

        int tracks();
        void setTracks( int tracks );

        QStringList timPos();
        void setTimPos( QStringList tp );

        QStringList volDiv();
        void setVolDiv( QStringList vd );

        QStringList volPos();
        void setVolPos( QStringList vp );

        virtual void updateStep() override;

        virtual void sethTick( uint64_t td ) override;
        virtual void setTimeDiv( uint64_t td ) override;

        int64_t timePos( int ch ){ return m_timePos[ch]; }
        void setTimePos( int ch, int64_t tp );

        double voltDiv( int ch ){ return m_voltDiv[ch]; }
        void setVoltDiv( int ch, double vd );

        double voltPos( int ch ){ return m_voltPos[ch]; }
        void setVoltPos( int ch, double vp );

        void hideChannel( int ch, bool hide );

        void toggleExpand();
        virtual void expand( bool e ) override;

        DataWidget* dataW() { return m_dataWidget; }

    private:

        double m_filter;

        int m_trigger;
        int m_auto;

        int64_t m_timePos[4];
        double  m_voltDiv[4];
        double  m_voltPos[4];
        bool    m_hideCh[4];

        OscopeChannel* m_channel[4];
        OscWidget*  m_oscWidget;
        DataWidget* m_dataWidget;
};

#endif

