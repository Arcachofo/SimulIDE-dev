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

//#include <QDebug>
//#include <math.h>   // fabs(x,y)

#include "e-pn.h"
#include "simulator.h"
#include "e-pin.h"

ePN::ePN( QString id )
   : eResistor(id )
{
    m_threshold = 0.7;
}
ePN::~ePN(){}

void ePN::stamp()
{
    /*if( m_ePin[0]->isConnected() )
    {
        eNode* node = m_ePin[0]->getEnode();
        //node->addToNoLinList(this);
        node->setSwitched( true );
    }
    if( m_ePin[1]->isConnected() )
    {
        eNode* node = m_ePin[1]->getEnode();
        //node->addToNoLinList(this);
        node->setSwitched( true );
    }*/
    eResistor::setAdmit( cero_doub );
    eResistor::stamp();
}

void ePN::initialize()
{
    eResistor::setRes( 0.6 );
    m_accuracy = Simulator::self()->NLaccuracy();
    m_voltPN  = 0;
    m_deltaV  = 0;
    m_current = 0;
}

double ePN::step( double volt )
{
    //m_converged = false;

    m_voltPN = volt;

    double deltaV = m_threshold;

    if( m_voltPN < m_threshold )
    {
        eResistor::setAdmit( cero_doub );
        m_ePin[0]->stampCurrent( 0 );
        m_ePin[1]->stampCurrent( 0 );
        m_deltaV = m_voltPN;
        m_current = 0;
        return 0;
    }

    //qDebug() <<"ePN::setVChanged,  deltaR: "<< deltaR << "  deltaV" << deltaV << "m_voltPN" << m_voltPN ;
    m_current = (m_voltPN-m_threshold)*m_admit;

    /*if( fabs(deltaV-m_deltaV) < m_accuracy/10 )
    {
        m_converged = true;
        return;
    }*/

    m_deltaV = deltaV;

    double current = deltaV*m_admit;

    eResistor::stampCurrent( current );

    return m_current;
}

void ePN::setThreshold( double threshold )
{
    m_threshold = threshold;
}
