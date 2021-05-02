/***************************************************************************
 *   Copyright (C) 2012 by santiago González                               *
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

#include "mcucomponentpin.h"
#include "baseprocessor.h"
#include "simulator.h"

McuComponentPin::McuComponentPin( McuComponent* mcuComponent, QString id, QString type, QString label, int pos, int xpos, int ypos, int angle )
               : QObject( mcuComponent )
               , eSource( id, 0l )
{
    m_mcuComponent = mcuComponent;
    m_processor = mcuComponent->processor();

    m_id    = id;
    m_type  = type;
    m_angle = angle;
    m_pos   = pos;
    
    m_pinType = 0;
    m_attached = false;
    m_openColl = false;

    Pin* pin = new Pin( angle, QPoint( xpos, ypos ), mcuComponent->itemID()+"-"+id, pos, m_mcuComponent );
    pin->setLabelText( label );
    m_ePin[0] = pin;

    setVoltHigh( 5 );
    setPinMode( input );

    type = type.toLower();
    if( type == "gnd" 
     || type == "vdd" 
     || type == "vcc" 
     || type == "unused" 
     || type == "nc" ) 
     pin->setUnused( true );
     
    initialize();
}
McuComponentPin::~McuComponentPin(){ }

void McuComponentPin::stamp()
{
    if( m_ePin[0]->isConnected() && m_attached )        // Receive voltage change notifications
        m_ePin[0]->getEnode()->voltChangedCallback( this );

    eSource::stamp();
}

void McuComponentPin::initialize()
{
    if( m_pinType == 1 )
    {
        m_enableIO = true;
        setPinMode( input );
    }
    eSource::initialize();
}

void McuComponentPin::setDirection( bool out )
{
    if( out )       // Set Pin to Output
    {
        if( m_ePin[0]->isConnected() && m_attached )
            m_ePin[0]->getEnode()->remFromChangedCallback( this ); // Don't Receive voltage change notifications

        if( m_openColl ) setPinMode( output_open );
        else             setPinMode( output );
        //eSource::setState( m_outState, true );
    }
    else           // Set Pin to Input
    {
        if( m_ePin[0]->isConnected() && m_attached )
            m_ePin[0]->getEnode()->voltChangedCallback( this ); // Receive voltage change notifications

        setPinMode( input );
    }
    setState( m_state );
}

void McuComponentPin::setState( bool state )
{
    //if( state == m_state ) return;

    if( m_pinMode == input )  return;
    if( !m_enableIO ) return;

    eSource::setState( state, true );
}


void McuComponentPin::setPullup( bool up )
{
    if( m_pinMode != input ) return;

    if( up ) m_vddAdmEx = 2/1e5; // Activate pullup
    else     m_vddAdmEx = 0;     // Deactivate pullup

    update();
}

void McuComponentPin::setExtraSource( double vddAdmit, double gndAdmit ) // Comparator Vref out to Pin for example
{
    m_vddAdmEx = vddAdmit;
    m_gndAdmEx = gndAdmit;
    update();
}

void McuComponentPin::enableIO( bool en )
{
    if( m_enableIO == en ) return;
    m_enableIO = en;

    if( !(m_ePin[0]->isConnected() && m_attached) ) return;

    if( en )
    {
        setDirection( m_prevPinMode == output );
    }
    else
    {
        m_ePin[0]->getEnode()->remFromChangedCallback( this );
    }
    m_prevPinMode = m_pinMode;
}

void McuComponentPin::move( int dx, int dy )
{
    pin()->moveBy( dx, dy );
}

#include "moc_mcucomponentpin.cpp"
