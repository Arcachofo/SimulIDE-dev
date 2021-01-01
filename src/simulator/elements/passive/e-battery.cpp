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

#include "e-battery.h"
#include "simulator.h"



eBattery::eBattery( QString id )
         : eElement( id )
{
    m_volt = 5;
    m_ePin.resize(2);
}
eBattery::~eBattery(){
    //qDebug() << "eBattery::~eBattery deleting" << QString::fromStdString( m_elmId );
}

void eBattery::stamp()
{
    m_ePin[0]->setEnodeComp( m_ePin[1]->getEnode() );
    m_ePin[1]->setEnodeComp( m_ePin[0]->getEnode() );
    m_ePin[0]->stampAdmitance( 1/cero_doub );
    m_ePin[1]->stampAdmitance( 1/cero_doub );
    m_ePin[0]->stampCurrent( m_volt/cero_doub );
    m_ePin[1]->stampCurrent(-m_volt/cero_doub );

    //qDebug() << "eBattery::initialize"<<QString::fromStdString(m_elmId)<<m_volt;
}

void eBattery::initialize()
{
    m_accuracy = Simulator::self()->NLaccuracy();
    m_lastOut = 0;
}

double eBattery::volt()
{ 
    return m_volt;
}

void eBattery::setVolt( double volt )
{
    bool pauseSim = Simulator::self()->isRunning();
    if( pauseSim ) Simulator::self()->pauseSim();

    if( volt < 1e-12 ) volt = 1e-12;
    m_volt = volt;

    if( pauseSim ) Simulator::self()->resumeSim();
}

