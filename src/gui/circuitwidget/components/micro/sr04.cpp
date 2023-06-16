/***************************************************************************
 *   Copyright (C) 2019 by Santiago González                               *
 *                                                                         *
 ***( see copyright.txt file at root folder )*******************************/

#include <QPainter>

#include "sr04.h"
#include "iopin.h"
#include "itemlibrary.h"
#include "simulator.h"

Component* SR04::construct( QObject* parent, QString type, QString id )
{ return new SR04( parent, type, id ); }

LibraryItem* SR04::libraryItem()
{
    return new LibraryItem(
        "HC-SR04",
        "Sensors",
        "sr04.png",
        "SR04",
        SR04::construct);
}

SR04::SR04( QObject* parent, QString type, QString id )
    : Component( parent, type, id )
    , eElement( id )
{
    m_area = QRect( -10*8, -4*8, 21*8, 9*8 );
    setLabelPos(-16,-48, 0);
    m_background = ":/sr04.png";
    
    m_pin.resize(5);

    m_inpin = new Pin( 180, QPoint(-11*8,-3*8), id+"-inpin", 0, this );
    m_inpin->setLabelText( " In v=m" );
    m_pin[0] = m_inpin;

    Pin* vccPin = new Pin( 270, QPoint(-8,48), id+"-vccpin", 0, this );
    vccPin->setLabelText( " Vcc" );
    vccPin->setUnused( true );
    m_pin[1] = vccPin;

    Pin* gndPin = new Pin( 270, QPoint(16,48), id+"-gndpin", 0, this );
    gndPin->setLabelText( " Gnd" );
    gndPin->setUnused( true );
    m_pin[2] = gndPin;

    m_trigpin = new Pin( 270, QPoint(0,48), id+"-trigpin", 0, this );
    m_trigpin->setLabelText( " Trig" );
    m_pin[3] = m_trigpin;
    
    m_echo = new IoPin( 270, QPoint(8,48), id+"-outpin", 0, this, output );
    m_echo->setLabelText( " Echo" );
    m_echo->setOutHighV( 5 );
    m_pin[4] = m_echo;
    
    SR04::initialize();
}
SR04::~SR04(){}

void SR04::stamp()
{
    m_trigpin->changeCallBack( this ); // Register for Trigger Pin changes
}

void SR04::initialize()
{
    m_lastStep = Simulator::self()->circTime();
    m_lastTrig = false;
    m_echouS = 0;
}

void SR04::voltChanged()              // Called when Trigger Pin changes
{
    bool trigState = m_trigpin->getVoltage()>2.5;
    
    if( !m_lastTrig && trigState )                 // Rising trigger Pin
    {
        m_lastStep = Simulator::self()->circTime();
    }
    else if( m_lastTrig && !trigState )            // Triggered
    {
        uint64_t time = Simulator::self()->circTime()-m_lastStep; // in picosecondss

        if( time >= 10*1e6 )     // >=10 uS Trigger pulse
        {
            m_echouS = (m_inpin->getVoltage()*2000/0.344+0.5);
            if     ( m_echouS < 116 )   m_echouS = 116;   // Min range 2 cm = 116 us pulse
            else if( m_echouS > 38000 ) m_echouS = 38000; // Timeout 38 ms
            
            Simulator::self()->addEvent( 200*1e6, this ); // Send echo after 200 us
        }
    }
    m_lastTrig = trigState;
}

void SR04::runEvent()
{
    if( m_echouS )
    {
        m_echo->sheduleState( true, 0 );
        Simulator::self()->addEvent( m_echouS*1e6, this ); // Event to finish echo
        m_echouS = 0;
    }
    else m_echo->sheduleState( false, 0 );
}

void SR04::paint( QPainter* p, const QStyleOptionGraphicsItem* option, QWidget* widget )
{
    Component::paint( p, option, widget );
    
    p->drawRoundedRect( m_area, 2, 2 );

    int ox = m_area.x();
    int oy = m_area.y();

    p->drawPixmap( ox, oy, QPixmap( m_background ));
}
