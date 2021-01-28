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

#include "component.h"
#include "e-element.h"
#include "topwidget.h"


enum paCond {
    None = 0,
    Rising,
    Falling,
    High,
    Low
};

class PlotDisplay;
class DataChannel;

class MAINMODULE_EXPORT PlotBase : public Component, public eElement
{
    Q_OBJECT
    Q_PROPERTY( int Basic_X   READ baSizeX  WRITE setBaSizeX  DESIGNABLE true USER true )
    Q_PROPERTY( int Basic_Y   READ baSizeY  WRITE setBaSizeY  DESIGNABLE true USER true )

    Q_PROPERTY( bool    Data_Log READ paOnCond WRITE setPaOnCond DESIGNABLE true USER true )
    Q_PROPERTY( double  Log_us   READ dataSize WRITE setDataSize DESIGNABLE true USER true )
    Q_PROPERTY( paCond  CH1_Cond READ ch1Cond  WRITE setCh1Cond  DESIGNABLE true USER true )
    Q_PROPERTY( paCond  CH2_Cond READ ch2Cond  WRITE setCh2Cond  DESIGNABLE true USER true )
    Q_PROPERTY( paCond  REF_Cond READ refCond  WRITE setRefCond  DESIGNABLE true USER true )

    Q_PROPERTY( quint64 hTick  READ timeDiv  WRITE setTimeDiv )
    Q_PROPERTY( qint64  hPos1  READ hPos1  WRITE setHPos1 )
    Q_PROPERTY( qint64  hPos2  READ hPos2  WRITE setHPos2 )
    Q_PROPERTY( double  vTick1 READ vTick1 WRITE setVTick1 )
    Q_PROPERTY( double  vTick2 READ vTick2 WRITE setVTick2 )
    Q_PROPERTY( double  vPos1  READ vPos1  WRITE setVPos1 )
    Q_PROPERTY( double  vPos2  READ vPos2  WRITE setVPos2 )
    Q_PROPERTY( QString Probe1 READ probe1 WRITE setProbe1 )
    Q_PROPERTY( QString Probe2 READ probe2 WRITE setProbe2 )
    Q_ENUM( paCond )

    public:
        PlotBase( QObject* parent, QString type, QString id );
        ~PlotBase();

        int baSizeX() { return m_baSizeX; }
        void setBaSizeX( int size );

        int baSizeY() { return m_baSizeY; }
        void setBaSizeY( int size );

        double dataSize() { return m_dataSize/1e6; }
        void setDataSize( double ds ) { m_dataSize = ds*1e6; }

        bool paOnCond() { return m_paOnCond; }
        void setPaOnCond( bool pa ) { m_paOnCond = pa; }

        paCond ch1Cond();
        void setCh1Cond( paCond cond );

        paCond ch2Cond();
        void setCh2Cond( paCond cond );

        paCond refCond() { return m_refCond; }
        void setRefCond( paCond cond ) { m_refCond = cond; }

        QString probe1() { return m_probe[0]; }
        void setProbe1( QString p );

        QString probe2() { return m_probe[1]; }
        void setProbe2( QString p );

        int hPos1() { return m_timePos[0]; }
        void setHPos1( int hp ){ setTimePos( 0, hp );}

        int hPos2() { return m_timePos[1]; }
        void setHPos2( int hp ){ setTimePos( 1, hp );}

        double vTick1() { return m_voltDiv[0]; }
        void setVTick1( double vd ){ setVoltDiv(0, vd);}

        double vTick2() { return m_voltDiv[1]; }
        void setVTick2( double vd ){ setVoltDiv( 1, vd );}

        double vPos1() { return m_voltPos[0]; }
        void setVPos1( double vp ){ setVoltPos( 0, vp );}

        double vPos2() { return m_voltPos[1]; }
        void setVPos2( double vp ){ setVoltPos( 1, vp );}


        uint64_t timeDiv() { return m_timeDiv; }
        virtual void setTimeDiv( uint64_t td ){ m_timeDiv = td;}
        int64_t timePos( int ch ){ return m_timePos[ch]; }
        virtual void setTimePos( int ch, int64_t tp );

        double voltDiv( int ch ){ return m_voltDiv[ch]; }
        virtual void setVoltDiv( int ch, double vd );

        double voltPos( int ch ){ return m_voltPos[ch]; }
        virtual void setVoltPos( int ch, double vp );

        virtual void initialize() override;
        virtual void remove() override;

        virtual void expand( bool e ){;}

        void pauseOnCond();

        PlotDisplay* display() { return m_display; }

        void connectProbe( int ch, bool con ) { m_probeConnected[ch] = con; }

        QColor getColor( int c ) { return m_color[c]; }

        virtual void paint( QPainter* p, const QStyleOptionGraphicsItem* option, QWidget* widget );

    protected:
        int m_bufferSize;

        bool m_paOnCond;
        paCond m_refCond;

        bool m_pinConnected[4];
        bool m_probeConnected[4];

        bool m_expand;

        int m_screenSizeX;
        int m_screenSizeY;
        int m_extraSize;

        int m_baSizeX;
        int m_baSizeY;

        double m_dataSize;

        uint64_t m_timeDiv;
        int64_t  m_timePos[4];

        double m_voltDiv[4];
        double m_voltPos[4];

        QString m_probe[4];

        QColor m_color[5];

        DataChannel* m_channel[4];
        PlotDisplay* m_display;

        QGraphicsProxyWidget* m_proxy;
};

#endif

