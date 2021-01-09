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
#include "dataplotwidget.h"
#include "topwidget.h"
#include "datachannel.h"

class MAINMODULE_EXPORT PlotBase : public Component, public eElement
{
    friend class DataPlotWidget;

    Q_OBJECT
    Q_PROPERTY( int Basic_X   READ baSizeX  WRITE setBaSizeX  DESIGNABLE true USER true )
    Q_PROPERTY( int Basic_Y   READ baSizeY  WRITE setBaSizeY  DESIGNABLE true USER true )
    Q_PROPERTY( int Expand_X  READ adSizeX  WRITE setAdSizeX  DESIGNABLE true USER true )
    Q_PROPERTY( int Expand_Y  READ adSizeY  WRITE setAdSizeY  DESIGNABLE true USER true )

    Q_PROPERTY( bool    Data_Log READ paOnCond WRITE setPaOnCond DESIGNABLE true USER true )
    Q_PROPERTY( double  Log_us   READ dataSize WRITE setDataSize DESIGNABLE true USER true )
    Q_PROPERTY( paCond  CH1_Cond READ ch1Cond  WRITE setCh1Cond  DESIGNABLE true USER true )
    Q_PROPERTY( paCond  CH2_Cond READ ch2Cond  WRITE setCh2Cond  DESIGNABLE true USER true )
    Q_PROPERTY( paCond  REF_Cond READ refCond  WRITE setRefCond  DESIGNABLE true USER true )

    Q_PROPERTY( bool Expand  READ advanc  WRITE setAdvanc )
    Q_PROPERTY( int  Trigger READ trigger WRITE setTrigger )
    Q_PROPERTY( int  AutoSC  READ autoSC  WRITE setAutoSC )

    Q_PROPERTY( quint64 hTick  READ hTick  WRITE setHTick )
    Q_PROPERTY( double  vTick1 READ vTick1 WRITE setVTick1 )
    Q_PROPERTY( double  vTick2 READ vTick2 WRITE setVTick2 )
    Q_PROPERTY( double  vPos1  READ vPos1  WRITE setVPos1 )
    Q_PROPERTY( double  vPos2  READ vPos2  WRITE setVPos2 )
    Q_PROPERTY( QString Probe1 READ probe1 WRITE setProbe1 )
    Q_PROPERTY( QString Probe2 READ probe2 WRITE setProbe2 )

    Q_ENUMS( paCond )

    public:

        PlotBase( QObject* parent, QString type, QString id );
        ~PlotBase();

        enum paCond {
            None = 0,
            Rising,
            Falling,
            High,
            Low
        };

        int baSizeX() { return m_baSizeX; }
        void setBaSizeX( int size ) { m_baSizeX = (size/10)*10; setAdvanc( m_advanc ); }

        int baSizeY() { return m_baSizeY; }
        void setBaSizeY( int size ) { m_baSizeY = (size/10)*10; setAdvanc( m_advanc ); }

        int adSizeX() { return m_adSizeX; }
        void setAdSizeX( int size ) { m_adSizeX = (size/10)*10; setAdvanc( m_advanc ); }

        int adSizeY() { return m_adSizeY; }
        void setAdSizeY( int size ) { m_adSizeY = (size/10)*10; setAdvanc( m_advanc ); }

        double dataSize() { return DataChannel::m_dataSize/1e6; }
        void setDataSize( double ds ) { DataChannel::m_dataSize = ds*1e6; }

        bool paOnCond() {return m_dataPlotW->m_paOnCond; }
        void setPaOnCond( bool pa ) { m_dataPlotW->m_paOnCond = pa; }

        paCond ch1Cond() { return (paCond)m_channel[0]->m_chCond; }
        void setCh1Cond( paCond cond ) { m_channel[0]->m_chCond = cond; }

        paCond ch2Cond() { return (paCond)m_channel[1]->m_chCond; }
        void setCh2Cond( paCond cond ) { m_channel[1]->m_chCond = cond; }

        paCond refCond() { return (paCond)m_dataPlotW->m_refCond; }
        void setRefCond( paCond cond ) { m_dataPlotW->m_refCond = cond; }

        int trigger() { return m_dataPlotW->m_trigger; }
        void setTrigger( int ch ) { m_dataPlotW->setTrigger( ch ); }

        bool advanc() { return m_advanc; }

        int autoSC() { return m_dataPlotW->m_auto; }
        void setAutoSC( int ch ) { m_dataPlotW->setAuto( ch ); }

        QString probe1() { return m_probe[0]; }
        void setProbe1( QString p );

        QString probe2() { return m_probe[1]; }
        void setProbe2( QString p );

        uint64_t hTick() { return m_dataPlotW->m_hTick; }
        void setHTick( uint64_t hTick ){ m_dataPlotW->setHTick( hTick );}

        double vTick1() { return m_dataPlotW->m_vTick[0]; }
        void setVTick1( double vTick ){ m_dataPlotW->setVTick( 0, vTick );}

        double vTick2() { return m_dataPlotW->m_vTick[1]; }
        void setVTick2( double vTick ){ m_dataPlotW->setVTick( 1, vTick );}

        double vPos1() { return m_dataPlotW->m_vPos[0]; }
        void setVPos1( double vPos ){ m_dataPlotW->setVPos( 0, vPos );}

        double vPos2() { return m_dataPlotW->m_vPos[1]; }
        void setVPos2( double vPos ){ m_dataPlotW->setVPos( 1, vPos );}

        virtual void initialize() override;
        virtual void remove() override;

        void pauseOnCond();
        void updateTrig( int ch );

        void connectProbe( int ch, bool con ) { m_probeConnected[ch] = con; }

        virtual void paint( QPainter* p, const QStyleOptionGraphicsItem* option, QWidget* widget );

    public slots:
        void setAdvanc( bool advanc );

    protected:
        int m_bufferSize;

        int m_trigger;

        bool m_pinConnected[2];
        bool m_probeConnected[2];

        int m_sizeX;
        int m_sizeY;
        int m_extraSize;

        int m_baSizeX;
        int m_baSizeY;

        int m_adSizeX;
        int m_adSizeY;

        bool m_advanc;

        QString m_probe[2];

        DataChannel* m_channel[2];
        PlotDisplay* m_display;

        TopWidget*            m_topW;
        DataPlotWidget*       m_dataPlotW;
        QGraphicsProxyWidget* m_proxy;
};

#endif

