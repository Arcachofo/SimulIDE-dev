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

#ifndef PLOTBASE_H
#define PLOTBASE_H

//#include <QScriptEngine>
//#include <QScriptProgram>

#include "component.h"
#include "scriptmodule.h"
#include "datachannel.h"

class PlotDisplay;
class QGraphicsProxyWidget;

class MAINMODULE_EXPORT PlotBase : public Component, public ScriptModule
{
        Q_OBJECT
    public:
        PlotBase( QObject* parent, QString type, QString id );
        ~PlotBase();

        virtual bool setPropStr( QString prop, QString val ) override;

        int baSizeX() { return m_baSizeX; }
        void setBaSizeX( int size );

        int baSizeY() { return m_baSizeY; }
        void setBaSizeY( int size );

        int bufferSize() { return m_bufferSize; }
        void setBufferSize( int bs );

        QString timDiv();
        void setTimDiv( QString td );

        virtual QString timPos()=0;
        virtual void setTimPos( QString tp )=0;

        virtual QString volDiv()=0;
        virtual void setVolDiv( QString vd )=0;

        uint64_t timeDiv() { return m_timeDiv; }
        virtual void setTimeDiv( uint64_t td );

        virtual void moveTimePos( int64_t delta )=0;

        int trigger() { return m_trigger; }
        virtual void setTrigger( int ch )=0;

        virtual QString tunnels();
        virtual void setTunnels( QString tunnels )=0;

        virtual void expand( bool e ){;}
        void toggleExpand() { expand( !m_expand ); }

        QString conds() { return m_conditions; }
        virtual void setConds( QString conds ){;}
        void updateConds( QString conds );

        virtual void channelChanged( int ch, QString name ) { m_channel[ch]->m_chTunnel = name; }

        PlotDisplay* display() { return m_display; }

        QColor getColor( int c ) { return m_color[c]; }

        void conditonMet( int ch, cond_t cond );

        virtual void paint( QPainter* p, const QStyleOptionGraphicsItem* option, QWidget* widget );

    protected:
        int m_bufferSize;
        int m_trigger;

        bool m_expand;

        int m_screenSizeX;
        int m_screenSizeY;
        int m_baSizeX;
        int m_baSizeY;

        double m_dataSize;

        uint64_t m_timeDiv;
        uint64_t m_risEdge;

        int m_numChannels;
        DataChannel* m_channel[8];

        int m_condCh1;
        int m_condCh2;
        int m_condCh3;
        int m_condCh4;
        int m_condCh5;
        int m_condCh6;
        int m_condCh7;
        int m_condCh8;
        bool m_pause;
        asIScriptFunction* m_pauseFunc;
        QString m_conditions;
        //QScriptProgram m_condProgram;
        //QScriptEngine m_engine;

        QHash<QString, QString> m_condTo;

        QColor m_color[5];

        PlotDisplay* m_display;

        QGraphicsProxyWidget* m_proxy;
};

#endif
