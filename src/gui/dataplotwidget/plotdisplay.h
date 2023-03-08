/***************************************************************************
*   Copyright (C) 2020 by Santiago González                               *
*                                                                         *
***************************************************************************/

#ifndef PLOTDISPLAY_H
#define PLOTDISPLAY_H

#include <QPixmap>
#include <QWidget>

class DataChannel;
class PlotBase;

class PlotDisplay : public QWidget
{
    friend class DataPlotWidget;
    friend class PlotBase;

    Q_OBJECT

    public:
        PlotDisplay( QWidget* parent = 0 );

        void setPlotBase( PlotBase* pb ) { m_component = pb; }
        void setExpand( bool expand ) { m_expand = expand; }
        void hideChannel( int ch, bool hide ) { m_hideCh[ch] = hide; }
        void connectChannel( int ch, bool con ) { m_ncCh[ch] = !con; }
        int tracks() { return m_tracks; }
        void setTracks( int tracks ) { m_tracks = tracks; update();}
        void setChannel( int ch, DataChannel* channel ) { m_channel[ch] = channel; }
        void setChannels( int ch ) { m_channels = ch; }
        void setColor( int ch, QColor color ) { m_color[ch] = color; }
        void setTimeDiv( uint64_t td );
        void setTimeEnd( double timeEnd );
        void setVTick( int ch, double tick ) { m_vTick[ch] = tick; }
        void setVPos( int ch, double vPos ) { m_vPos[ch] = -vPos; }
        void setHPos( int ch, int64_t hPos ) { m_hPos[ch] = -hPos; }
        void setLimits( int ch, double max, double min ) { m_vMin[ch]  = min; }

        void setTimeZero( double t );

        uint64_t startTime() { return m_timeStart; }
        uint64_t endTime()   { return m_timeEnd; }

        double sizeX() { return m_sizeX; }

        void updateValues();

    protected:
        virtual void paintEvent( QPaintEvent* event ) override;
        virtual void wheelEvent( QWheelEvent* event ) override;

    private:
        inline void drawBackground( QPainter* p );

        PlotBase*    m_component;
        DataChannel* m_channel[8];

        QVector<double>* m_buffer[8];
        QVector<uint64_t>* m_time[8];
        int m_bufferCounter[8];

        double m_timeStart;
        double m_timeEnd;
        double m_timeDiv;
        double m_timeZero;

        bool m_expand;

        int m_channels;
        int m_tracks;

        double m_sizeX;
        double m_sizeY;

        double m_ceroX ;
        double m_endX;
        double m_ceroY;
        double m_endY;
        double m_hCenter;
        double m_vCenter;
        double m_marginX;
        double m_marginY;
        double m_scaleY[8];
        double m_scaleX;

        double m_cursorVolt[8];
        double m_vMaxVal[8];
        double m_vMinVal[8];
        double m_vMin[8];
        double m_vTick[8];
        double m_vPos[8];
        double m_hPos[8];
        double m_posY[8];

        bool   m_hideCh[8];
        bool   m_ncCh[8];

        QFont m_fontB;
        QFont m_fontXS;
        QFont m_fontS;
        QFont m_fontL;

        QColor m_color[8];
        QColor m_dark[2];
        QColor m_scaleColor[3];
};

#endif

