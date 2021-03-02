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

#include "datachannel.h"
#include "plotdisplay.h"
#include "simulator.h"

DataChannel::DataChannel( PlotBase* plotBase, QString id )
           : eElement( id )
{
    m_plotBase = plotBase;
    m_ePin.resize( 2 );
    m_ePin[1] = NULL;
}

DataChannel::~DataChannel()
{
    eNode* enode =  m_ePin[0]->getEnode();
    if( enode ) enode->remFromPlotterList( this );
}

void DataChannel::stamp()                     // Called at Simulation Start
{
    eNode* enode =  m_ePin[0]->getEnode();
    if( enode )
    {
        enode->voltChangedCallback( this );
        enode->addToPlotterList( this );
    }

    if( !m_ePin[1] ) return;
    enode =  m_ePin[1]->getEnode();
    if( enode ) enode->voltChangedCallback( this );
}

void DataChannel::fetchData( uint64_t orig, uint64_t origAbs , uint64_t offset )
{
    voltChanged();

    int pos = m_bufferCounter;
    uint64_t time;
    double val;

    uint64_t timeStep = m_plotBase->timeDiv()/50;
    uint64_t lastTime = m_time.at(pos)+timeStep;
    uint64_t maxTime = 0;
    uint64_t minTime = 0;
    double   maxVal  = -1e12;
    double   minVal  = 1e12;
    bool subSample = false;

    m_points->clear();

    for( int i=0; i<m_buffer.size(); ++i ) // Read Backwards
    {
        time = m_time.at(pos);
        val  = m_buffer[pos];

        if( lastTime-time < timeStep ) // SubSample
        {
            subSample = true;
            if     ( val > maxVal ) { maxVal = val; maxTime = time; }
            else if( val < minVal ) { minVal = val; minTime = time; }
        }
        else {
            if( subSample )
            {
                if( maxTime > minTime )
                {
                    m_points->prepend( QPointF( (int64_t)(maxTime-orig+offset), maxVal ));
                    if( minTime > 0 ) m_points->prepend( QPointF( (int64_t)(minTime-orig+offset), minVal ));
                }
                else if ( minTime > maxTime )
                {
                    m_points->prepend( QPointF( (int64_t)(minTime-orig+offset), minVal ));
                    if( maxTime > 0 ) m_points->prepend( QPointF( (int64_t)(maxTime-orig+offset), maxVal ));
                }
                subSample = false;
                maxVal = -1e12;
                minVal = 1e12;
                maxTime = 0;
                minTime = 0;
            }
            lastTime = time;
            m_points->prepend( QPointF( (int64_t)(time-orig+offset), val ));
        }
        if( time < origAbs ) break; // End of data
        if( --pos < 0 ) pos += m_buffer.size();
    }
    m_plotBase->display()->setData( m_channel, m_points );
}

