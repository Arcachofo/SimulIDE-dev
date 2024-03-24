/***************************************************************************
 *   Copyright (C) 2020 by Santiago González                               *
 *                                                                         *
 ***( see copyright.txt file at root folder )*******************************/

#include "oscopechannel.h"
#include "oscope.h"
#include "plotdisplay.h"
#include "datawidget.h"
#include "simulator.h"
#include "e-pin.h"
#include "utils.h"

OscopeChannel::OscopeChannel( Oscope* oscope, QString id )
             : DataChannel( oscope, id )
{
    m_oscope = oscope;
    m_filter = 0.0;
}
OscopeChannel::~OscopeChannel()  { }

void OscopeChannel::initialize()
{
    m_rising   = false;
    m_falling  = false;

    m_simTime = -1;
    m_period  = 0;
    m_risEdge = 0;
    m_nCycles = 0;
    m_totalP  = 0;
    m_numMax  = 0;
    m_lastMax = 0;
    m_ampli   = 0;
    m_maxVal = -1e12;
    m_minVal = 1e12;
    m_midVal = 0;
    m_dispMax = 5;
    m_dispMin =-5;

    m_lastValue = 0;
    m_bufferCounter = 0;

    m_freq = 0;

    m_buffer.fill(0);
    m_time.fill(0);

    updateStep();
}

void OscopeChannel::updateValues()
{
    /// m_dataTime = 0;

    if( !Simulator::self()->isRunning() ) m_freq = 0;
    double val = m_freq*1e12;
    int decs = 0;
    QString unit = " ";
    if( val >= 1 ) {valToUnit( val, unit, decs );}
    QString f = " "+QString::number( val, 'f', decs )+unit+"Hz";

    m_oscope->dataW()->setData( m_channel, f );
}

void OscopeChannel::updateStep()
{
    uint64_t simTime = Simulator::self()->circTime();

    if( m_period > 10 )  // We have a wave
    {
        if( m_numMax > 1 )  // Got enought maximums to calculate Freq
        {
            double avgPeriod = (double)m_totalP/(double)(m_numMax-1);
            m_freq = 1e12/avgPeriod;

            m_totalP  = 0;
            m_numMax  = 0;
        }
        if( m_freq < 20 )voltChanged();
    }else{
        voltChanged();
        m_freq = 0;
        m_maxVal  =-1e12;
        m_minVal  = 1e12;
    }
    if( m_oscope->autoSC() == m_channel )
    {
        m_oscope->setTimePos( m_channel, 0 );
        if( m_period > 10 )
        {
            m_oscope->setTimeDiv( m_period/5 );
            m_oscope->setVoltDiv( m_channel, m_ampli/10 );
            m_oscope->setVoltPos( m_channel, -m_midVal );
    }   }
    else{
        m_dispMax = m_oscope->voltDiv( m_channel )*10;
        m_dispMin = 0;
    }
    updateValues();

    if( m_period > 10 )  // Do we still have a wave?
    {
        uint64_t psPF  = Simulator::self()->psPerFrame();
        uint64_t lost = m_period*2;
        if( lost < psPF*2 ) lost = psPF*2;

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
}   }   }

void OscopeChannel::voltChanged()
{
    if( !m_connected ) return;

    /*if( ++m_dataTime > m_buffer.size() ) // Measure time taken to fill buffer
    {
        m_dataTime = 0;
        if( m_subRate > 0 )
            m_subSample = (simTime-m_dataZero)/1e3*m_subRate/m_buffer.size();
        m_dataZero = simTime;
    }
    if( m_subSample > 0 ) // Buffer should hold enought time to display
    {                     // Skip samples acordingly
        if( ++m_subStep > m_subSample ) m_subStep = 0;
        else return;
    }*/
    double data = m_ePin[0]->getVoltage()-m_ePin[1]->getVoltage();
    double delta = data-m_lastValue;
    if( delta == 0 ) return;

    if( data > m_maxVal ) m_maxVal = data;
    if( data < m_minVal ) m_minVal = data;

    uint64_t simTime = Simulator::self()->circTime();

    if( m_simTime != simTime )
    {
        m_simTime = simTime;
        if( ++m_bufferCounter >= m_buffer.size() ) m_bufferCounter = 0;
    }
    m_buffer[m_bufferCounter] = data;
    m_time[m_bufferCounter] = simTime;

    if( delta > m_filter )               // Rising
    {
        //if( delta > m_filter )
        {
            if( m_falling && !m_rising )     // Min To Rising
            {
                if( m_numMax > 0 ) m_totalP += simTime-m_lastMax;
                m_lastMax = simTime;

                m_numMax++;
                m_nCycles++;
                m_falling = false;
            }
            m_rising = true;
            m_lastValue = data;
        }
        if( m_nCycles > 2 )     // Trigger lost? Reset Max and Min values
        {
            m_nCycles = 0;
            m_maxVal  =-1e12;
            m_minVal  = 1e12;
        }
        else if( m_nCycles > 1 )     // Wait for a full wave
        {
            m_ampli  = m_maxVal-m_minVal;
            m_midVal = m_minVal + m_ampli/2;

            if( data >= m_midVal )            // Rising edge
            {
                if( m_numMax > 1 )
                {
                    m_dispMax = m_maxVal;
                    m_dispMin = m_minVal;
                    m_maxVal  =-1e12;
                    m_minVal  = 1e12;
                }
                m_nCycles--;

                if( m_risEdge > 0 ) m_period = simTime-m_risEdge; // period = this_edge_time - last_edge_time
                m_risEdge = simTime;
                //if( m_trigger ) m_oscope->triggerEvent();
    }   }   }

    else if( delta < -m_filter )         // Falling
    {
        if( m_rising && !m_falling )    // Max Found
        {
            m_rising = false;
        }
        m_falling = true;
        m_lastValue = data;
}   }

void OscopeChannel::setFilter( double f )
{
    m_risEdge = 0;
    m_nCycles = 0;
    m_totalP  = 0;
    m_numMax  = 0;

    m_filter = f;
}
