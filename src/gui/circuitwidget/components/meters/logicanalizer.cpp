/***************************************************************************
 *   Copyright (C) 2018 by Santiago González                               *
 *                                                                         *
 ***( see copyright.txt file at root folder )*******************************/

#include <QGraphicsProxyWidget>

#include "logicanalizer.h"
#include "itemlibrary.h"
#include "connector.h"
#include "circuitwidget.h"
#include "circuit.h"
#include "simulator.h"
#include "lachannel.h"
#include "lawidget.h"
#include "datalawidget.h"
#include "tunnel.h"
#include "e-node.h"

#include "doubleprop.h"
#include "intprop.h"

Component* LAnalizer::construct( QObject* parent, QString type, QString id )
{ return new LAnalizer( parent, type, id ); }

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
    m_numChannels = 8;
    m_bufferSize = 100000;

    m_laWidget   = new LaWidget( CircuitWidget::self(), this );
    m_dataWidget = new DataLaWidget( NULL, this );
    m_proxy = Circuit::self()->addWidget( m_dataWidget );
    m_proxy->setParentItem( this );
    m_dataWidget->show();
    m_display = m_laWidget->display();
    m_display->setPlotBase( this );
    m_display->setFixedSize( m_baSizeX+6*8, m_baSizeY+2*8 );
    m_display->setChannels( 8 );
    m_display->setTracks( 8 );

    m_pin.resize(8);
    for( int i=0; i<8; ++i )
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

    m_threshold = 2.5;

    setTimePos( 0 );
    setTimeDiv( 1e9 ); // 1 ms
    setVoltDiv( 0.6 );

    setLabelPos(-90,-100, 0);
    expand( false );
    setTrigger( 9 ); // Trigger = None

    addPropGroup( { tr("Electric"), {
new DoubProp<LAnalizer>( "Treshold",tr("Logic Threshold"),"V", this, &LAnalizer::threshold, &LAnalizer::setThreshold )
    } } );
}
LAnalizer::~LAnalizer()
{
    m_proxy->setWidget( NULL );
    delete m_dataWidget;

    m_laWidget->setParent( NULL );
    m_laWidget->close();
    delete m_laWidget;
}

void LAnalizer::updateStep()
{
    if( !Simulator::self()->isPaused() )
    {
        uint64_t simTime = Simulator::self()->circTime(); // free running
        if( m_trigger < 9 )
        {
            uint64_t risEdge;
            if( m_trigger == 8 ) risEdge = m_risEdge;
            else                 risEdge = m_channel[m_trigger]->m_risEdge;

            if( risEdge > 0 ) // We have a Trigger
            {
                if( m_trigger < 8 ) m_channel[m_trigger]->m_risEdge = 0;
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
            m_channel[i]->m_connected = connected;
            if( connected ) m_channel[i]->updateStep();
            else            m_channel[i]->initialize();
            m_channel[i]->m_trigIndex = m_channel[i]->m_bufferCounter;
        }
        m_risEdge = 0;
    }
    m_display->update(); //redrawScreen();
}

void LAnalizer::expand( bool e )
{
    m_expand = e;
    if( e ){
        m_screenSizeY = m_baSizeY+2*4;
        m_display->setMaximumSize( 9999, 9999 );
        m_laWidget->getDispLayout()->insertWidget( 0, m_display );
        m_laWidget->setWindowTitle( idLabel() );
        m_laWidget->show();
        m_screenSizeX = -4-2;
    }else{
        m_screenSizeX = m_baSizeX+2*4;
        m_screenSizeY = m_baSizeY+2*4;
        m_laWidget->hide();
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

QString LAnalizer::timPos()
{
    return QString::number( m_timePos );
}

void LAnalizer::setTimPos( QString tp )
{
    setTimePos( tp.toLongLong() );
}

QString LAnalizer::volDiv()
{
    return QString::number( m_voltDiv );
}

void LAnalizer::setVolDiv( QString vd )
{
    setVoltDiv( vd.toDouble() );
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
    for( int i=0; i<8; ++i ) m_display->setHPos( i, m_timePos );
    m_laWidget->updateTimePosBox( tp );
}

void LAnalizer::moveTimePos( int64_t delta )
{
    m_timePos = m_timePos+delta;
    for( int i=0; i<8; ++i ) m_display->setHPos( i, m_timePos );
    m_laWidget->updateTimePosBox( m_timePos );
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

    bool pauseOnCond = ( m_trigger == 8 ); // Pause on condition
    for( int i=0; i<8; i++ ) m_channel[i]->m_pauseOnCond = pauseOnCond;
}

void LAnalizer::setConds( QString conds )
{
    m_laWidget->setConds( conds );
    updateConds( conds );
}

void LAnalizer::setTunnels( QString tunnels )
{
    QStringList list = tunnels.split(",");
    for( int i=0; i<list.size(); i++ )
    {
        if( i >= m_numChannels ) break;
        m_channel[i]->m_chTunnel = list.at(i);
        m_dataWidget->setTunnel( i, list.at(i) );
}   }
