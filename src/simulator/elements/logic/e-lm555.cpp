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

#include "e-lm555.h"
#include "e-source.h"
#include "simulator.h"

eLm555::eLm555( QString id )
      : eLogicDevice( id )
{
    setNumEpins(8);
    initialize();
}
eLm555::~eLm555()
{ 
    delete m_output;
    delete m_cv;
    delete m_dis;
}

void eLm555::stamp()
{
    for( int i=0; i<8; ++i )
    {
        if( i == 2 ) continue; // Output
        if( i == 6 ) continue; // Discharge
        
        if( m_ePin[i]->isConnected() ) m_ePin[i]->getEnode()->addToNoLinList(this);
    }
}

void eLm555::initialize()
{
    m_outState = false;

    m_voltLast = 0;
    m_voltNegLast = 0;
    m_voltHightLast = 0;
    m_disImpLast = cero_doub;
}

void eLm555::voltChanged()
{
    bool changed = false;
    double voltPos = m_ePin[7]->getVolt();
    m_voltNeg = m_ePin[0]->getVolt();
    m_volt    = voltPos - m_voltNeg;
    
    double reftTh = m_ePin[4]->getVolt();
    double reftTr = reftTh/2;
    
    if( m_voltLast != m_volt ) changed = true;

    double voltTr = m_ePin[1]->getVolt();
    double voltTh = m_ePin[5]->getVolt();
    
    double voltRst = m_ePin[3]->getVolt();
    
    bool reset = ( voltRst < (m_voltNeg+0.7) );
    bool th    = ( voltTh > reftTh );
    bool tr    = ( reftTr > voltTr );
    
    bool outState = m_outState;
    
    if     ( reset )     outState = false;
    else if( tr )        outState =  true;
    else if( !tr && th ) outState =  false;

    //qDebug() << "eLm555::setVChanged" << outState<<"th"<<th<<"tr"<<tr;
    if( outState != m_outState )
    {
        m_outState = outState;
        m_voltHight = m_voltNeg;

        if( outState ) 
        {
            m_voltHight = voltPos - 1.7;
            if( m_voltHight < m_voltNeg ) m_voltHight = m_voltNeg;
            m_disImp = high_imp;
        }
        else m_disImp = 1;

        changed = true;

        //qDebug() << "eLm555::setVChanged" << outState<<reset<<th<<tr;
    }
    if( changed ) Simulator::self()->addEvent( m_propDelay, this );
}

void eLm555::runEvent()
{
    if( m_voltLast != m_volt )
    {
        m_cv->setVoltHigh( m_volt*2/3 );
        m_cv->stampOutput();
        m_voltLast = m_volt;
    }
    if( m_voltNegLast != m_voltNeg )
    {
        m_dis->setVoltHigh( m_voltNeg );
        m_dis->stampOutput();
        m_voltNegLast = m_voltNeg;
    }
    if( m_voltHightLast != m_voltHight )
    {
        m_output->setVoltHigh( m_voltHight );
        m_output->stampOutput();
        m_voltHightLast = m_voltHight;
    }
    if( m_disImpLast != m_disImp )
    {
        m_dis->setImp( m_disImp );
        m_disImpLast = m_disImp;
    }
}
