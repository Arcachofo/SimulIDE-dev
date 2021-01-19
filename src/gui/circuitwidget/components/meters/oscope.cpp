/***************************************************************************
 *   Copyright (C) 2018 by santiago González                               *
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

#include "oscope.h"
#include "connector.h"
#include "circuit.h"
#include "simulator.h"
#include "itemlibrary.h"
#include "oscopechannel.h"

static const char* Oscope_properties[] = {
    QT_TRANSLATE_NOOP("App::Property","Filter")
};

Component* Oscope::construct( QObject* parent, QString type, QString id )
{
    return new Oscope( parent, type, id );
}

LibraryItem* Oscope::libraryItem()
{
    return new LibraryItem(
        tr( "Oscope" ),
        tr( "Meters" ),
        "oscope.png",
        "Oscope",
        Oscope::construct );
}

Oscope::Oscope( QObject* parent, QString type, QString id )
      : PlotBase( parent, type, id )
{
    Q_UNUSED( Oscope_properties );

    m_graphical = true;

    setLabelPos(-80,-80, 0);

    m_baSizeX = 120;
    m_baSizeY = 100;
    m_adSizeX = 240;
    m_adSizeY = 200;

    m_bufferSize = 600000;

    for(int i=0; i<2; i++ )
    {
        m_oscCh[i] = new OscopeChannel( (id+"Chan"+QString::number(i)));
        m_channel[i] = m_oscCh[i];
        m_channel[i]->m_channel = i;
        m_channel[i]->m_ePin[0] = m_pin[i];
        m_channel[i]->m_ePin[1] = m_pin[2]; // Ref Pin
    }

    setAdvanc( false ); // Create Widgets
    m_filter = 0.0;
}
Oscope::~Oscope() {}

QList<propGroup_t> Oscope::propGroups()
{
    propGroup_t mainGroup { tr("Main") };
    mainGroup.propList.append( {"Filter", tr("Filter"),"V"} );

    propGroup_t sizeGroup { tr("Screen") };
    sizeGroup.propList.append( {"", tr("Baisc Mode:"),""} );
    sizeGroup.propList.append( {"Basic_X", tr("Size X"),"Pixels"} );
    sizeGroup.propList.append( {"Basic_Y", tr("Size Y"),"Pixels"} );
    sizeGroup.propList.append( {"", tr("Expanded Mode:"),""} );
    sizeGroup.propList.append( {"Expand_X", tr("Size X"),"Pixels"} );
    sizeGroup.propList.append( {"Expand_Y", tr("Size Y"),"Pixels"} );

    propGroup_t logGroup { tr("One Shot") };
    logGroup.propList.append( {"Data_Log", tr("Active (pauses simulation at trigger)"),""} );
    logGroup.propList.append( {"Log_us", tr("Log Size"),"µs"} );
    logGroup.propList.append( {"", tr("Conditions:"),""} );
    logGroup.propList.append( {"CH1_Cond", tr("Channel 1"),"enum"} );
    logGroup.propList.append( {"CH2_Cond", tr("Channel 2"),"enum"} );
    logGroup.propList.append( {"REF_Cond", tr("Ref. Pin"),"enum"} );

    return {mainGroup, sizeGroup, logGroup};
}

void Oscope::updateStep()
{
    uint64_t period = 0;
    uint64_t hTick  = m_dataPlotW->m_hTick;
    uint64_t timeFrame = hTick*10;
    uint64_t offset = 0;
    uint64_t orig;
    uint64_t origAbs;
    uint64_t simTime;

    if( m_trigger < 2  ) period = m_oscCh[m_trigger]->m_period; // We want a trigger

    if( period > 10 ) // We have a Trigger
    {
        uint64_t risEdge = m_oscCh[m_trigger]->m_risEdge;

        uint64_t nCycles = timeFrame*2/period;
        if( timeFrame%period ) nCycles++;
        if( nCycles%2 )        nCycles++;

        uint64_t delta = nCycles*period/2-timeFrame/2;
        if( delta > risEdge ) delta = risEdge;
        simTime = risEdge-delta;
    }
    else simTime = Simulator::self()->circTime(); // free running

    m_dataPlotW->m_display->setXFrame( timeFrame );

    if( simTime>timeFrame ) orig = simTime-timeFrame;
    else
    {
        orig = 1;
        offset = timeFrame-simTime;
    }
    if( simTime>timeFrame*2 ) origAbs = simTime-timeFrame*2;
    else                      origAbs = 1;

    for( int i=0; i<2; i++ )
    {
        if( m_pin[i]->isConnected() )
        {
            m_channel[i]->fetchData( orig, origAbs, offset );
            m_channel[i]->updateStep();
        }
    }
    m_dataPlotW->m_display->update();
}

void Oscope::setFilter( double filter )
{
    m_filter = filter;

    for(int i=0; i<2; i++ ) m_channel[i]->setFilter( filter );
}

#include "moc_oscope.cpp"
