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
#include "circuitwidget.h"
#include "simulator.h"
#include "circuit.h"
#include "pin.h"


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
      , eLatchD( id )
{
    m_width  = 4;
    m_height = 10;
    
    m_tristate = true;
    
    m_outEnPin = new Pin( 0, QPoint( 0,0 ), m_id+"-Pin-outEnable", 0, this );
    m_outEnPin->setLabelText( "OE " );
    m_outEnPin->setLabelColor( QColor( 0, 0, 0 ) );
    eLogicDevice::createOutEnablePin( m_outEnPin );                    // Output Enable

    m_trigPin = new Pin( 180, QPoint( 0,0 ), m_id+"-Pin-clock", 0, this );
    m_trigPin->setLabelText( ">" );
    m_trigPin->setLabelColor( QColor( 0, 0, 0 ) );
    eLogicDevice::createClockPin( m_trigPin );

    setTrigger( InEnable );

    m_channels = 0;
    setChannels( 8 );
}
LatchD::~LatchD(){}

void LatchD::createLatches( int n )
{
    int chans = m_channels + n;
    
    int origY = -(m_height/2)*8;
    
    m_outPin.resize( chans );
    m_numOutPins = chans;
    m_inPin.resize( chans );
    m_numInPins = chans;
    
    for( int i=m_channels; i<chans; i++ )
    {
        QString number = QString::number(i);

        m_inPin[i] = new Pin( 180, QPoint(-24,origY+8+i*8 ), m_id+"-in"+number, i, this );
        m_inPin[i]->setLabelText( " D"+number );
        m_inPin[i]->setLabelColor( QColor( 0, 0, 0 ) );
        eLogicDevice::createInput( m_inPin[i] );

        m_outPin[i] = new Pin( 0, QPoint(24,origY+8+i*8 ), m_id+"-out"+number, i, this );
        m_outPin[i]->setLabelText( "O"+number+" " );
        m_outPin[i]->setLabelColor( QColor( 0, 0, 0 ) );
        eLogicDevice::createOutput( m_outPin[i] );
    }
}

void LatchD::deleteLatches( int n )
{
    eLogicDevice::deleteOutputs( n );
    eLogicDevice::deleteInputs( n );
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
    
    m_trigPin->setPos( QPoint(-24,origY+8+channels*8 ) );
    m_trigPin->isMoved();
    m_trigPin->setLabelPos();
    
    m_outEnPin->setPos( QPoint(24,origY+8+channels*8) );
    m_outEnPin->isMoved();
    m_outEnPin->setLabelPos();
    
    m_channels = channels;
    m_area   = QRect( -(m_width/2)*8, origY, m_width*8, m_height*8 );

    Circuit::self()->update();
}

void LatchD::setTristate( bool t )
{
    if( !t ) 
    {
        if( m_outEnPin->isConnected() ) m_outEnPin->connector()->remove();
        m_outEnPin->reset();
        m_outEnPin->setLabelText( "" );
    }
    else m_outEnPin->setLabelText( "OE " );
    m_outEnPin->setVisible( t );
    m_tristate = t;
    eLogicDevice::updateOutEnabled();
}

void LatchD::setTrigger( Trigger trigger )
{
    if( Simulator::self()->isRunning() ) CircuitWidget::self()->powerCircOff();

    int trig = static_cast<int>( trigger );
    eLogicDevice::seteTrigger( trig );
    LogicComponent::setTrigger( trigger );

    Circuit::self()->update();
}

void LatchD::remove()
{
    if( m_trigPin->connector() )  m_trigPin->connector()->remove();
    if( m_outEnPin->connector() ) m_outEnPin->connector()->remove();
    
    LogicComponent::remove();
}
#include "moc_latchd.cpp"
