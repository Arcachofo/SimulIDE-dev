/***************************************************************************
 *   Copyright (C) 2020 by santiago González                               *
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

#include "oscopechannel.h"
#include "plotbase.h"
#include "simulator.h"

OscopeChannel::OscopeChannel( QString id )
             : DataChannel( id )
{
    m_filter = 0.1;

    m_trigger = 2;

    m_hTick = 1;
    m_vTick = 1;
    m_Vpos  = 0;

    m_points = &m_pointsA;
}
OscopeChannel::~OscopeChannel()  { }

void OscopeChannel::initialize()
{
    //qDebug() <<"OscopeChannel::resetState";
    m_reading  = true;
    m_rising   = false;
    m_falling  = false;
    m_runEvent = false;
    m_chCondFlag = false;

    m_period = 0;
    m_risEdge = 0;
    m_nCycles = 0;
    m_totalP = 0;
    m_numMax = 0;
    m_lastMax = 0;
    m_ampli = 0;
    m_maxVal =-1e12;
    m_minVal = 1e12;
    m_dispMax = 5;
    m_dispMin =-5;

    m_lastValue = 0;
    m_bufferCounter = 0;

    m_hTick = 1e9;
    m_vTick = 1;
    m_Vpos = 0;
    m_freq = 0;

    m_stepsPerS = 1e12;

    m_pointsA.clear();
    m_pointsB.clear();

    //DataChannel::updateStep();
    m_dataPlotW->m_data1Label[m_channel]->setText( "---" );
    m_dataPlotW->m_data2Label[m_channel]->setText( "---" );
    m_dataPlotW->m_display->update();
}

void OscopeChannel::runEvent()
{
    if( !m_runEvent ) return;
    voltChanged();
    Simulator::self()->addEvent( m_hTick/20, this );
}

void OscopeChannel::updateStep()
{
    if( m_connected )
    {
        uint64_t simTime = Simulator::self()->circTime();

        if( !m_reading ) // There is data set ready to display
        {
            m_reading = true;
            m_dataPlotW->m_display->setData( m_channel, m_points );

            if( m_points == &m_pointsA ) m_points = &m_pointsB;
            else                         m_points = &m_pointsA;

            m_dataPlotW->m_display->update();
        }

        if( m_numMax > 1 )  // Got enought maximums to calculate Freq
        {
            m_freq = 1e12/((double)m_totalP/(double)(m_numMax-1));
            m_totalP  = 0;
            m_numMax  = 0;
        }

        if( m_period > 10 )  // We have a wave
        {
            if( m_numMax > 1 )
            {
                uint64_t stepsPF = Simulator::self()->circuitRate()*1e6;
                uint64_t lost = m_period*2;
                if( lost < stepsPF ) lost = stepsPF*2;

                if( simTime-m_lastMax > lost ) // Wave lost
                {
                    m_freq    = 0;
                    m_period  = 0;
                    m_risEdge = 0;
                    m_nCycles = 0;
                    m_totalP  = 0;
                    m_numMax  = 0;
                    m_lastMax = 0;
                    m_ampli   = 0;
                    //m_dispMax = 5;
                    //m_dispMin =-5;
                    m_maxVal  = -1e12;
                    m_minVal  = 1e12;
                }
            }
        }
        else if( !m_runEvent ) // No wave, then read voltages in runEvent
        {
            if( !m_dataPlotW->m_paOnCond )
            {
                m_runEvent = true;
                Simulator::self()->addEvent( m_hTick/20, this );
            }
            //voltChanged();
        }
        if( m_ampli > 1e-6 ) m_dataPlotW->m_display->setMaxMin( m_channel, m_dispMax, m_dispMin );

        double freq = m_freq;

        int Fdecs = 2;
        QString unit = "  Hz";

        if( freq > 999 )
        {
            freq /= 1e3; unit = " KHz";
            if( freq > 999 ) { freq /= 1e3; unit = " MHz"; }
        }
        if     ( freq < 10 )  Fdecs = 4;
        else if( freq < 100 ) Fdecs = 3;
        m_dataPlotW->m_data1Label[m_channel]->setText( " "+QString::number( freq, 'f', Fdecs )+unit );

        unit = " V";
        double val = m_ampli;

        if( val < 1 )
        {
            unit = " mV";
            val *= 1e3;
        }
        int Vdecimals = 0;
        if     ( val < 10 )  Vdecimals = 2;
        else if( val < 100 ) Vdecimals = 1;
        m_dataPlotW->m_data2Label[m_channel]->setText( "Amp "+QString::number( val,'f', Vdecimals )+unit );

        DataChannel::updateStep();
    }
}

void OscopeChannel::voltChanged()
{
    uint64_t simTime = Simulator::self()->circTime();

    double data = m_ePin[0]->getVolt() + m_ePin[1]->getVolt();

    if( data > m_maxVal ) m_maxVal = data;
    if( data < m_minVal ) m_minVal = data;

    if( ++m_bufferCounter >= m_buffer.size() ) m_bufferCounter = 0;
    m_buffer[m_bufferCounter] = data;
    uint64_t time = simTime;
    m_time[m_bufferCounter] = time;

    double delta = data-m_lastValue;

    if( delta > 0 )               // Rising
    {
        if( delta > m_filter )
        {
            if( m_falling && !m_rising )     // Min To Rising
            {
                if( m_numMax > 0 ) m_totalP += simTime-m_lastMax;
                m_lastMax = simTime;

                m_numMax++;
                m_nCycles++;
                m_falling = false;

                if( m_dataPlotW->m_paOnCond )
                {
                    if( (m_chCond == Rising) || (m_chCond == High) ) // Pause on Rising or High
                    {
                        m_chCondFlag = true;
                        m_dataPlotW->m_plotB->pauseOnCond();
                        if( m_chCond == Rising ) m_chCondFlag = false;
                    }
                    else if( m_chCond == Low ) m_chCondFlag = false;
                }
            }
            else if( m_dataPlotW->m_paOnCond )
            {
                if( m_chCond == Rising ) m_chCondFlag = false;
            }
            m_rising = true;
            m_lastValue = data;
        }
        if( m_nCycles > 1 )     // Wait for a full wave
        {
            m_ampli = m_maxVal-m_minVal;
            double mid = m_minVal + m_ampli/2;

            if( data >= mid )            // Rising edge
            {
                m_dispMax = m_maxVal;
                m_dispMin = m_minVal;
                m_maxVal  = -1e12;
                m_minVal  = 1e12;
                m_nCycles--;

                if( m_risEdge > 0 ) m_period = simTime-m_risEdge; // period = this_edge_time - last_edge_time
                m_risEdge = simTime;

                if( m_period > 10 )
                {
                    m_runEvent = false;
                    if( m_dataPlotW->m_auto == m_channel )
                    {
                        m_Vpos = 0;
                        m_vTick = m_ampli/10;
                        if     ( m_vTick > 1000 )  m_vTick = 1000;
                        else if( m_vTick < 0.001 ) m_vTick = 0.001;

                        m_hTick = abs( (double)m_period/5 );
                        if( m_hTick < 1 ) m_hTick = 1;
                    }
                }
            }
        }
    }
    else if( delta < -m_filter )         // Falling
    {
        if( m_rising && !m_falling )    // Max Found
        {
            m_rising = false;

            if( m_dataPlotW->m_paOnCond )
            {
                if( (m_chCond == Falling) || (m_chCond == Low) ) // Pause on Falling or Low
                {
                    m_chCondFlag = true;
                    m_dataPlotW->m_plotB->pauseOnCond();
                    if( m_chCond == Falling ) m_chCondFlag = false;
                }
                else if( m_chCond == High ) m_chCondFlag = false;
            }
        }
        else if( m_dataPlotW->m_paOnCond )
        {
            if( m_chCond == Falling ) m_chCondFlag = false;
        }
        m_falling = true;
        m_lastValue = data;
    }
    if( m_dataPlotW->m_paOnCond ) return;
    if( m_reading )
    {
        if( m_trigger < 2  ) // We want wave
        {
            if( m_trigger == m_channel ) // We drive trigger
            {
                if( m_period > 10 )  // We have wave
                    m_dataPlotW->m_plotB->fetchData( m_channel, m_risEdge );
                else             //  We don't have a wave, trigger just now
                    m_dataPlotW->m_plotB->fetchData( m_channel, time );
            }
        }
        else            // Free running
        {
            fetchData( m_channel, time );
        }
    }
}

void OscopeChannel::fetchData( int ch, uint64_t edge )
{
    //if( m_trigger && ch != m_channel ) return; // we have our own trigger
    if( !m_reading ) return;

    uint64_t period = m_period;
    if( m_period <= 10 ) period = m_hTick/20;
    if( m_dataPlotW->m_paOnCond ) period = m_hTick/20;// Pause on Condition

    uint64_t timeFrame = m_hTick*10;
    uint64_t nCycles = timeFrame/period;
    if( timeFrame%period ) nCycles++;
    if( nCycles%2 ) nCycles++;

    uint64_t orig = edge-nCycles*period/2-timeFrame/2;
    uint64_t origAbs = edge-(nCycles+1)*period;

    if( m_dataPlotW->m_paOnCond ) // Pause on Condition
    {
        uint64_t size = edge-origAbs;
        if( size < m_dataSize )
        {
            if( edge < m_dataSize) origAbs = 0;
            else                   origAbs = edge - m_dataSize;
        }
    }
    int pos = m_bufferCounter;
    uint64_t timeAbs;
    int64_t  time;
    m_points->clear();

    m_points->prepend( QPointF( edge-orig, m_buffer[pos] )); // First poit at current time

    for( int i=0; i<m_buffer.size(); ++i ) // Read Backwards
    {
        timeAbs = m_time.at(pos);
        time = timeAbs-orig;
        m_points->prepend( QPointF( time, m_buffer[pos] ));
        if( timeAbs < origAbs ) break; // End of data

        pos--;
        if( pos < 0 ) pos += m_buffer.size();
    }
    //m_dataPlotW->m_display->setXFrame( m_channel, timeFrame );

    m_reading = false;
}
