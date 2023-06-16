/***************************************************************************
 *   Copyright (C) 2018 by Santiago González                               *
 *                                                                         *
 ***( see copyright.txt file at root folder )*******************************/

#include <QGraphicsProxyWidget>

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
#include "e-node.h"

#include "stringprop.h"
#include "doubleprop.h"
#include "intprop.h"

Component* Oscope::construct( QObject* parent, QString type, QString id )
{ return new Oscope( parent, type, id ); }

LibraryItem* Oscope::libraryItem()
{
    return new LibraryItem(
        tr( "Oscope" ),
        "Meters",
        "oscope.png",
        "Oscope",
        Oscope::construct );
}

Oscope::Oscope( QObject* parent, QString type, QString id )
      : PlotBase( parent, type, id )
{
    m_numChannels = 4;
    m_trigger = 4;
    m_auto    = 4;

    m_oscWidget  = new OscWidget( CircuitWidget::self(), this );
    m_dataWidget = new DataWidget( NULL, this );
    m_proxy = Circuit::self()->addWidget( m_dataWidget );
    m_proxy->setParentItem( this );
    m_dataWidget->show();
    m_display = m_oscWidget->display();
    m_display->setPlotBase( this );
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
    setFilter( 0.1 );
    setTimeDiv( 1e9 ); // 1 ms
    setLabelPos(-90,-100, 0);
    expand( false );

    addPropGroup( { "Hidden1", {
new DoubProp<Oscope>("Filter", tr("Filter"), "V", this, &Oscope::filter, &Oscope::setFilter ),
new IntProp <Oscope>("Trigger","", "", this, &Oscope::trigger, &Oscope::setTrigger ),
new IntProp <Oscope>("AutoSC" ,"", "", this, &Oscope::autoSC,  &Oscope::setAutoSC ),
new IntProp <Oscope>("Tracks" ,"", "", this, &Oscope::tracks,  &Oscope::setTracks ),
new StrProp <Oscope>("HideCh" ,"", "", this, &Oscope::hideCh,  &Oscope::setHideCh ),
new StrProp <Oscope>("VolPos" ,"", "", this, &Oscope::volPos,  &Oscope::setVolPos )
    }, groupHidden } );
}
Oscope::~Oscope()
{
    m_proxy->setWidget( NULL );
    delete m_dataWidget;

    m_oscWidget->setParent( NULL );
    m_oscWidget->close();
    delete m_oscWidget;
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

            eNode* enode = Tunnel::getEnode( chTunnel );
            m_pin[i]->setEnode( enode );
            if( enode )
            {
                enode->voltChangedCallback( m_channel[i] );
                connected = true;
            }
            display()->connectChannel( i, connected );
        }
        m_channel[i]->m_connected = connected;
        if( connected ) m_channel[i]->updateStep();
        m_channel[i]->m_trigIndex = m_channel[i]->m_bufferCounter;
    }
    m_display->update(); //redrawScreen();
}

void Oscope::expand( bool e )
{
    m_expand = e;
    m_screenSizeY = m_baSizeY+2*4;
    if( e ){
        m_display->setMaximumSize( 9999, 9999 );
        m_oscWidget->getLayout()->addWidget( m_display );
        m_oscWidget->setWindowTitle( idLabel() );
        m_oscWidget->show();
        m_screenSizeX = 0;
    }else{
        m_screenSizeX = m_baSizeX+2*4;
        m_oscWidget->hide();
        m_dataWidget->getLayout()->addWidget( m_display );
        m_display->setMaximumSize( m_screenSizeX, m_screenSizeY );
    }
    m_display->setMinimumSize( m_screenSizeX, m_screenSizeY );

    int widgetSizeX = m_screenSizeX+68+4;
    int widgetSizeY = m_screenSizeY+4;
    int centerY = widgetSizeY/2;
    m_dataWidget->setFixedSize( widgetSizeX, widgetSizeY );
    m_proxy->setPos( -80+2, -centerY-2+4 );
    m_area = QRectF( -80, -centerY, widgetSizeX+4, widgetSizeY+4+2 );

    m_display->setExpand( e );
    m_display->updateValues();
    Circuit::self()->update();
}

void Oscope::setFilter( double filter )
{
    m_filter = filter;
    m_oscWidget->setFilter( filter );
    for( int i=0; i<4; i++ ) m_channel[i]->setFilter( filter );
}

void Oscope::setTrigger( int ch )
{
    m_trigger = ch;
    m_oscWidget->setTrigger( ch );

    if( ch > 3 ) return;
    for( int i=0; i<4; ++i ){
        if( ch == i ) m_channel[i]->m_trigger = true;
        else          m_channel[i]->m_trigger = false;
}   }

void Oscope::setTunnels( QString tunnels )
{
    QStringList list = tunnels.split(",");
    for( int i=0; i<list.size(); i++ ){
        if( i >= m_numChannels ) break;
        m_channel[i]->m_chTunnel = list.at(i);
        m_dataWidget->setTunnel( i, list.at(i) );
}   }

void Oscope::setAutoSC( int ch )
{
    m_auto = ch;
    m_oscWidget->setAuto( ch );
}

QString Oscope::hideCh()
{
    QString list;
    QString hide;
    for( int i=0; i<4; ++i ){
        hide = m_hideCh[i]? "true":"false";
        list.append( hide ).append(",");
    }
    return list;
}

void Oscope::setHideCh( QString hc )
{
    QStringList list = hc.split(",");
    for( int i=0; i<4; ++i ){
        if( i == list.size() ) break;
        bool hide = (list.at(i) == "true")? true:false;
        hideChannel( i, hide );
}   }

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

QString Oscope::timPos()
{
    QString list;
    for( int i=0; i<4; ++i ) list.append( QString::number( m_timePos[i] )).append(",");
    return list;
}

void Oscope::setTimPos( QString tp )
{
    QStringList list = tp.split(",");
    for( int i=0; i<4; ++i ){
        if( i == list.size() ) break;
        setTimePos( i, list.at(i).toLongLong() );
}   }

QString Oscope::volDiv()
{
    QString list;
    for( int i=0; i<4; ++i ) list.append( QString::number( m_voltDiv[i] )).append(",");
    return list;
}

void Oscope::setVolDiv( QString vd )
{
    QStringList list = vd.split(",");
    for( int i=0; i<4; ++i ){
        if( i == list.size() ) break;
        setVoltDiv( i, list.at(i).toDouble() );
}   }

QString Oscope::volPos()
{
    QString list;
    for( int i=0; i<4; ++i ) list.append( QString::number( m_voltPos[i] )).append(",");
    return list;
}

void Oscope::setVolPos( QString vp )
{
    QStringList list = vp.split(",");
    for( int i=0; i<4; ++i ){
        if( i == list.size() ) break;
        setVoltPos( i, list.at(i).toDouble() );
}   }

void Oscope::setTimePos( int ch, int64_t tp )
{
    m_timePos[ch] = tp;
    m_display->setHPos( ch, tp );
    m_oscWidget->updateTimePosBox( ch, tp );
}

void Oscope::moveTimePos( int64_t delta )
{
    if( m_trigger < 4  )
    {
        uint64_t period = m_channel[m_trigger]->m_period; // check wave period
        if( period > 10 ) return;                // We have a Trigger: Don't Zoom at mouse pos.
    }
    for( int i=0; i<4; ++i )
    {
        m_timePos[i] = m_timePos[i]+delta;
        m_display->setHPos( i, m_timePos[i] );
        m_oscWidget->updateTimePosBox( i, m_timePos[i] );
    }
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

