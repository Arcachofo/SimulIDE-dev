/***************************************************************************
 *   Copyright (C) 2019 by santiago González                               *
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

#include "frequencimeter.h"
#include "simulator.h"
#include "pin.h"

Component* Frequencimeter::construct( QObject* parent, QString type, QString id )
{ return new Frequencimeter( parent, type, id ); }

LibraryItem* Frequencimeter::libraryItem()
{
    return new LibraryItem(
            tr( "Frequencimeter" ),
            tr( "Meters" ),
            "frequencimeter.png",
            "Frequencimeter",
            Frequencimeter::construct);
}

Frequencimeter::Frequencimeter( QObject* parent, QString type, QString id )
              : Component( parent, type, id )
              , eElement( id )
              , m_display( this )
{
    m_area = QRectF( -32, -10, 85, 20 );
    m_color = Qt::black;

    m_ePin.resize( 1 );
    m_pin.resize( 1 );
    QString pinId = m_id;
    pinId.append(QString("-lPin"));
    QPoint pinPos = QPoint(-40, 0);
    m_pin[0] = new Pin( 180, pinPos, pinId, 0, this);
    m_ePin[0] = m_pin[0];
    
    m_idLabel->setPos(-12,-24);
    setLabelPos(-32,-24, 0);

    QFont f( "Helvetica [Cronyx]", 10, QFont::Bold );
    f.setPixelSize(12);
    m_display.setFont(f);
    m_display.setBrush(  Qt::yellow );
    m_display.setPos( -30, -6 );
    m_display.setVisible( true );

    m_filter   = 0.1;

    Simulator::self()->addToUpdateList( this );

    initialize();
}
Frequencimeter::~Frequencimeter()
{
    Simulator::self()->remFromUpdateList( this );
}

QList<propGroup_t> Frequencimeter::propGroups()
{
    propGroup_t mainGroup { tr("Main") };
    mainGroup.propList.append( {"Filter", tr("Filter"),"V"} );
    return {mainGroup};
}

void Frequencimeter::initialize()
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

void Frequencimeter::stamp()                     // Called at Simulation Start
{
    eNode* enode =  m_ePin[0]->getEnode();
    if( enode ) enode->voltChangedCallback( this );
}

void Frequencimeter::updateStep()
{
    double freq = m_freq;
    if( m_period > 0 )  // We have a wave
    {
        uint64_t simTime = Simulator::self()->circTime();
        uint64_t stepsPF  = Simulator::self()->stepsPerFrame();
        uint64_t stepSize = Simulator::self()->stepSize();
        uint64_t lost = m_period*2;
        if( lost < stepsPF*2 ) lost = stepsPF*stepSize*2;

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
        }
    }

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
                }
            }
        }
        if     ( freq < 10 )   Fdecs = 4;
        else if( freq < 100 )  Fdecs = 3;
        else if( freq < 1000 ) Fdecs = 2;
        m_display.setText(QString::number( freq, 'f', Fdecs )+unit );
    }
}

void Frequencimeter::voltChanged()
{
    uint64_t simTime = Simulator::self()->circTime();
    double data = m_ePin[0]->getVolt();
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
    }
}

void Frequencimeter::paint( QPainter *p, const QStyleOptionGraphicsItem *option, QWidget *widget )
{
    Component::paint( p, option, widget );
    p->setBrush( Qt::black);

    p->drawRect( m_area );
}

//#include "moc_frequencimeter.cpp"
