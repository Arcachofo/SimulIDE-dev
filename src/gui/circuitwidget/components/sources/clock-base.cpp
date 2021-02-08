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
#include "pin.h"
#include "simulator.h"

static const char* ClockBase_properties[] = {
    QT_TRANSLATE_NOOP("App::Property","Freq")
};

ClockBase::ClockBase( QObject* parent, QString type, QString id )
         : LogicInput( parent, type, id )
{
    Q_UNUSED( ClockBase_properties );

    m_graphical = true;
    
    m_area = QRect( -14, -8, 22, 16 );
    
    m_isRunning = false;
    m_alwaysOn  = false;

    m_stepsPC = 0;
    setFreq( 1000 );

    Simulator::self()->addToUpdateList( this );

    connect( Simulator::self(), &Simulator::rateChanged,
             this,              &ClockBase::rateChanged, Qt::UniqueConnection );
}
ClockBase::~ClockBase(){}

void ClockBase::stamp()
{
    setFreq( m_freq );
    if( !Simulator::self()->isPaused() ) m_changed = true;
}

void ClockBase::updateStep()
{
    if( m_changed )
    {
        if( m_isRunning )
        {
            eLogicDevice::setOut( 0, true );
            Simulator::self()->cancelEvents( this );
            Simulator::self()->addEvent( 1, this );
        }
        else
        {
            eLogicDevice::setOut( 0, false );
            Simulator::self()->addEvent( 0, NULL );
        }
        m_changed = false;
    }
}

void ClockBase::setAlwaysOn( bool on )
{
    m_alwaysOn = on;
    setRunning( on );
    m_button->setVisible( !on );
}

void ClockBase::setFreq( double freq )
{
    m_fstepsPC = 1e6*1e6/freq;
    m_stepsPC  = m_fstepsPC;
    
    m_freq = freq;
    m_remainder = 0;
    
    emit freqChanged();
}

void ClockBase::rateChanged()
{
    setFreq( m_freq );
}

bool ClockBase::running() { return m_isRunning; }

void ClockBase::setRunning( bool running )
{
    m_button->setChecked( running );
    m_isRunning = running;
    m_changed = true;
    update();
}

void ClockBase::onbuttonclicked()
{
    setRunning( !m_isRunning );
}

#include "moc_clock-base.cpp"

