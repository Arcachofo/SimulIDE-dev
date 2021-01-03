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

#ifndef PLOTDISPLAY_H
#define PLOTDISPLAY_H

#include <QPixmap>
#include <QtWidgets>

class PlotDisplay : public QWidget
{
    friend class DataPlotWidget;
    friend class PlotBase;

    Q_OBJECT

    public:
        PlotDisplay( QWidget* parent = 0 );

        QSize minimumSizeHint() const;
        QSize sizeHint() const;

        void setSize( int width, int height );
        void setData( int ch, QList<QPointF>* data );
        void setXFrame( uint64_t tf );
        void setVTick( int ch, double tick );
        void setVPos( int ch, double vPos );
        void setHPos( int ch, int64_t hPos );
        void setLimits( int ch, double max, double min );
        void setFilter( double f ) {  m_filter = f; }

    protected:
        virtual void paintEvent( QPaintEvent *event );

    private:
        void drawBackground( QPainter *p );
        void updateValues();

        QList<QPointF>* m_data[2];

        double m_filter;

        int m_width;
        int m_height;

        double m_sizeX;
        double m_sizeY;

        double m_hCenter;
        double m_vCenter;
        double m_margin;
        double m_scaleY[2];
        double m_scaleX;

        double m_volt[2];
        double m_vMaxVal[2];
        double m_vMinVal[2];
        double m_vMin[2];
        double m_ampli[2];
        double m_vTick[2];
        double m_vPos[2];
        int64_t m_hPos[2];

        double m_ceroX ;
        double m_endX;
        double m_ceroY;
        double m_endS;
        double m_lineX;
        double m_linWi;

        double m_sclY[2];
        double m_posY[2];
        double m_vMaxPos[2];
        double m_vMinPos[2];

        QFont m_fontB;
        QFont m_fontS;
        QColor m_color[3];
        QColor m_dark[2];
        QColor m_scaleColor[3];
};

#endif

