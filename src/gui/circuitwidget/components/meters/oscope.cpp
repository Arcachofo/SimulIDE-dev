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

#include "oscope.h"
#include "connector.h"
#include "circuitwidget.h"
#include "circuit.h"
#include "simulator.h"
#include "itemlibrary.h"
#include "oscopechannel.h"
#include "oscwidget.h"
#include "datawidget.h"
#include "tunnel.h"

static const char* Oscope_properties[] = {
    QT_TRANSLATE_NOOP("App::Property","Filter")
};

Component* Oscope::construct( QObject* parent, QString type, QString id )
{
    return new Oscope( parent, type, id );
}

LibraryItem* Oscope::libraryItem()
{
    return new LibraryItem(
        tr( "Oscope" ),
        tr( "Meters" ),
        "oscope.png",
        "Oscope",
        Oscope::construct );
}

Oscope::Oscope( QObject* parent, QString type, QString id )
      : PlotBase( parent, type, id )
{
    Q_UNUSED( Oscope_properties );

    m_graphical = true;
    m_trigger = 4;
    m_auto    = 4;
    m_filter = 0.0;
    m_extraSize = 68;
    m_bufferSize = 600000;

    m_oscWidget  = new OscWidget( CircuitWidget::self(), this );
    m_dataWidget = new DataWidget( NULL, this );
    m_proxy = Circuit::self()->addWidget( m_dataWidget );
    m_proxy->setParentItem( this );
    m_dataWidget->show();
    m_display = m_oscWidget->display();
    m_display->setFixedSize( m_baSizeX+6*8, m_baSizeY+2*8 );

    m_pin.resize(5);
    m_pin[4] = new Pin( 180, QPoint( -80-8, 64 ), id+"-PinG", 0, this );

    for( int i=0; i<4; i++ )
    {
        m_pin[i] = new Pin( 180, QPoint( -80-8,-48+32*i ), id+"-Pin"+QString::number(i), 0, this );
        m_channel[i] = new OscopeChannel( this, id+"Chan"+QString::number(i) );
        m_channel[i]->m_channel = i;
        m_channel[i]->m_ePin[0] = m_pin[i];
        m_channel[i]->m_ePin[1] = m_pin[4]; // Ref Pin
        m_channel[i]->m_buffer.resize( m_bufferSize );
        m_channel[i]->m_time.resize( m_bufferSize );

        m_hideCh[i] = false;

        m_display->setChannel( i, m_channel[i] );
        m_display->setColor( i, m_color[i] );
        m_dataWidget->setColor( i, m_color[i] );

        setTimePos( i, 0 );
        setVoltDiv( i, 1 );
        setVoltPos( i, 0 );
    }
    setTimeDiv( 1e6 ); // 1 ms
    setLabelPos(-90,-100, 0);
    expand( false );
}
Oscope::~Oscope()
{
    m_proxy->setWidget( NULL );
    delete m_dataWidget;

    m_oscWidget->setParent( NULL );
    m_oscWidget->close();
    delete m_oscWidget;

    for( int i=0; i<4; i++ ) delete m_channel[i];
}

void Oscope::updateStep()
{
    uint64_t period = 0;
    uint64_t timeFrame = m_timeDiv*10;
    uint64_t simTime;

    if( m_trigger < 4  ) period = m_channel[m_trigger]->m_period; // We want a trigger

    if( period > 10 ) // We have a Trigger
    {
        uint64_t risEdge = m_channel[m_trigger]->m_risEdge;

        uint64_t nCycles = timeFrame/period;
        if( timeFrame%period ) nCycles++;
        if( nCycles%2 )        nCycles++;

        uint64_t delta = nCycles*period/2-timeFrame/2;
        if( delta > risEdge ) delta = risEdge;
        simTime = risEdge-delta;
    }
    else simTime = Simulator::self()->circTime(); // free running

    m_display->setTimeEnd( simTime );

    for( int i=0; i<4; i++ )
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
        m_channel[i]->m_subRate = (double)timeFrame/5e10;
        m_channel[i]->m_connected = connected;
        if( connected ) m_channel[i]->updateStep();
        m_channel[i]->m_trigIndex = m_channel[i]->m_bufferCounter;
    }
    m_display->update(); //redrawScreen();
}

void Oscope::expand( bool e )
{
    m_expand = e;

    if( e )
    {
        m_screenSizeY = m_baSizeY+2*10;
        m_display->setMaximumSize( 9999, 9999 );
        m_oscWidget->getLayout()->addWidget( m_display );
        m_oscWidget->setWindowTitle( idLabel() );
        m_oscWidget->show();
        m_screenSizeX = 8;
    }else{
        m_screenSizeX = m_baSizeX+2*4;
        m_screenSizeY = m_baSizeY+2*4;
        m_oscWidget->hide();
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
    m_display->updateValues();
    //QTimer::singleShot( 20, m_display, SLOT( updateValues() ) );

    Circuit::self()->update();
}

void Oscope::channelChanged( int ch, QString name )
{
    m_channel[ch]->m_chTunnel = name;
}

void Oscope::setFilter( double filter )
{
    m_filter = filter;
    for( int i=0; i<2; i++ ) m_channel[i]->setFilter( filter );
}

void Oscope::setTrigger( int ch )
{
    m_trigger = ch;
    m_oscWidget->setTrigger( ch );

    if( ch > 3 ) return;
    for( int i=0; i<4; ++i )
    {
        if( ch == i ) m_channel[i]->m_trigger = true;
        else          m_channel[i]->m_trigger = false;
    }
}

void Oscope::setAutoSC( int ch )
{
    m_auto = ch;
    m_oscWidget->setAuto( ch );
}


QStringList Oscope::hideCh()
{
    QStringList list;
    QString hide;
    for( int i=0; i<4; ++i )
    {
        hide = m_hideCh[i]? "true":"false";
        list << hide;
    }
    return list;
}

void Oscope::setHideCh( QStringList hc )
{
    for( int i=0; i<4; ++i )
    {
        if( i == hc.size() ) break;
        bool hide = (hc.at(i) == "true")? true:false;
        hideChannel( i, hide );
    }
}

void Oscope::hideChannel( int ch, bool hide )
{
    if( ch > 3 ) return;
    m_hideCh[ch] = hide;
    m_display->hideChannel( ch , hide );
    m_oscWidget->hideChannel( ch, hide );
}

int Oscope::tracks() { return m_display->tracks(); }
void Oscope::setTracks( int tracks )
{
    m_display->setTracks( tracks );
    m_oscWidget->setTracks( tracks );
}

void Oscope::setTimeDiv( uint64_t td )
{
    if( td < 1 ) td = 1;
    PlotBase::setTimeDiv( td );
    m_oscWidget->updateTimeDivBox( td );
}

QStringList Oscope::timPos()
{
    QStringList list;
    for( int i=0; i<4; ++i ) list << QString::number( m_timePos[i] );
    return list;
}

void Oscope::setTimPos( QStringList tp )
{
    for( int i=0; i<4; ++i )
    {
        if( i == tp.size() ) break;
        setTimePos( i, tp.at(i).toLongLong() );
    }
}

QStringList Oscope::volDiv()
{
    QStringList list;
    for( int i=0; i<4; ++i ) list << QString::number( m_voltDiv[i] );
    return list;
}

void Oscope::setVolDiv( QStringList vd )
{
    for( int i=0; i<4; ++i )
    {
        if( i == vd.size() ) break;
        setVoltDiv( i, vd.at(i).toDouble() );
    }
}

QStringList Oscope::volPos()
{
    QStringList list;
    for( int i=0; i<4; ++i ) list << QString::number( m_voltPos[i] );
    return list;
}

void Oscope::setVolPos( QStringList vp )
{
    for( int i=0; i<4; ++i )
    {
        if( i == vp.size() ) break;
        setVoltPos( i, vp.at(i).toDouble() );
    }
}

void Oscope::setTimePos( int ch, int64_t tp )
{
    m_timePos[ch] = tp;
    m_display->setHPos( ch, tp );
    m_oscWidget->updateTimePosBox( ch, tp );
}

void Oscope::setVoltDiv( int ch, double vd )
{
    m_voltDiv[ch] = vd;
    m_display->setVTick( ch, vd );
    m_oscWidget->updateVoltDivBox( ch, vd );
}

void Oscope::setVoltPos( int ch, double vp )
{
    m_voltPos[ch] = vp;
    m_display->setVPos( ch, vp );
    m_oscWidget->updateVoltPosBox( ch, vp );
}

QStringList Oscope::tunnels()
{
    QStringList list;
    for( int i=0; i<4; ++i ) list << m_channel[i]->m_chTunnel;
    return list;
}

void Oscope::setTunnels( QStringList tunnels )
{
    for( int i=0; i<tunnels.size(); i++ )
    {
        if( i > 3 ) break;
        m_channel[i]->m_chTunnel = tunnels.at(i);
        m_dataWidget->setTunnel( i, tunnels.at(i) );
    }
}

#include "moc_oscope.cpp"
