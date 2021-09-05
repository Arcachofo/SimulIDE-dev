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
    if( m_ePin[0]->isConnected() )
    {
        eNode* node = m_ePin[0]->getEnode();
        node->addToNoLinList( this );
        node->setSwitched( true );
    }
    if( m_ePin[1]->isConnected() )
    {
        eNode* node = m_ePin[1]->getEnode();
        node->addToNoLinList( this );
        node->setSwitched( true );
    }
    eResistor::stamp();
}

void eDiode::initialize()
{
    m_resist = high_imp;
    m_admit = 0;
    m_voltPN  = 0;
    m_deltaV  = 0;
    m_current = 0;
    m_lastCurrent   = 0;
}

void eDiode::voltChanged()
{
    m_voltPN = m_ePin[0]->getVolt()-m_ePin[1]->getVolt();

    double deltaR = m_imped;
    double deltaV = m_threshold;

    double delta = m_threshold-m_voltPN;
    if( delta > 1e-12 )   // Not conducing
    {
        if( (m_zenerV > 0) && (m_voltPN <-m_zenerV) )
            deltaV =-m_zenerV;
        else
        {
            deltaV = m_voltPN;
            deltaR = high_imp;
    }   }
    if( deltaR != m_resist )
    {
        m_resist = deltaR;
        if( deltaR == high_imp ) eResistor::setAdmit( 0 );
        else                     eResistor::setAdmit( 1/m_resist );
    }
    m_deltaV = deltaV;

    double current = deltaV/m_resist;
    if( deltaR == high_imp ) current = 0;
    if( current == m_lastCurrent )return;
    m_lastCurrent = current;

    m_ePin[0]->stampCurrent( current );
    m_ePin[1]->stampCurrent(-current );
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
    
    if( m_resist == high_imp ) return;

    if( m_ePin[0]->isConnected() && m_ePin[1]->isConnected() )
    {
        double volt = m_voltPN - m_deltaV;
        if( volt>0 ) m_current = volt/m_resist;
}   }
