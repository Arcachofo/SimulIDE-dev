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

#ifndef DATAPLOTWIDGET_H
#define DATAPLOTWIDGET_H

#include <QtWidgets>
#include <QVector>

#include "plotdisplay.h"
#include "probe.h"

enum {
    None = 0,
    Rising,
    Falling,
    High,
    Low
};

class PlotBase;

class MAINMODULE_EXPORT DataPlotWidget : public QWidget
{
    friend class DataChannel;
    friend class OscopeChannel;
    friend class PlotBase;
    friend class Oscope;

    Q_OBJECT

    public:
        DataPlotWidget(QWidget* parent , PlotBase* PlotBase );
        ~DataPlotWidget();

        void setupWidget();
        void showControlls( bool show );
        void setSize( int width, int height );

        void setHTick( uint64_t hTick );
        void setVTick( int ch, double vTick );
        void setHPos( int ch, double vPos );
        void setVPos( int ch, double vPos );

        void updateWidgets();

        bool eventFilter( QObject* object, QEvent* event );

    public slots:
        void setAuto( int ch );
        void setTrigger( int ch );

        void channel1Changed();
        void channel2Changed();

        void updateHTickBox();
        void updateHPosBox( int ch );
        void updateVTickBox( int ch );
        void updateVPosBox( int ch );


        void hTickDialChanged( int Hscale );
        void vTickDial1Changed( int Vscale );

        void hPosDial1Changed( int hPos );
        void vPosDial1Changed( int vPos );

        void hTickBoxChanged();
        void vTickBox1Changed();

        void hPosBox1Changed();
        void vPosBox1Changed();

        void buttonChanged( int ch );

        void probeRemoved( Probe* p );
        void setProbe( int ch );

    private:
        void vTickDialChanged( int ch, int Vscale );
        void hPosDialChanged( int ch, int vPos );
        void vPosDialChanged( int ch, int vPos );

        void vTickBoxChanged( int ch );
        void hPosBoxChanged( int ch );
        void vPosBoxChanged( int ch );

        QDial*          createDial();
        QDoubleSpinBox* createSpinBox( QString suffix );
        QHBoxLayout*    createQHBoxLayout();

        QHBoxLayout* m_horizontalLayout;
        QVBoxLayout* m_vChannelLayout;
        QVBoxLayout* m_controllsLayout;

        QLabel* m_hTickLabel;
        QLabel* m_hPosLabel;
        QLabel* m_vTickLabel;
        QLabel* m_vPosLabel;
        QLabel* m_autoLabel;
        QLabel* m_trigLabel;

        QLabel* m_data1Label[2];
        QLabel* m_data2Label[2];

        QCheckBox* m_advaCheck;

        QButtonGroup* m_autoGroup;
        QRadioButton* m_autoCheck[2];

        QButtonGroup* m_trigGroup;
        QRadioButton* m_trigCheck[2];

        QDial*          m_hTickDial;
        QDoubleSpinBox* m_hTickBox;
        QDial*          m_hPosDial;
        QDoubleSpinBox* m_hPosBox;
        QDial*          m_vTickDial;
        QDoubleSpinBox* m_vTickBox;
        QDial*          m_vPosDial;
        QDoubleSpinBox* m_vPosBox;

        QButtonGroup* m_buttonGroup;
        QPushButton*  m_button[3];

        QLineEdit* m_channel[2];

        QFrame* m_line[3];
        //QPalette m_palette;

        int m_numCh;
        int m_sizeX;
        int m_sizeY;

        int   m_auto;
        int   m_trigger;

        int    m_prevHscale;
        int    m_prevHPos;
        double m_prevVscale;
        double m_prevVPos;

        uint64_t m_hTick;
        int64_t  m_hPos[2];
        double   m_vTick[2];
        double   m_vPos[2];

        bool m_paOnCond;
        int  m_refCond;
        bool m_refCondFlag;

        QString m_colorN[4];

        PlotDisplay* m_display;
        PlotBase*    m_plotB;
        Probe*       m_probe[2];
};

#endif
