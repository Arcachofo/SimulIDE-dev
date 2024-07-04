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

bool DataChannel::doTest()
{
    int dataSize = m_bufferTest.size();
    bool compare = dataSize > 0;
    if( compare && dataSize != m_bufferCounter ) return false;

    bool ok = true;
    for( int n=0; n<m_bufferCounter; ++n )
    {
        uint64_t time = m_time.at(n);
        double  value = m_buffer.at(n);
        if( compare )
        {
            uint64_t cTime = m_timeTest.at(n);
            double  cValue = m_bufferTest.at(n);
            if( time != cTime || value != cValue ) ok = false;
        }else{
            m_timeTest.append( time );
            m_bufferTest.append( value );
        }
    }
    return ok;
}

QString DataChannel::testData()
{
    QString td;

    for( int n=0; n<m_bufferTest.size(); ++n )
    {
        td += QString::number( m_timeTest.at(n), 16 )+" ";
        td += QString::number( m_bufferTest.at(n) )+"|";
    }
    td.remove( td.size()-1, 1);
    return td;
}

void DataChannel::setTestData( QString td )
{
    if( td.isEmpty() ) return;

    QStringList dataList = td.split("|");
    for( int i=0; i<dataList.size(); ++i )
    {
        QStringList dataPair = dataList.at(i).split(" ");
        m_timeTest.append( dataPair.first().toUInt(0,16) );
        m_bufferTest.append( dataPair.last().toDouble() );
    }
}
