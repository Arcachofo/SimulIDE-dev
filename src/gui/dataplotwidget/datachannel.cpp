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
#include "plotbase.h"
#include "simulator.h"

DataChannel::DataChannel( PlotBase* plotBase, QString id )
           : eElement( id  )
           , Updatable()
{
    m_plotBase = plotBase;
    m_ePin.resize( 2 );
    m_ePin[1] = NULL;
    m_chTunnel = "";
    m_trigIndex = 0;
    m_pauseOnCond = false;
}

DataChannel::~DataChannel()
{
}

void DataChannel::stamp()    // Called at Simulation Start
{
    bool connected = false;
    eNode* enode =  m_ePin[0]->getEnode();
    if( enode )
    {
        enode->voltChangedCallback( this );
        connected = true;
    }
    m_plotBase->display()->connectChannel( m_channel, connected );

    if( !m_ePin[1] ) return;
    m_ePin[1]->changeCallBack( this );

    m_bufferCounter = 0;
    m_trigIndex = 0;
}

