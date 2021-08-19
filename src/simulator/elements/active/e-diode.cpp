/***************************************************************************
 *   Copyright (C) 2012 by santiago González                               *
 *   santigoro@gmail.com                                                   *
 *                                                                         *
 *   This program is free software; you can redistribute it and/or modify  *
 *   it under the terms of the GNU General Public License as published by  *
 *   the Free Software Foundation; either version 2 of the License, or     *
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

//#include <QDebug>
//#include <math.h>

#include "e-diode.h"
#include "e-node.h"
#include "simulator.h"

eDiode::eDiode( QString id )
      : eResistor( id )
{
    m_imped = 0.6;
    m_threshold = 0.7;
    m_zenerV = 0;
}
eDiode::~eDiode(){}

void eDiode::stamp()
{
    //qDebug() << "eDiode::stamp"<<QString::fromStdString(m_elmId);
    if( m_ePin[0]->isConnected() )
    {
        eNode* node = m_ePin[0]->getEnode();
        node->addToNoLinList( this );
        ///node->setSwitched( true );
    }
    if( m_ePin[1]->isConnected() )
    {
        eNode* node = m_ePin[1]->getEnode();
        node->addToNoLinList( this );
        ///node->setSwitched( true );
    }
    eResistor::stamp();
}

void eDiode::initialize()
{
    m_resist = off_imp;
    //m_admit = 0;
    m_admit = 1/off_imp; /// changed to non-switched
    m_voltPN  = 0;
    m_deltaV  = 0;
    m_current = 0;
    m_lastC   = 0;
}

void eDiode::voltChanged()
{
    m_voltPN = m_ePin[0]->getVolt()-m_ePin[1]->getVolt();

    double deltaR = m_imped;
    double deltaV = m_threshold;

    if( (m_threshold-m_voltPN) > 1e-6 )   // Not conducing
    {
        if( (m_zenerV > 0)&&(m_voltPN <-m_zenerV) )
            deltaV =-m_zenerV;
        else                        
        {
            deltaV = m_voltPN;
            deltaR = off_imp;
        }
    }
    //qDebug() <<"eDiode::setVChanged,  deltaR: "<< deltaR << "  deltaV" << deltaV << "m_voltPN" << m_voltPN<<m_threshold<<m_imped ;
    //qDebug() <<"eDiode::setVChanged : "<< (m_voltPN==m_threshold);

    if( deltaR != m_resist )
    {
        m_resist = deltaR;
        ///if( deltaR == off_imp ) eResistor::setAdmit( 0 );
        ///else
            eResistor::setAdmit( 1/m_resist );
        //m_converged = false;
    }
    //if( fabs(deltaV - m_deltaV) > 1e-18 ) 
    {
        m_deltaV = deltaV;

        double current = deltaV/m_resist;
        //qDebug() <<"eDiode::setVChanged current: "<< current;
        ///if( deltaR == off_imp ) current = 0;
        if( current == m_lastC )
        {
            //m_converged = true;
            return;
        }
        m_lastC = current;

        m_ePin[0]->stampCurrent( current );
        m_ePin[1]->stampCurrent(-current );
        //Simulator::self()->addEvent( 0, 0l );
    }
}

void eDiode::setThreshold( double threshold )
{
    m_threshold = threshold;
}

double eDiode::res()
{
    return m_imped;
}

void eDiode::setRes( double resist )
{
    Simulator::self()->pauseSim();

    if( resist == 0 ) resist = 0.1;
    m_imped = resist;
    voltChanged();

    Simulator::self()->resumeSim();
}

void  eDiode::setZenerV( double zenerV ) 
{ 
    if( zenerV > 0 ) m_zenerV = zenerV; 
    else             m_zenerV = 0;
    setResSafe( m_imped );
}

void eDiode::updateVI()
{
    m_current = 0;
    
    if( m_resist == off_imp ) return;

    if( m_ePin[0]->isConnected() && m_ePin[1]->isConnected() )
    {
        double volt = m_voltPN - m_deltaV;
        if( volt>0 ) m_current = volt/m_resist;
        //qDebug() << "m_voltPN"<<m_voltPN<<"m_deltaV"<<m_deltaV<<"volt"<<volt;
    }
}

