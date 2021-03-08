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
        //m_scaleY[i] = 1;
        m_hPos[i] = 0;
        m_vTick[i] = 1;
        m_vPos[i] = 0;
        m_vMaxVal[i] = 5;
        m_vMinVal[i] = 0;
        m_vMin[i] = 0;
        m_data[i] = NULL;
        m_hideCh[i] = false;
    }
    m_sizeX = 135;
    m_sizeY = 135;
    m_marginX = 3*10;
    m_marginY = 10;
    m_tracks = 1;
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

void PlotDisplay::setColor( int ch, QColor color )
{
    m_color[ch] = color;
}

void PlotDisplay::setChannels( int ch )
{
    if( ch > 8 ) ch = 8;
    m_channels = ch;
}

void PlotDisplay::setTracks( int tracks )
{
    m_tracks = tracks;
    updateValues();
}

void PlotDisplay::setLimits( int ch, double max, double min )
{
    m_vMin[ch]  = min;

    double ampli = max-min;
    m_ampli[ch] = ampli;
    //m_scaleY[ch] = ampli/(m_vTick[ch]*10);
    updateValues();
}

void PlotDisplay::setData( int ch, QList<QPointF>* data )
{
    m_data[ch] = data;
}

void PlotDisplay::setXFrame( uint64_t tf )
{
    m_scaleX = (double)m_sizeX/(double)tf;
}

void PlotDisplay::setVTick( int ch, double tick )
{
    if( m_tracks == 8 ) tick = tick/5;
    m_vTick[ch] = tick;
    //m_scaleY[ch] = m_ampli[ch]/(tick*10);
    updateValues();
}

void PlotDisplay::setVPos( int ch, double vPos )
{
    m_vPos[ch] = -vPos;
    updateValues();
}

void PlotDisplay::setHPos( int ch, int64_t hPos )
{
    m_hPos[ch] = -hPos;
    update();
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
    m_endX  = width()-m_ceroX;
    m_ceroY = m_marginY;
    m_endS  = height()-m_marginY;
    m_lineX = m_ceroX;
    m_trackScl = (m_tracks+1)/2;

    for( int i=0; i<m_channels; ++i )
    {
        m_sclY[i] = m_sizeY/(m_vTick[i]*10)/m_trackScl;
        m_posY[i] = m_vMin[i]+m_vPos[i];
        if     ( m_tracks == 2 ) m_posY[i] += (i&1)*m_ampli[i]/2;
        else if( m_tracks == 4 ) m_posY[i] += (i-1)*m_ampli[i]/2;
        else if( m_tracks == 8 ) m_posY[i] += (i-3)*m_ampli[i]/10-0.5;
    }
    update();
}

void PlotDisplay::drawBackground( QPainter* p )
{
    p->setBrush( QColor( 0, 0, 0 ) );
    p->drawRoundRect(0, 0, width(), height(), 7, 7 );

    double divs = 10;
    double ceroY = m_ceroY;
    if( m_channels == 8 )
    {
        divs = 8;
        ceroY += m_sizeY/16;
    }

    QPen pen( m_scaleColor[0], 1.5, Qt::SolidLine, Qt::RoundCap, Qt::RoundJoin );
    p->setPen( pen );
    for( double i=ceroY; i<=m_endS; i+=m_sizeY/divs )
        p->drawLine( QPointF(m_ceroX, i), QPointF(m_endX, i) ); //Horizontal lines

    for( double i=m_lineX; i<=m_endX; i+=m_sizeX/10. )
        p->drawLine( QPointF(i, m_ceroY), QPointF(i, m_endS) ); //Vertical lines

    if( m_channels < 8 )
    {
        if( m_expand )
        {
            QPen pen2( m_scaleColor[1], 1, Qt::SolidLine, Qt::RoundCap, Qt::RoundJoin );
            p->setPen( pen2 );
            double m = 2;
            for( double i=m_ceroY; i<=m_endS; i+=m_sizeY/5*divs )
                p->drawLine( QPointF(m_hCenter-m, i), QPointF(m_hCenter+m, i) ); //Horizontal Marks

            for( double i=m_lineX; i<=m_endX; i+=m_sizeX/50. )
                p->drawLine( QPointF(i, m_vCenter-m), QPointF(i, m_vCenter+m) ); //Vertical Marks
        }
        QPen pen3( m_scaleColor[1], 1.5, Qt::SolidLine, Qt::RoundCap, Qt::RoundJoin );
        p->setPen( pen3 );
        p->drawLine( m_ceroX, m_vCenter, m_endX, m_vCenter );  //Horizontal Center line
        p->drawLine( m_hCenter, m_ceroY, m_hCenter, m_endS );  //Vertical Center line
    }
}

void PlotDisplay::paintEvent( QPaintEvent* /* event */ )
{
    QPainter p( this );

    drawBackground( &p );
    p.setRenderHint( QPainter::Antialiasing, true );

    QPoint cPos = QCursor::pos()-mapToGlobal( QPoint(0, 0) );
    //qreal scale = CircuitView::self()->getScale();
    int cursorX = cPos.x(); // /scale;
    int cursorY = cPos.y(); // /scale;

    bool drawCursor = false;
    if( (cursorX > m_ceroX) && (cursorX < m_endX ) // Draw Cursor?
     && (cursorY > 0) && (cursorY < height()) ) drawCursor = true;

    QPointF P1;
    double x1;
    double y1;
    double x2;
    double y2;
    double p1Volt = 0;
    double p2Volt = 0;

    for( int i=0; i<m_channels; ++i )  // PRINT DATA
    {
        if( !m_data[i] ) continue;
        if( m_hideCh[i] ) continue;
        m_volt[i] = -1e12;

        QPen pen2( m_color[i], 2, Qt::SolidLine, Qt::RoundCap, Qt::RoundJoin );
        p.setPen( pen2 );

        int k = 0;
        //double filter = m_filter*m_sclY[i];//m_ampli[i]*m_sclY[i]/4; //

        m_vMaxVal[i] = -1e12;
        m_vMinVal[i] =  1e12;

        for( QPointF P2 : *m_data[i] )
        {
            p2Volt = P2.y();
            x2 = P2.x()*m_scaleX+m_marginX+m_hPos[i]*m_scaleX;
            y2 = m_vCenter-(p2Volt-m_posY[i])*m_sclY[i];
            P2.setX( x2 );
            P2.setY( y2 );

            if( k == 0 ) // First Point
            {
                if( x2 > 0 ) P1 = QPointF( 0, y2 ); // We need another point before
                else         P1 = P2;
                k++;
                continue;
            }
            x1 = P1.x();
            y1 = P1.y();

            if(( x1<0 && x2<0 ) ||( x1>width() && x2>width() )) // Out of display
            {
                P1 = P2; continue;
            }
            //if( fabs( y2-y1 ) <= filter )    // Filter
            {
                p.drawLine( P1, P2 );
                if( drawCursor && cursorX>x1 && cursorX<x2 ) // Cursor Voltage
                {
                    double cVolt= p1Volt;
                    if( x2 != x1 && p2Volt!=p1Volt)
                        cVolt = p1Volt+(cursorX-x1)*(p2Volt-p1Volt)/(x2-x1);
                    if( cVolt < 0 ) cVolt += 0;
                    m_volt[i] = cVolt;
                }
            }/*else
            {
                QPointF PM = QPointF( x2, y1 );
                p.drawLine( P1, PM );
                p.drawLine( PM, P2 );

                if( drawCursor && cursorX>x1 && cursorX<x2)
                    m_volt[i] = p1Volt ;// Cursor Voltage
            }*/
            if( p2Volt > m_vMaxVal[i] ) m_vMaxVal[i] = p2Volt;
            if( p2Volt < m_vMinVal[i] ) m_vMinVal[i] = p2Volt;
            p1Volt = p2Volt;
            P1 = P2;
        }
    }
    // Draw Rects to crop data plots
    p.fillRect( 0,                   0, m_marginX-1, height(), QColor( 10, 15, 50 ) );
    p.fillRect( m_sizeX+m_marginX+1, 0, m_marginX-1, height(), QColor( 10, 15, 50 ) );
    if( !m_expand || (!m_data[0] && !m_data[1] &&!m_data[2] && !m_data[3]) )
    { p.end(); return; }

    if( m_channels < 8 )
    {
        for( int i=0; i<m_channels; ++i ) // SCALES, MAX-MIN
        {
            if( !m_data[i] ) continue;
            if( m_hideCh[i] ) continue;

            QPen pen1( m_color[i], 0.5, Qt::SolidLine, Qt::RoundCap, Qt::RoundJoin );
            p.setPen( pen1 );
            m_vMaxPos[i] = m_vCenter-(m_vMaxVal[i]-m_posY[i])*m_sclY[i];
            m_vMinPos[i] = m_vCenter-(m_vMinVal[i]-m_posY[i])*m_sclY[i];
            p.drawLine( m_ceroX, m_vMaxPos[i], m_endX, m_vMaxPos[i] );   //Horizontal Max V line
            p.drawLine( m_ceroX, m_vMinPos[i], m_endX, m_vMinPos[i] );   //Horizontal Min V line

            int xPos = 2+(i&1)*(m_sizeX+m_marginX);

            QPen pen3( m_color[i], 1.5, Qt::SolidLine, Qt::RoundCap, Qt::RoundJoin );
            p.setPen( pen3 );
            p.setFont( m_fontS );

            // Draw Background scale values
            int s = 0;
            int pY;
            for( double j=m_ceroY; j<=m_endS; j+=m_sizeY/10 )
            {
                if( i < 2 ) pY = j-4;
                else        pY = j+4;
                int decs = 0;
                double volTick = (m_vTick[i]*5-m_vTick[i]*s++)*m_trackScl;
                if     ( volTick < 10)   decs = 2;
                else if( volTick < 100)  decs = 1;
                QString number = QString::number( volTick,'f', decs);
                if( volTick >= 0 ) number.prepend(" ");

                p.drawText( xPos, pY-5, width(), 10, Qt::AlignVCenter, number );
            }
            // Draw Rects behind Max & Min Numbers
            p.fillRect( xPos, m_vMaxPos[i]-4, 21, 8, QColor( 10, 15, 50 ) );
            p.fillRect( xPos, m_vMinPos[i]-4, 21, 8, QColor( 10, 15, 50 ) );

            // Draw Max & Min Values
            p.setFont( m_fontB );
            p.drawText( xPos, m_vMaxPos[i]-5, width(), 10, Qt::AlignVCenter, QString::number(m_vMaxVal[i],'f', 2) );
            p.drawText( xPos, m_vMinPos[i]-5, width(), 10, Qt::AlignVCenter, QString::number(m_vMinVal[i],'f', 2) );

            //p.setFont( m_fontS );
            if( drawCursor )              // Draw Cursor Voltages
            {
                pen3.setColor( m_color[i] );
                p.setPen( pen3 );
                double val = m_volt[i]*1e12; // Volt in pV m_dark
                QString unit;
                int decs = 0;
                valToUnit( val, unit, decs );

                int y = cursorY-22-10*(3-i);
                p.drawText( cursorX-62, y, 60, 12, Qt::AlignRight, QString::number( val,'f', decs) );
                p.drawText( cursorX-1,  y, 60, 12, Qt::AlignLeft, unit+"V" );
            }
        }
    }
    if( drawCursor )  // CURSOR
    {
        QPen pen1( m_scaleColor[2], 1, Qt::DotLine, Qt::RoundCap, Qt::RoundJoin );
        p.setPen( pen1 );

        p.drawLine( cursorX, 0, cursorX, height() );       //Horizontal Cursor V line
        p.drawLine( m_ceroX, cursorY, m_endX, cursorY );   //Horizontal Cursor H line

        pen1.setColor( Qt::white );
        p.setPen( pen1 );
        double val = (double)(cursorX-m_hCenter)*1e3/m_scaleX; // Time in ps *1e3 bcos sim time is in ns
        QString unit;
        int decs = 0;
        valToUnit( val, unit, decs );

        p.drawText( cursorX-62, cursorY-12, 60, 12, Qt::AlignRight, QString::number( val,'f', decs) );
        p.drawText( cursorX-1,  cursorY-12, 20, 12, Qt::AlignLeft, unit+"s" );
    }
    p.end();
}


#include "moc_plotdisplay.cpp"
