/***************************************************************************
 *   Copyright (C) 2020 by Santiago González                               *
 *                                                                         *
 ***( see copyright.txt file at root folder )*******************************/

#include "datachannel.h"
#include "plotdisplay.h"
#include "plotbase.h"
#include "simulator.h"
#include "e-node.h"
#include "pin.h"

DataChannel::DataChannel( PlotBase* plotBase, QString id )
           : eElement( id  )
           , Updatable()
{
    m_plotBase = plotBase;
    m_ePin.resize( 2 );
    m_pin = NULL;
    m_ePin[1] = NULL;
    m_chTunnel = "";
    m_trigIndex = 0;
    m_pauseOnCond = false;
}
DataChannel::~DataChannel(){}

void DataChannel::stamp()    // Called at Simulation Start
{
    m_bufferCounter = 0;
    m_trigIndex = 0;
    bool connected = false;

    eNode* enode =  m_ePin[0]->getEnode();
    if( enode ){
        enode->voltChangedCallback( this );
        connected = true;
    }
    m_plotBase->display()->connectChannel( m_channel, connected );

    if( !m_ePin[1] ) return;
    m_ePin[1]->changeCallBack( this );
}

bool DataChannel::isBus()
{
    if( m_pin ) return m_pin->isBus();
    return false;
}

