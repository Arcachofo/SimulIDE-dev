/***************************************************************************
 *   Copyright (C) 2016 by santiago González                               *
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

#include "latchd.h"
#include "itemlibrary.h"
#include "circuitwidget.h"
#include "simulator.h"
#include "circuit.h"
#include "iopin.h"


Component* LatchD::construct( QObject* parent, QString type, QString id )
{
    return new LatchD( parent, type, id );
}

LibraryItem* LatchD::libraryItem()
{
    return new LibraryItem(
        tr( "Latch" ),
        tr( "Logic/Memory" ),
        "subc.png",
        "LatchD",
        LatchD::construct );
}

LatchD::LatchD( QObject* parent, QString type, QString id )
      : LogicComponent( parent, type, id )
      , eElement( id )
{
    m_width  = 4;
    m_height = 10;
    
    m_tristate = true;
    
    m_oePin = new IoPin( 0, QPoint( 0,0 ), m_id+"-Pin-outEnable", 0, this, input );
    m_oePin->setLabelText( "OE " );
    m_oePin->setLabelColor( QColor( 0, 0, 0 ) );

    m_clockPin = new IoPin( 180, QPoint( 0,0 ), m_id+"-Pin-clock", 0, this, input );
    m_clockPin->setLabelText( ">" );
    m_clockPin->setLabelColor( QColor( 0, 0, 0 ) );

    setTrigger( InEnable );

    m_channels = 0;
    setChannels( 8 );
}
LatchD::~LatchD(){}

QList<propGroup_t> LatchD::propGroups()
{
    propGroup_t mainGroup { tr("Main") };

    mainGroup.propList.append( {"Tristate", tr("Tristate"),""} );
    mainGroup.propList.append( {"Inverted", tr("Invert Outputs"),""} );
    mainGroup.propList.append( {"Channels", tr("Size"),"Channels"} );
    mainGroup.propList.append( {"Trigger", tr("Trigger Type"),"enum"} );

    QList<propGroup_t> pg = LogicComponent::propGroups();
    pg.prepend( mainGroup );
    return pg;
}

void LatchD::stamp()
{
    if( m_trigger != Clock )
    {
        for( int i=0; i<m_numInputs; ++i )
        {
            eNode* enode = m_inPin[i]->getEnode();
            if( enode ) enode->voltChangedCallback( this );
        }
    }
    LogicComponent::stamp( this );
}

void LatchD::voltChanged()
{
    LogicComponent::updateOutEnabled();

    if( getClockState() == Clock_Allow )
    {
        m_nextOutVal = 0;
        for( int i=0; i<m_numOutputs; ++i )
            if( m_inPin[i]->getInpState() ) m_nextOutVal |= 1<<i;
    }
    sheduleOutPuts( this );
}

void LatchD::createLatches( int n )
{
    int chans = m_channels + n;
    
    int origY = -(m_height/2)*8;
    
    m_outPin.resize( chans );
    m_numOutputs = chans;
    m_inPin.resize( chans );
    m_numInputs = chans;
    
    for( int i=m_channels; i<chans; i++ )
    {
        QString number = QString::number(i);

        m_inPin[i] = new IoPin( 180, QPoint(-24,origY+8+i*8 ), m_id+"-in"+number, i, this, input );
        m_inPin[i]->setLabelText( " D"+number );
        m_inPin[i]->setLabelColor( QColor( 0, 0, 0 ) );

        m_outPin[i] = new IoPin( 0, QPoint(24,origY+8+i*8 ), m_id+"-out"+number, i, this, output );
        m_outPin[i]->setLabelText( "O"+number+" " );
        m_outPin[i]->setLabelColor( QColor( 0, 0, 0 ) );
    }
}

void LatchD::deleteLatches( int n )
{
    LogicComponent::deleteOutputs( n );
    LogicComponent::deleteInputs( n );
}

void LatchD::setChannels( int channels )
{
    if( channels == m_channels ) return;
    if( channels < 1 ) return;
    
    if( Simulator::self()->isRunning() ) CircuitWidget::self()->powerCircOff();

    m_height = channels+2;
    int origY = -(m_height/2)*8;

    if     ( channels < m_channels ) deleteLatches( m_channels-channels );
    else if( channels > m_channels ) createLatches( channels-m_channels );
    
    for( int i=0; i<channels; i++ )
    {
        m_inPin[i]->setPos( QPoint(-24,origY+8+i*8 ) );
        m_inPin[i]->setLabelPos();
        m_inPin[i]->isMoved();
        m_outPin[i]->setPos( QPoint(24,origY+8+i*8 ) ); 
        m_outPin[i]->setLabelPos();
        m_outPin[i]->isMoved();
    }
    
    m_clockPin->setPos( QPoint(-24,origY+8+channels*8 ) );
    m_clockPin->isMoved();
    m_clockPin->setLabelPos();
    
    m_oePin->setPos( QPoint(24,origY+8+channels*8) );
    m_oePin->isMoved();
    m_oePin->setLabelPos();
    
    m_channels = channels;

    updateSize();
}

void LatchD::setTristate( bool t )
{
    if( !t ) 
    {
        if( m_oePin->connector() ) m_oePin->connector()->remove();
        m_oePin->reset();
        m_oePin->setLabelText( "" );
    }
    else m_oePin->setLabelText( "OE " );
    m_oePin->setVisible( t );
    m_tristate = t;

    LogicComponent::updateOutEnabled();
    updateSize();
}

void LatchD::setTrigger( trigger_t trigger )
{
    eClockedDevice::setTrigger( trigger );
    updateSize();
}

void LatchD::updateSize()
{
    int height = m_height;
    if( !m_tristate && (m_trigger == None) ) height--;
    m_area   = QRect( -(m_width/2)*8, -(m_height/2)*8, m_width*8, height*8 );
    Circuit::self()->update();
}

#include "moc_latchd.cpp"
