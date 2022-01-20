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

#include <QBrush>
#include <QPen>

#include "plotdisplay.h"
#include "datachannel.h"
#include "mainwindow.h"
#include "circuitview.h"
#include "simulator.h"
#include "utils.h"

PlotDisplay::PlotDisplay( QWidget* parent )
           : QWidget( parent )
{
    m_channels = 4;
    for( int i=0; i<8; ++i )
    {
        m_scaleY[i] = 1;
        m_hPos[i] = 0;
        m_vTick[i] = 1;
        m_vPos[i] = 0;
        m_vMaxVal[i] = 5;
        m_vMinVal[i] = 0;
        m_vMin[i] = 0;
        m_channel[i] = NULL;
        m_hideCh[i] = false;
    }
    m_sizeX = 135;
    m_sizeY = 135;
    m_marginX = 3*10;
    m_marginY = 10;
    m_tracks = 1;
    m_timeEnd = 0;
    m_expand = false;

    double fontScale = MainWindow::self()->fontScale();
    m_fontB.setPixelSize( 9*fontScale );
    m_fontB.setBold( true );
    m_fontS.setPixelSize( 8*fontScale );

    m_scaleColor[0] = QColor( 70, 70, 70 );
    m_scaleColor[1] = QColor( 100, 100, 100 );
    m_scaleColor[2] = QColor( 200, 200, 200 );

    m_dark[0] = QColor( 200, 200, 60 );
    m_dark[1] = QColor( 180, 180, 215 );

    setMouseTracking(true);
}

void PlotDisplay::setTimeDiv( uint64_t td )
{
    m_timeDiv = td;
    setTimeEnd( m_timeEnd );
}

void PlotDisplay::setTimeEnd( double timeEnd )
{
    double timeFrame = m_timeDiv*10;
    m_timeEnd = timeEnd;
    m_timeStart = timeEnd - timeFrame;
    m_scaleX = (double)m_sizeX/timeFrame;
}

void PlotDisplay::updateValues()
{
    if( m_expand )
    {
        m_marginX = 3*10;
        m_marginY = 10;
    }else{
        m_marginX = 4;
        m_marginY = 4;
    }
    m_sizeX = width()-2*m_marginX;
    m_sizeY = height()-2*m_marginY;

    m_hCenter = (double)width()/2;
    m_vCenter = (double)height()/2;
    m_ceroX = m_marginX;
    m_endX  = width()- m_marginX;
    m_ceroY = m_marginY;
    m_endY  = height()-m_marginY;

    for( int i=0; i<m_channels; ++i )
    {
        m_scaleY[i] = m_sizeY/(m_vTick[i]*10)/m_tracks;
        m_posY[i] = m_ceroY+m_vPos[i]*m_scaleY[i];
        m_posY[i] += (i%m_tracks+0.5)*m_sizeY/m_tracks;
    }
}

void PlotDisplay::drawBackground( QPainter* p )
{
    p->setBrush( QColor( 0, 0, 0 ) );
    p->drawRoundRect( 0, 0, width(), height(), 7, 7 );

    double divs = 10*m_tracks;
    double ceroY = m_ceroY;
    if( m_channels == 8 )
    {
        divs = 8;
        ceroY += m_sizeY/16;
    }

    QPen pen( m_scaleColor[0], 1.5, Qt::SolidLine, Qt::RoundCap, Qt::RoundJoin );
    p->setPen( pen );

    if( m_expand )
        for( double i=ceroY; i<=m_endY+1; i+=m_sizeY/divs )
            p->drawLine( QPointF(m_ceroX, i), QPointF(m_endX, i) ); //Horizontal lines

    for( double i=m_ceroX; i<=m_endX+1; i+=m_sizeX/10. )
        p->drawLine( QPointF(i, m_ceroY), QPointF(i, m_endY) );     //Vertical lines

    if( m_expand )
    {
        QPen pen2( m_scaleColor[1], 1, Qt::SolidLine, Qt::RoundCap, Qt::RoundJoin );
        p->setPen( pen2 );
        double m = 2;
        double ym = 10*5;
        if     ( m_tracks == 2 ) ym = 10*2*2;
        else if( m_tracks == 4 ) ym = 1;

        for( double i=m_ceroY; i<=m_endY; i+=m_sizeY/ym )
            p->drawLine( QPointF(m_hCenter-m, i), QPointF(m_hCenter+m, i) ); // Vertical Marks

        QPen pen3( m_scaleColor[1], 1.5, Qt::SolidLine, Qt::RoundCap, Qt::RoundJoin );
        p->setPen( pen3 );
        for( int i=0; i<m_tracks; i++ )
        {
            double y = m_ceroY+(i+0.5)*m_sizeY/m_tracks;
            p->drawLine( m_ceroX, y, m_endX, y );                //Horizontal Center lines

            for( double i=m_ceroX; i<=m_endX; i+=m_sizeX/50. )
                p->drawLine( QPointF(i, y-m), QPointF(i, y+m) ); // Horizontal Center Marks
        }
        p->drawLine( m_hCenter, m_ceroY, m_hCenter, m_endY );    //Vertical Center line
}   }

void PlotDisplay::paintEvent( QPaintEvent* /* event */ )
{
    QPainter p( this );

    updateValues();
    drawBackground( &p );

    if( m_timeEnd == 0 ) return;

    p.setRenderHint( QPainter::Antialiasing, true );

    QPoint cPos = QCursor::pos()-mapToGlobal( QPoint(0, 0) );
    int cursorX = cPos.x(); // /scale;
    int cursorY = cPos.y(); // /scale;

    bool drawCursor = false;
    if( (cursorX > m_ceroX) && (cursorX < m_endX ) // Draw Cursor?
     && (cursorY > 0) && (cursorY < height()) ) drawCursor = true;

    for( int i=0; i<m_channels; ++i )  // PRINT DATA
    {
        if( m_hideCh[i] || m_ncCh[i] ) continue;

        QPen pen2( m_color[i], 2, Qt::SolidLine, Qt::RoundCap, Qt::RoundJoin );
        p.setPen( pen2 );

        QVector<double>*   voltData = &m_channel[i]->m_buffer;
        QVector<uint64_t>* timeData = &m_channel[i]->m_time;

        m_vMaxVal[i] = -1e12;
        m_vMinVal[i] =  1e12;

        int pos = m_channel[i]->m_trigIndex;
        int bufferSize = m_channel[i]->m_buffer.size();

        double timeStart = m_timeStart-m_hPos[i];
        if( timeStart < 0 ) timeStart = 0;
        double timeEnd   = m_timeEnd-m_hPos[i];
        double time, x1, x2, y1, y2;
        double p1Volt=0;//, p2Volt=0;
        QPointF P1, P2, Pm;

        // Subsample
        double lastX = 1e12;
        double maxX = 0;
        double minX = 0;
        double maxY  = -1e12;
        double minY  = 1e12;
        bool subSample = false;

        for( int j=0; j<bufferSize; ++j ) // Read Backwards
        {
            p1Volt = voltData->at(pos);
            time   = timeData->at(pos);
            x1 = m_ceroX + (time+m_hPos[i]-m_timeStart)*m_scaleX;
            y1 = m_posY[i] - p1Volt*m_scaleY[i];

            if( (j > 0) && (time <= m_timeEnd) )
            {
                if( time <= timeEnd )
                {
                    if( p1Volt > m_vMaxVal[i] ) m_vMaxVal[i] = p1Volt; // Maximum Voltage
                    if( p1Volt < m_vMinVal[i] ) m_vMinVal[i] = p1Volt; // MiniMum Voltage
                    if( drawCursor && cursorX>x1 && cursorX<x2 ) // Claculate Cursor Voltage betwen 2 points
                    {
                        double cVolt= p1Volt;
                        ///if( x2 != x1 && p2Volt!=p1Volt) cVolt = p1Volt+(cursorX-x1)*(p2Volt-p1Volt)/(x2-x1);
                        ///if( cVolt < 0 ) cVolt += 0;
                        m_cursorVolt[i] = cVolt;
                    }

                    if( lastX-x1 < 0.5 ) // SubSample
                    {
                        if( !subSample ) P2 = P1;
                        subSample = true;
                        if     ( y1 > maxY ) { maxY = y1; maxX = x1; }
                        else if( y1 < minY ) { minY = y1; minX = x1; }
                    }else{
                        if( subSample )
                        {
                            if( maxX > minX )
                            {
                                P1 = QPointF( maxX, maxY );
                                p.drawLine( P1, P2 );
                                P2 = P1;
                                if( minX > 0 )
                                {
                                    P1 = QPointF( minX, minY );
                                    p.drawLine( P1, P2 );
                                    P2 = P1;
                                }
                            }
                            else if ( minX > maxX )
                            {
                                P1 = QPointF( minX, minY );
                                p.drawLine( P1, P2 );
                                P2 = P1;
                                if( maxX > 0 )
                                {
                                    P1 = QPointF( maxX, maxY );
                                    p.drawLine( P1, P2 );
                                    P2 = P1;
                                }
                            }
                            subSample = false;
                            maxY = -1e12; minY = 1e12; maxX = 0; minX = 0;
                            //P1 = QPointF( x1, y1 );
                            //p.drawLine( P1, P2 );
                        }
                        //else
                        {
                            P2 = QPointF( x2, y2 );
                            P1 = QPointF( x1, y1 );
                            Pm = QPointF( x2, y1 );
                            p.drawLine( P1, Pm );
                            p.drawLine( Pm, P2 );
                        }
                        lastX = x1;
                }   }
                if( time <= timeStart ) break;
            }
            x2 = x1; y2 = y1;
            //p2Volt = p1Volt;
            if( --pos < 0 ) pos += bufferSize;
    }   }
    // Draw Rects to crop data plots
    p.fillRect( 0,                   0, m_marginX-1, height(), QColor( 10, 15, 50 ) );
    p.fillRect( m_sizeX+m_marginX+1, 0, m_marginX-1, height(), QColor( 10, 15, 50 ) );

    if( !m_expand ) { p.end(); return; }

    if( m_channels < 8 )
    {
        for( int i=0; i<m_channels; ++i ) // SCALES, MAX-MIN
        {
            if( m_hideCh[i] || m_ncCh[i] ) continue;

            QPen pen1( m_color[i], 0.5, Qt::SolidLine, Qt::RoundCap, Qt::RoundJoin );
            p.setPen( pen1 );
            double vMaxPos = m_posY[i] -(m_vMaxVal[i]-m_vMin[i])*m_scaleY[i];
            double vMinPos = m_posY[i] -(m_vMinVal[i]-m_vMin[i])*m_scaleY[i];
            p.drawLine( m_ceroX, vMaxPos, m_endX, vMaxPos );   //Horizontal Max V line
            p.drawLine( m_ceroX, vMinPos, m_endX, vMinPos );   //Horizontal Min V line

            int xPos = 2+(i&1)*(m_sizeX+m_marginX);

            QPen pen3( m_color[i], 1.5, Qt::SolidLine, Qt::RoundCap, Qt::RoundJoin );
            p.setPen( pen3 );
            p.setFont( m_fontS );

            // Draw Background scale values
            /*int s = 0;
            int pY;
            for( double j=m_ceroY; j<=m_endY; j+=m_sizeY/10 )
            {
                if( i < 2 ) pY = j-4;
                else        pY = j+4;
                int decs = 0;
                double volTick = (m_vTick[i]*5-m_vTick[i]*s++);
                if     ( volTick < 10)   decs = 2;
                else if( volTick < 100)  decs = 1;
                QString number = QString::number( volTick,'f', decs);
                if( volTick >= 0 ) number.prepend(" ");

                p.drawText( xPos, pY-5, width(), 10, Qt::AlignVCenter, number );
            }
            // Draw Rects behind Max & Min Numbers
            p.fillRect( xPos, vMaxPos-4, 21, 8, QColor( 10, 15, 50 ) );
            p.fillRect( xPos, vMinPos-4, 21, 8, QColor( 10, 15, 50 ) );*/

            // Draw Max & Min Values
            p.setFont( m_fontB );
            p.drawText( xPos, vMaxPos-5, width(), 10, Qt::AlignVCenter, QString::number(m_vMaxVal[i],'f', 2) );
            p.drawText( xPos, vMinPos-5, width(), 10, Qt::AlignVCenter, QString::number(m_vMinVal[i],'f', 2) );

            if( drawCursor )              // Draw Cursor Voltages
            {
                pen3.setColor( m_color[i] );
                p.setPen( pen3 );
                double val = m_cursorVolt[i]*1e12; // Volt in pV
                QString unit;
                int decs = 0;
                valToUnit( val, unit, decs );

                int y = cursorY-22-10*(3-i);
                p.drawText( cursorX-62, y, 60, 12, Qt::AlignRight, QString::number( val,'f', decs) );
                p.drawText( cursorX-1,  y, 60, 12, Qt::AlignLeft, unit+"V" );
    }   }   }
    if( drawCursor )  // CURSOR and cursor Time
    {
        QPen pen1( m_scaleColor[2], 1, Qt::DotLine, Qt::RoundCap, Qt::RoundJoin );
        p.setPen( pen1 );

        p.drawLine( cursorX, 0, cursorX, height() );       //Horizontal Cursor V line
        p.drawLine( m_ceroX, cursorY, m_endX, cursorY );   //Horizontal Cursor H line

        pen1.setColor( Qt::white );
        p.setPen( pen1 );
        double val = (double)(cursorX-m_hCenter)/m_scaleX; // Time in ps *1e3 bcos sim time is in ns
        QString unit;
        int decs = 0;
        valToUnit( val, unit, decs );

        p.drawText( cursorX-62, cursorY-12, 60, 12, Qt::AlignRight, QString::number( val,'f', decs) );
        p.drawText( cursorX-1,  cursorY-12, 20, 12, Qt::AlignLeft, unit+"s" );
    }
    p.end();
}

#include "moc_plotdisplay.cpp"
