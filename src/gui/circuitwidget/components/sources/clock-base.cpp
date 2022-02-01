/***************************************************************************
 *   Copyright (C) 2010 by santiago González                               *
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

#include "clock-base.h"
#include "iopin.h"
#include "simulator.h"

#include "boolprop.h"

ClockBase::ClockBase( QObject* parent, QString type, QString id )
         : FixedVolt( parent, type, id )
{
    m_area = QRect( -14, -8, 22, 16 );

    m_graphical = true;
    m_isRunning = false;
    m_alwaysOn  = false;

    m_stepsPC = 0;
    setFreq( 1000 );

    Simulator::self()->addToUpdateList( this );

    addPropGroup( { "Hidden1", {
new BoolProp<ClockBase>( "Running", "","", this, &ClockBase::running, &ClockBase::setRunning ),
    }} );
}
ClockBase::~ClockBase(){}

bool ClockBase::setPropStr( QString prop, QString val )
{
    if( prop =="Out" ) setRunning( val == "true" );   // Old: TODELETE
    else return Component::setPropStr( prop, val );
    return true;
}

void ClockBase::stamp()
{
    setFreq( m_freq );
    if( !Simulator::self()->isPaused() ) m_changed = true;
}

void ClockBase::updateStep()
{
    if( !m_changed ) return;
    if( m_isRunning )
    {
        Simulator::self()->cancelEvents( this );
        Simulator::self()->addEvent( 1, this );
    }
    m_outpin->sheduleState( false, 0 );
    m_state = false;
    m_changed = false;
}

void ClockBase::setAlwaysOn( bool on )
{
    m_alwaysOn = on;
    if( on ) setRunning( on );
    m_button->setVisible( !on );
}

void ClockBase::setFreq( double freq )
{
    m_fstepsPC = 1e6*1e6/freq;
    m_stepsPC  = m_fstepsPC;
    
    m_freq = freq;
    m_remainder = 0;
}

void ClockBase::setRunning( bool running )
{
    m_button->setChecked( running );
    m_isRunning = running;
    m_changed = true;
    update();
}

void ClockBase::onbuttonclicked() { setRunning( !m_isRunning ); }

#include "moc_clock-base.cpp"
