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
               : IoPin( angle, QPoint( xpos, ypos ), mcuComponent->itemID()+"-"+id, pos, mcuComponent, input )
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

    //Pin* pin = new Pin( angle, QPoint( xpos, ypos ), mcuComponent->itemID()+"-"+id, pos, m_mcuComponent );
    setLabelText( label );

    setOutHighV( 5 );
    setPinMode( undef_mode );

    type = type.toLower();
    if( type == "gnd" 
     || type == "vdd" 
     || type == "vcc" 
     || type == "unused" 
     || type == "nc" ) 
     this->setUnused( true );
     
    initialize();
}
McuComponentPin::~McuComponentPin(){ }

void McuComponentPin::stamp()
{
    if( m_enode && m_attached ) changeCallBack( this ); // Receive voltage change notifications
    IoPin::stamp();
}

void McuComponentPin::initialize()
{
    if( m_pinType == 1 )
    {
        m_enableIO = true;
        setPinMode( input );
    }
    IoPin::initialize();
}

void McuComponentPin::setDirection( bool out )
{
    if( out )       // Set Pin to Output
    {
        if( m_enode && m_attached )
            m_enode->remFromChangedCallback( this ); // Don't Receive voltage change notifications

        if( m_openColl ) setPinMode( open_col );
        else             setPinMode( output );
        //eSource::setState( m_outState, true );
    }
    else           // Set Pin to Input
    {
        if( m_enode && m_attached )
            m_enode->voltChangedCallback( this ); // Receive voltage change notifications

        setPinMode( input );
    }
    setOutState( m_outState );
}

void McuComponentPin::setState( bool state )
{
    //if( state == m_state ) return;

    if( m_pinMode == input )  return;
    if( !m_enableIO ) return;

    IoPin::setOutState( state );
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

    if( !(m_enode && m_attached) ) return;

    if( en )
    {
        setDirection( m_prevPinMode == output );
    }
    else
    {
        m_enode->remFromChangedCallback( this );
    }
    m_prevPinMode = m_pinMode;
}

/*void McuComponentPin::move( int dx, int dy )
{
    this->moveBy( dx, dy );
}*/

#include "moc_mcucomponentpin.cpp"
