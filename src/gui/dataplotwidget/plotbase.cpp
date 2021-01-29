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

#include "plotbase.h"
#include "plotdisplay.h"
#include "datachannel.h"
#include "connector.h"
#include "simulator.h"
#include "circuit.h"
#include "itemlibrary.h"
#include "circuitwidget.h"


PlotBase::PlotBase( QObject* parent, QString type, QString id )
        : Component( parent, type, id )
        , eElement( id )
{
    m_pin.resize(5);

    m_pin[0] = new Pin( 180, QPoint( -80-8,-56 ), id+"-Pin0", 0, this );
    m_pin[1] = new Pin( 180, QPoint( -80-8,-24 ), id+"-Pin1", 0, this );
    m_pin[2] = new Pin( 180, QPoint( -80-8, 8  ), id+"-Pin2", 0, this );
    m_pin[3] = new Pin( 180, QPoint( -80-8, 40 ), id+"-Pin3", 0, this );
    m_pin[4] = new Pin( 180, QPoint( -80-8, 64 ), id+"-PinG", 0, this );

    for( int i=0; i<5; i++ )
    {
        m_pin[i]->setLabelColor( QColor( 0, 0, 0 ) );
        //m_pin[i]->setLength( 5 );
    }
    for( int ch=0; ch<4; ch++ )
    {
        m_pinConnected[ch]   = false;
        m_probeConnected[ch] = false;
        m_probe[ch] = "";
    }
    m_color[0] = QColor( 240, 240, 100 );
    m_color[1] = QColor( 220, 220, 255 );
    m_color[2] = QColor( 255, 210, 90  );
    m_color[3] = QColor( 000, 245, 160 );
    m_color[4] = QColor( 255, 255, 255 );

    m_baSizeX = 135;
    m_baSizeY = 135;

    m_expand = false;

    Simulator::self()->addToUpdateList( this );
}
PlotBase::~PlotBase() {}

void PlotBase::initialize()
{
    ///m_dataPlotW->m_refCondFlag = false;
}

void PlotBase::setBaSizeX( int size )
{
    if( size < 135 ) size = 135;
    m_baSizeX = size;
    expand( m_expand );
}

void PlotBase::setBaSizeY( int size )
{
    if( size < 135 ) size = 135;
    m_baSizeY = size;
    expand( m_expand );
}

void PlotBase::pauseOnCond()
{
    bool pause = true;
    if( (m_channel[0]->m_chCond != None)
     && (m_channel[0]->m_chCondFlag == false ) ) pause = false;

    if( (m_channel[1]->m_chCond != None)
     && (m_channel[1]->m_chCondFlag == false ) ) pause = false;

    ///if( (m_dataPlotW->m_refCond != None)
     ///&& (m_dataPlotW->m_refCondFlag == false ) ) pause = false;

    if( pause )
    {
        uint64_t simTime = Simulator::self()->circTime();
        uint64_t orig = simTime - m_dataSize;
        CircuitWidget::self()->pauseSim();

        m_channel[0]->fetchData( orig, orig, 0 );
        m_display->setData( 0, m_channel[0]->m_points );
        m_display->setLimits( 0, m_channel[0]->m_dispMax, m_channel[0]->m_dispMin );

        m_channel[1]->fetchData( orig, orig, 0  );
        m_display->setData( 1, m_channel[1]->m_points );
        m_display->setLimits( 1, m_channel[1]->m_dispMax, m_channel[1]->m_dispMin );

        m_display->setXFrame( m_timeDiv*10 );

        m_channel[0]->m_chCondFlag = false;
        m_channel[1]->m_chCondFlag = false;
        ///m_dataPlotW->m_refCondFlag = false;
    }
}

paCond PlotBase::ch1Cond()
{
    return m_channel[0]->m_chCond;
}

void PlotBase::setCh1Cond( paCond cond )
{
    m_channel[0]->m_chCond = cond;
}

paCond PlotBase::ch2Cond()
{
    return m_channel[1]->m_chCond;
}

void PlotBase::setCh2Cond( paCond cond )
{
    m_channel[1]->m_chCond = cond;
}

void PlotBase::remove()
{
    Simulator::self()->remFromUpdateList( this );

    for( int i=0; i<4; i++ ) delete m_channel[i];

    //m_proxy->setWidget( NULL );
    //delete m_topW;

    Component::remove();
}

void PlotBase::setProbe1( QString p )
{
    m_probe[0] = p;
    //m_dataPlotW->m_channel[0]->setText( p );
    //m_dataPlotW->setProbe( 0 );
}

void PlotBase::setProbe2( QString p )
{
    m_probe[1] = p;
    //m_dataPlotW->m_channel[1]->setText( p );
    //m_dataPlotW->setProbe( 1 );
}

void PlotBase::paint( QPainter* p, const QStyleOptionGraphicsItem* option, QWidget* widget )
{
    Component::paint( p, option, widget );
    
    //p->setBrush( Qt::darkGray );
    p->setBrush(QColor( 230, 230, 230 ));
    p->drawRoundedRect( m_area, 4, 4 );
    
    p->setBrush( Qt::white );
    QPen pen = p->pen();
    pen.setWidth( 0 );
    pen.setColor( Qt::white );
    p->setPen(pen);
    
    //p->drawRoundedRect( QRectF( -80-4, -(m_screenSizeY+20)/2-4, m_screenSizeX+m_extraSize+4, m_screenSizeY+20+4 ), 3, 3 );
}

#include "moc_plotbase.cpp"


