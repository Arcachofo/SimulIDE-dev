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

PlotDisplay::PlotDisplay( QWidget* parent )
           : QWidget( parent )
{
    for( int i=0; i<2; i++ )
    {
        m_scaleY[i] = 1;
        m_hPos[i] = 0;
        m_vMax[i] = 0;
        m_vMin[i] = 0;
        m_data[i] = 0l;
        m_foregroundA[i] = QPixmap( 100, 100 );
        m_foregroundB[i] = QPixmap( 100, 100 );
        m_foreground[i]  = &m_foregroundA[i];
    }
    m_margin = 8;
    m_background = QPixmap( 100, 100 );

    double fontScale = MainWindow::self()->fontScale();
    m_fontB.setPixelSize(int(8*fontScale));
    m_fontB.setBold(true);
    m_fontS.setPixelSize(int(7*fontScale));

    m_scaleColor[0] = QColor( 90, 90, 90 );
    m_scaleColor[1] = QColor( 130, 130, 130 );
    m_scaleColor[2] = QColor( 230, 230, 230 );
    m_color[0] = QColor( 240, 240, 100 ); //QColor( 190, 190, 0 );
    m_color[1] = QColor( 220, 220, 255 ); //QColor( 255, 110, 50 );
    m_color[2] = QColor( 255, 255, 255 );

    setMouseTracking(true);
}

QSize PlotDisplay::minimumSizeHint() const  {  return QSize( m_width, m_height );  }

QSize PlotDisplay::sizeHint() const  { return QSize( m_width, m_height ); }

void PlotDisplay::setSize( int width, int height )
{
    m_width  = width+6*m_margin;
    m_height = height+2*m_margin;

    m_sizeX = width;
    m_sizeY = height;
    m_scaleX = 1;

    setFixedSize( m_width, m_height );
    m_background = m_background.scaled( m_width, m_height );

    for( int i=0; i<2; i++ )
    {
        m_foregroundA[i] = m_foregroundA[i].scaled( m_sizeX+2, m_sizeY+2 );
        m_foregroundB[i] = m_foregroundB[i].scaled( m_sizeX+2, m_sizeY+2 );
        m_vTick[i] = 1;
        m_vPos[i] = 0;
        m_data[i] = 0l;
    }
    updateValues();

    QPainter p( &m_background );
    drawBackground( &p );
    p.end();
}

void PlotDisplay::setMaxMin( int ch, double max, double min )
{
    m_vMax[ch]  = max;
    m_vMin[ch]  = min;

    double ampli = max-min;
    m_ampli[ch] = ampli;
    m_scaleY[ch] = ampli/(m_vTick[ch]*10);
    updateValues();
}

void PlotDisplay::setData( int ch, QList<QPointF>* data )
{
    m_data[ch] = data;
    updateValues();
}

void PlotDisplay::setXFrame( uint64_t tf )
{
    m_scaleX = (double)m_sizeX/(double)tf;
    updateValues();
}

void PlotDisplay::setVTick( int ch, double tick )
{
    m_vTick[ch] = tick;
    m_scaleY[ch] = m_ampli[ch]/(tick*10);
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
    m_hCenter = (double)m_width/2;
    m_vCenter = (double)m_height/2;
    m_ceroX = m_margin*3;
    m_endX  = m_width-m_ceroX;
    m_ceroY = m_margin;
    m_endS  = m_height-m_margin;
    m_lineX = m_ceroX;
    m_linWi = (4+6*m_sizeY/200)/10;

    for( int i=0; i<2; i++ )
    {
        m_endY[i] = m_vCenter+(m_height-m_margin*2)*m_scaleY[i]/2;
        m_midV[i] = (m_vMax[i]+m_vMin[i])/2;
        m_sclY[i] = m_sizeY/(m_vTick[i]*10);
        m_posY[i] = m_vMin[i]+m_vPos[i];
        m_vMaxPos[i] = m_endY[i]-(m_vMax[i]-m_posY[i])*m_sclY[i];
        m_vMinPos[i] = m_endY[i]-(m_vMin[i]-m_posY[i])*m_sclY[i];
    }
    update();
}

void PlotDisplay::drawBackground( QPainter* p )
{
    //p->setRenderHint( QPainter::Antialiasing, true );

    p->setBrush( QColor( 0, 0, 0 ) );
    p->drawRoundRect(0, 0, m_width, m_height, 7, 7 );

    QPen pen( m_scaleColor[0], 1.5*m_linWi, Qt::SolidLine, Qt::RoundCap, Qt::RoundJoin );
    p->setPen( pen );
    for( double i=m_ceroY; i<=m_endS; i+=m_sizeY/10. )
        p->drawLine( QPointF(m_ceroX, i), QPointF(m_endX, i) ); //Horizontal lines

    for( double i=m_lineX; i<=m_endX; i+=m_sizeX/10. )
        p->drawLine( QPointF(i, m_ceroY), QPointF(i, m_endS) ); //Vertical lines

    QPen pen2( m_scaleColor[1], 1*m_linWi, Qt::SolidLine, Qt::RoundCap, Qt::RoundJoin );
    p->setPen( pen2 );

    double m = 2*m_linWi;
    for( double i=m_ceroY; i<=m_endS; i+=m_sizeY/50. )
        p->drawLine( QPointF(m_hCenter-m, i), QPointF(m_hCenter+m, i) ); //Horizontal Marks

    for( double i=m_lineX; i<=m_endX; i+=m_sizeX/50. )
        p->drawLine( QPointF(i, m_vCenter-m), QPointF(i, m_vCenter+m) ); //Vertical Marks

    QPen pen3( m_scaleColor[1], 1.5*m_linWi, Qt::SolidLine, Qt::RoundCap, Qt::RoundJoin );
    p->setPen( pen3 );
    p->drawLine( m_ceroX, m_vCenter, m_endX, m_vCenter );  //Horizontal Center line
    p->drawLine( m_hCenter, m_ceroY, m_hCenter, m_endS );  //Vertical Center line
}

/*void PlotDisplay::clear( int ch )
{
    //for( int i=0; i<2; i++ )
        m_foregroundA[ch].fill( Qt::transparent );
        m_foregroundB[ch].fill( Qt::transparent );
}*/

void PlotDisplay::paintEvent( QPaintEvent* /* event */ )
{
    QPainter p( this );

    drawBackground( &p );
    p.setRenderHint( QPainter::Antialiasing, true );

    QPointF P1;
    double x1;
    double y1;
    double x2;
    double y2;

    for( int i=0; i<2; i++ )
    {
      if( m_data[i] )
        {
            QPen pen1( m_color[i], 0.5*m_linWi, Qt::SolidLine, Qt::RoundCap, Qt::RoundJoin );
            p.setPen( pen1 );

            p.drawLine( m_ceroX, m_vMaxPos[i], m_endX, m_vMaxPos[i] );   //Horizontal Max V line
            p.drawLine( m_ceroX, m_vMinPos[i], m_endX, m_vMinPos[i] );   //Horizontal Min V line

            // PRINT DATA
            QPen pen2( m_color[i], 2*m_linWi, Qt::SolidLine, Qt::RoundCap, Qt::RoundJoin );
            p.setPen( pen2 );

            int k = 0;
            double filter = m_filter*m_sclY[i];//m_ampli[i]*m_sclY[i]/4; //

            for( QPointF P2 : *m_data[i] )
            {
                x2 = P2.x()*m_scaleX+m_margin*3+m_hPos[i]*m_scaleX;
                y2 = m_endY[i]-(P2.y()-m_posY[i])*m_sclY[i];
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

                if(( x1<0 && x2<0 ) ||( x1>m_width && x2>m_width )) // Out of display
                {
                    P1 = P2; continue;
                }
                if( fabs( y2-y1 ) <= filter ) p.drawLine( P1, P2 );
                else
                {
                    QPointF PM = QPointF( x2, y1 );
                    p.drawLine( P1, PM );
                    p.drawLine( PM, P2 );
                }
                P1 = P2;
            }

            // Draw Rects to crop data plots
            p.fillRect( 0, 0, m_margin*3, m_height, QColor( 10, 15, 50 ) );
            p.fillRect( m_sizeX+m_margin*3, 0, m_margin*3, m_height, QColor( 10, 15, 50 ) );
        }
    }
    for( int i=0; i<2; i++ ) // Draw scales, text, Max-Min
    {
        if( m_data[i] )
        {
            int xPos = 2+i*(m_sizeX+3*m_margin);

            QPen pen3( m_color[i], 1.5*m_linWi, Qt::SolidLine, Qt::RoundCap, Qt::RoundJoin );
            p.setPen( pen3 );
            p.setFont( m_fontS );

            // Draw Background scale values
            int s = 0;
            for( double j=m_ceroY; j<=m_endS; j+=m_sizeY/10 )
            {
                //p.drawLine( m_ceroX, i, m_endX, i ); //Horizontal lines

                int decs = 0;
                double volTick = (m_vTick[i]*5-m_vTick[i]*s++);
                //int vt = volTick;
                //volTick = ((double)vt+0.5)/100;
                if     ( volTick < 10)   decs = 2;
                else if( volTick < 100)  decs = 1;

                p.drawText( xPos, j-5, m_width, 10, Qt::AlignVCenter, QString::number( volTick,'f', decs) );
            }

            /*p.drawText( xPos,           3, m_width, 10,       Qt::AlignVCenter, QString::number( m_midV[i]+m_vTick[i]*5,'f', 2) );
            p.drawText( xPos, m_vCenter-5, m_width, m_margin, Qt::AlignVCenter, QString::number( m_midV[i]          ,'f', 2) );
            p.drawText( xPos,    m_endS-5, m_width, 10,       Qt::AlignVCenter, QString::number( m_midV[i]-m_vTick[i]*5,'f', 2) );
*/
            // Draw Rects behind Max & Min Numbers
            p.fillRect( xPos, m_vMaxPos[i]-4, 21, 8, QColor( 10, 15, 50 ) );
            p.fillRect( xPos, m_vMinPos[i]-4, 21, 8, QColor( 10, 15, 50 ) );

            // Draw Max & Min Values
            p.setFont( m_fontB );
            //p.setPen( m_color[i] );
            p.drawText( xPos, m_vMaxPos[i]-5, m_width, 10, Qt::AlignVCenter, QString::number(m_vMax[i],'f', 2) );
            p.drawText( xPos, m_vMinPos[i]-5, m_width, 10, Qt::AlignVCenter, QString::number(m_vMin[i],'f', 2) );
        }
    }
    QPoint cPos = QCursor::pos()-mapToGlobal( QPoint(0, 0) );
    qreal scale = CircuitView::self()->getScale();
    int cursorX = cPos.x()/scale;
    int cursorY = cPos.y()/scale;

    if( (cursorX > m_ceroX) && (cursorX < m_endX ) // Draw Cursor
     && (cursorY > 0) && (cursorY < m_height) )
    {
        QPen pen1( m_scaleColor[2], 1*m_linWi, Qt::SolidLine, Qt::RoundCap, Qt::RoundJoin );
        p.setPen( pen1 );

        p.drawLine( cursorX, 0, cursorX, m_height );  //Horizontal Cursor V line
        p.drawLine( m_ceroX, cursorY, m_endX, cursorY );   //Horizontal Cursor H line m_scaleX[i]

        pen1.setColor( Qt::white );
        p.setPen( pen1 );
        double volt = (double)(cursorX-m_hCenter)/m_scaleX/1e6;
        p.drawText( cursorX-25, cursorY-10, 50, 10, Qt::AlignLeft, QString::number( volt,'f', 1)+" us" );
    }
    p.end();
}


#include "moc_plotdisplay.cpp"
