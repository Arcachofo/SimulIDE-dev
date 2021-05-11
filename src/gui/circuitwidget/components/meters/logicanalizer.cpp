/***************************************************************************
 *   Copyright (C) 2018 by santiago González                               *
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

#include "logicanalizer.h"
#include "connector.h"
#include "circuitwidget.h"
#include "circuit.h"
#include "simulator.h"
#include "itemlibrary.h"
#include "lachannel.h"
#include "lawidget.h"
#include "datalawidget.h"
#include "tunnel.h"

Component* LAnalizer::construct( QObject* parent, QString type, QString id )
{
    return new LAnalizer( parent, type, id );
}

LibraryItem* LAnalizer::libraryItem()
{
    return new LibraryItem(
        tr( "Logic Analizer" ),
        tr( "Meters" ),
        "lanalizer.png",
        "LAnalizer",
        LAnalizer::construct );
}

LAnalizer::LAnalizer( QObject* parent, QString type, QString id )
         : PlotBase( parent, type, id )
{
    m_graphical = true;
    m_extraSize = 68-12;
    m_bufferSize = 600000;

    m_laWidget  = new LaWidget( CircuitWidget::self(), this );
    m_dataWidget = new DataLaWidget( NULL, this );
    m_proxy = Circuit::self()->addWidget( m_dataWidget );
    m_proxy->setParentItem( this );
    m_dataWidget->show();
    m_display = m_laWidget->display();
    m_display->setFixedSize( m_baSizeX+6*8, m_baSizeY+2*8 );
    m_display->setChannels( 8 );
    m_display->setTracks( 8 );

    m_pin.resize(8);
    for( int i=0; i<8; i++ )
    {
        m_pin[i] = new Pin( 180, QPoint( -80-8,-64+16*i ), id+"-Pin"+QString::number(i), 0, this );
        m_channel[i] = new LaChannel( this, id+"Chan"+QString::number(i) );
        m_channel[i]->m_channel = i;
        m_channel[i]->m_ePin[0] = m_pin[i];
        m_channel[i]->m_buffer.resize( m_bufferSize );
        m_channel[i]->m_time.resize( m_bufferSize );

        m_display->setChannel( i, m_channel[i] );
        m_display->setColor( i, m_color[i%4] );
        m_display->setLimits( i, 5, 0 );
        m_display->setVPos( i, -2.5 );

        m_dataWidget->setColor( i, m_color[i%4] );
    }
    m_updtCount = 0;
    m_trigger = 0;
    setTimeDiv( 1e9 ); // 1 ms
    setVoltDiv( 0.6 );
    setLabelPos(-90,-100, 0);
    expand( false );
}
LAnalizer::~LAnalizer()
{
    m_proxy->setWidget( NULL );
    delete m_dataWidget;

    m_laWidget->setParent( NULL );
    m_laWidget->close();
    delete m_laWidget;

    for( int i=0; i<8; i++ ) delete m_channel[i];
}

void LAnalizer::updateStep()
{
    if( !m_sampling )
    {
        m_display->update();
        return;
    }

    uint64_t simTime = Simulator::self()->circTime(); // free running

    if( m_trigger < 8 )
    {
        uint64_t risEdge = m_channel[m_trigger]->m_risEdge;

        if( risEdge > 0 ) // We have a Trigger
        {
            if( m_oneShot )
            {
                m_sampling = false;
                for( int i=0; i<8; ++i ) m_channel[i]->m_sampling = false;
            }
            m_channel[m_trigger]->m_risEdge = 0;
            simTime = risEdge;
            m_updtCount = 0;
        }
        else if( ++m_updtCount < 20 ) return;
    }

    m_display->setTimeEnd( simTime );

    for( int i=0; i<8; i++ )
    {
        bool connected = m_pin[i]->connector();
        if( !connected )
        {
            QString chTunnel = m_channel[i]->m_chTunnel;

            eNode* enode = Tunnel::m_eNodes.value(  chTunnel, NULL );
            m_pin[i]->setEnode( enode );
            if( enode )
            {
                enode->voltChangedCallback( m_channel[i] );
                connected = true;
            }
            display()->connectChannel( i, connected );
        }
        if( connected ) m_channel[i]->updateStep();
        else            m_channel[i]->initialize();
        m_channel[i]->m_trigIndex = m_channel[i]->m_bufferCounter;
    }
    m_display->update(); //redrawScreen();
}

void LAnalizer::expand( bool e )
{
    m_expand = e;

    if( e )
    {
        m_screenSizeY = m_baSizeY+2*10;
        m_display->setMaximumSize( 9999, 9999 );
        m_laWidget->getLayout()->addWidget( m_display );
        m_laWidget->setWindowTitle( idLabel() );
        m_laWidget->show();
        m_screenSizeX = 8;
    }else{
        m_screenSizeX = m_baSizeX+2*10;
        m_screenSizeY = m_baSizeY+2*10;
        m_laWidget->hide();
        m_dataWidget->getLayout()->addWidget( m_display );
        m_display->setMaximumSize( m_screenSizeX, m_screenSizeY );
    }
    m_display->setMinimumSize( m_screenSizeX, m_screenSizeY );

    int widgetSizeX = m_screenSizeX+m_extraSize+4;
    int widgetSizeY = m_screenSizeY+4;
    int centerY = widgetSizeY/2;
    m_dataWidget->setFixedSize( widgetSizeX, widgetSizeY );
    m_proxy->setPos( -80+2, -centerY-2+4 );
    m_area = QRectF( -80, -centerY, widgetSizeX+4, widgetSizeY+4+2 );

    m_display->setExpand( e );

    Circuit::self()->update();
}

void LAnalizer::channelChanged( int ch, QString name )
{
    m_channel[ch]->m_chTunnel = name;
}

void LAnalizer::setTimeDiv( uint64_t td )
{
    if( td < 1 ) td = 1;
    PlotBase::setTimeDiv( td );
    m_laWidget->updateTimeDivBox( td );
}

void LAnalizer::setTimePos( int64_t tp )
{
    m_timePos = tp;
    for( int i=0; i<8; ++i ) m_display->setHPos( i, tp );
    m_laWidget->updateTimePosBox( tp );
}

void LAnalizer::setVoltDiv( double vd )
{
    m_voltDiv = vd;
    for( int i=0; i<8; i++ ) m_display->setVTick( i, vd );
    m_laWidget->updateVoltDivBox( vd );
}

void LAnalizer::setTrigger( int ch )
{
    m_trigger = ch;
    m_laWidget->setTrigger( ch );
}

QStringList LAnalizer::tunnels()
{
    QStringList list;
    for( int i=0; i<8; ++i ) list << m_channel[i]->m_chTunnel;
    return list;
}

void LAnalizer::setTunnels( QStringList tunnels )
{
    for( int i=0; i< tunnels.size(); i++ )
    {
        if( i > 7 ) break;
        m_channel[i]->m_chTunnel = tunnels.at(i);
        m_dataWidget->setTunnel( i, tunnels.at(i) );
    }
}

void LAnalizer::setOneShot( bool shot )
{
    m_oneShot = shot;
    m_sampling = true;
    for( int i=0; i<8; ++i ) m_channel[i]->m_sampling = true;
}

#include "moc_logicanalizer.cpp"
