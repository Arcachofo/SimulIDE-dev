/***************************************************************************
 *   Copyright (C) 2010 by santiago González                               *
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

#include "gate.h"
#include "connector.h"
#include "circuit.h"
#include "iopin.h"

Gate::Gate( QObject* parent, QString type, QString id, int inputs )
    : LogicComponent( parent, type, id )
{
    m_width = 2;

    setNumOuts( 1, "", 0, false );
    setNumInps( inputs );  // Create Input Pins

    m_rndPD = true; // Randomize Propagation Delay
}
Gate::~Gate(){}

void Gate::stamp()
{
    LogicComponent::stamp();
    for( uint i=0; i<m_inPin.size(); ++i ) m_inPin[i]->changeCallBack( this );
    m_out = false;
}

void Gate::voltChanged()
{
    if( m_tristate ) LogicComponent::updateOutEnabled();

    int inputs = 0;

    for( uint i=0; i<m_inPin.size(); ++i )
    {
        bool state = m_inPin[i]->getInpState();
        if( state ) inputs++;
    }
    bool out = calcOutput( inputs ); // In each gate type

    m_nextOutVal = out? 1:0;
    if( m_out == out && !m_tristate ) return;
    m_out = out;

    sheduleOutPuts( this );
}

bool Gate::calcOutput( int inputs )
{
    return ((uint)inputs == m_inPin.size()); // Default for: Buffer, Inverter, And, Nand
}

void Gate::setNumInps( int inputs )
{
    if( inputs < 1 ) return;
    IoComponent::setNumInps( inputs, "" );
    m_outPin[0]->setY( 0 );
}
