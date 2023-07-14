/***************************************************************************
 *   Copyright (C) 2010 by Santiago González                               *
 *                                                                         *
 ***( see copyright.txt file at root folder )*******************************/

#include "clock-base.h"
#include "iopin.h"
#include "simulator.h"
#include "custombutton.h"

#include "boolprop.h"

ClockBase::ClockBase( QObject* parent, QString type, QString id )
         : FixedVolt( parent, type, id )
{
    m_area = QRect(-14,-8, 22, 16 );

    m_graphical = true;
    m_isRunning = false;
    m_alwaysOn  = false;

    m_stepsPC = 0;
    ClockBase::setFreq( 1000 );

    Simulator::self()->addToUpdateList( this );

    addPropGroup( { "Hidden1", {
new BoolProp<ClockBase>( "Running", "","", this, &ClockBase::running, &ClockBase::setRunning ),
    }, groupHidden} );
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
    m_changed = false;

    if( m_isRunning )
    {
        m_lastTime = Simulator::self()->circTime();
        Simulator::self()->cancelEvents( this );
        Simulator::self()->addEvent( m_stepsPC/2, this );
    }
    m_outpin->setOutState( false );
    m_state = false;
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

    setRunning( m_isRunning && (freq>0) );
}

void ClockBase::setRunning( bool running )
{
    running = running && (m_freq>0);
    m_button->setChecked( running );
    m_isRunning = running;
    m_changed = true;
    update();
}

void ClockBase::setLinkedValue( int v, int )
{
    setFreq( v );
}


void ClockBase::setHidden( bool hide, bool hidArea, bool hidLabel )
{
    Component::setHidden( hide, hidArea, hidLabel );
    if  ( hidArea ) m_area = QRectF( 0, 0,-4,-4 );     // Totally hidden
    else if( hide ) m_area = QRectF(-30,-6, 12, 12 );  // In Board
    else            m_area = QRect(-14,-8, 22, 16 );   // Normal

    m_proxy->setFlag( QGraphicsItem::ItemStacksBehindParent, hide && !hidArea );
}

void ClockBase::onbuttonclicked() { setRunning( !m_isRunning ); }
