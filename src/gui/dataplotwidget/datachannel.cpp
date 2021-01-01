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
#include "simulator.h"

double DataChannel::m_dataSize = 0;

DataChannel::DataChannel( QString id )
           : eElement( id+"-eElement" )
{
    m_ePin.resize( 2 );
    m_connected = false;
    m_chCond = None;
}

DataChannel::~DataChannel()
{
}

void DataChannel::stamp()                     // Called at Simulation Start
{
    eNode* enode =  m_ePin[0]->getEnode();
    if( enode ) enode->voltChangedCallback( this );

    enode =  m_ePin[1]->getEnode();
    if( enode ) enode->voltChangedCallback( this );
}

void DataChannel::updateStep()
{
    if( m_dataPlotW->m_auto == m_channel )
    {
        m_dataPlotW->setHTick( m_hTick );
        m_dataPlotW->setVTick( m_channel, (m_dispMax-m_dispMin)/10 );
        m_dataPlotW->setVPos( m_channel, 0 );
        m_dataPlotW->setHPos( m_channel, 0 );
    }
    else
    {
        m_hTick = m_dataPlotW->m_hTick;
        //m_dataPlotW->setVPos( m_channel, m_dataPlotW->m_vPos[m_channel] );
        //m_vTick = m_dataPlotW->m_vTick[m_channel];
        //m_Vpos  = m_dataPlotW->m_Vpos[m_channel];
    }
}

