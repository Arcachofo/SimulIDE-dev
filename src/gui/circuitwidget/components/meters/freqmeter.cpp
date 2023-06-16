/***************************************************************************
 *   Copyright (C) 2019 by Santiago González                               *
 *                                                                         *
 ***( see copyright.txt file at root folder )*******************************/

#include <QPainter>
#include <QFont>

#include "freqmeter.h"
#include "itemlibrary.h"
#include "simulator.h"
#include "pin.h"
#include "label.h"

#include "doubleprop.h"

Component* FreqMeter::construct( QObject* parent, QString type, QString id )
{ return new FreqMeter( parent, type, id ); }

LibraryItem* FreqMeter::libraryItem()
{
    return new LibraryItem(
        tr( "FreqMeter" ),
        "Meters",
        "frequencimeter.png",
        "FreqMeter",
        FreqMeter::construct);
}

FreqMeter::FreqMeter( QObject* parent, QString type, QString id )
         : Component( parent, type, id )
         , eElement( id )
         , m_display( this )
{
    m_area = QRectF( -32, -10, 85, 20 );
    m_color = Qt::black;

    m_ePin.resize( 1 );
    m_pin.resize( 1 );
    m_ePin[0] = m_pin[0] = new Pin( 180, QPoint(-40, 0), id+"-lPin", 0, this);
    
    m_idLabel->setPos(-12,-24);
    setLabelPos(-32,-24, 0);

    QFont f( "Helvetica [Cronyx]", 10, QFont::Bold );
    f.setPixelSize( 12 );
    m_display.setFont( f );
    m_display.setBrush( Qt::yellow );
    m_display.setPos( -30, -6 );
    m_display.setVisible( true );

    m_filter   = 0.1;

    Simulator::self()->addToUpdateList( this );

    FreqMeter::initialize();

    addPropGroup( { tr("Main"), {
new DoubProp<FreqMeter>("Filter", tr("Filter"), "V", this, &FreqMeter::filter, &FreqMeter::setFilter ),
    },0 } );
}
FreqMeter::~FreqMeter(){}

void FreqMeter::initialize()
{
    m_rising  = false;
    m_falling = false;

    m_lastData = 0;
    m_freq     = 0;
    m_numMax   = 0;
    m_lastMax  = 0;
    m_totalP   = 0;
    m_period   = 0;
    
    m_display.setText( "0 Hz" );
}

void FreqMeter::stamp()          // Called at Simulation Start
{
    m_ePin[0]->changeCallBack( this );
}

void FreqMeter::updateStep()
{
    double freq = m_freq;
    if( m_period > 0 )  // We have a wave
    {
        uint64_t simTime = Simulator::self()->circTime();
        uint64_t psPF  = Simulator::self()->psPerFrame();
        uint64_t lost = m_period*2;
        if( lost < psPF*2 ) lost = psPF*2;

        if( simTime-m_lastMax > lost ) // Wave lost
        {
            freq = 0;
            m_period = 0;
            m_totalP = 0;
            m_numMax = 0;
        }
        else if( m_numMax > 1 )
        {
            freq = 1e12/((double)m_totalP/(double)(m_numMax-1));
            m_totalP = 0;
            m_numMax = 0;
    }   }
    if( m_freq != freq )
    {
        m_freq = freq;

        int Fdecs = 1;
        QString unit = "  Hz";

        if( freq > 999 )
        {
            freq /= 1e3; unit = " KHz";
            if( freq > 999 ) {
                freq /= 1e3; unit = " MHz";
                if( freq > 999 ) {
                    freq /= 1e3; unit = " GHz";
        }   }   }
        if     ( freq < 10 )   Fdecs = 4;
        else if( freq < 100 )  Fdecs = 3;
        else if( freq < 1000 ) Fdecs = 2;
        m_display.setText(QString::number( freq, 'f', Fdecs )+unit );
}   }

void FreqMeter::voltChanged()
{
    uint64_t simTime = Simulator::self()->circTime();
    double data = m_ePin[0]->getVoltage();
    double delta = data-m_lastData;
    
    if( delta > m_filter )                         // Rising
    {
        if( m_falling && !m_rising ) m_falling = false; // Min Found

        m_rising = true;
        m_lastData = data;
    }
    else if( delta < -m_filter )                  // Falling
    {
        if( m_rising && !m_falling )                        // Max Found
        {
            if( m_numMax > 0 )
            {
                m_period = simTime-m_lastMax;
                m_totalP += m_period;
            }
            m_lastMax = simTime;
            m_numMax++;
            m_rising = false;
        }
        m_falling = true;
        m_lastData = data;
}   }

void FreqMeter::setflip()
{
    Component::setflip();
    int xlabelpos = (m_Hflip == 1) ? -30 : 51;
    int ylabelpos = -6*m_Vflip;

    m_display.setPos( xlabelpos, ylabelpos );
    m_display.setTransform( QTransform::fromScale( m_Hflip, m_Vflip ) );
}

void FreqMeter::paint( QPainter* p, const QStyleOptionGraphicsItem* option, QWidget* widget )
{
    Component::paint( p, option, widget );
    p->setBrush( Qt::black);
    p->drawRect( m_area );
}
