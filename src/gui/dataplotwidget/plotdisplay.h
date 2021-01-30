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

#include <QtWidgets>

class PlotDisplay : public QWidget
{
    Q_OBJECT

    public:
        PlotDisplay( QWidget* parent = 0 );

        void setExpand( bool expand ) { m_expand = expand; }
        void hideChannel( int ch, bool hide ) { m_hideCh[ch] = hide; }
        int tracks() { return m_tracks; }
        void setTracks( int tracks );
        void setColor( int ch, QColor color );
        void setData( int ch, QList<QPointF>* data );
        void setXFrame( uint64_t tf );
        void setVTick( int ch, double tick );
        void setVPos( int ch, double vPos );
        void setHPos( int ch, int64_t hPos );
        void setLimits( int ch, double max, double min );

    public slots:
        void updateValues();

    protected:
        virtual void paintEvent( QPaintEvent* event );

    private:
        void drawBackground( QPainter *p );

        QList<QPointF>* m_data[4];

        bool m_expand;

        int m_tracks;
        int m_trackScl;

        double m_sizeX;
        double m_sizeY;
        double m_hCenter;
        double m_vCenter;
        double m_marginX;
        double m_marginY;
        double m_scaleY[4];
        double m_scaleX;

        double m_volt[4];
        double m_vMaxVal[4];
        double m_vMinVal[4];
        double m_vMin[4];
        double m_ampli[4];
        double m_vTick[4];
        double m_vPos[4];
        int64_t m_hPos[4];
        bool   m_hideCh[4];

        double m_ceroX ;
        double m_endX;
        double m_ceroY;
        double m_endS;
        double m_lineX;

        double m_sclY[4];
        double m_posY[4];
        double m_vMaxPos[4];
        double m_vMinPos[4];

        QFont m_fontB;
        QFont m_fontS;
        QColor m_color[5];
        QColor m_dark[2];
        QColor m_scaleColor[3];
};

#endif

