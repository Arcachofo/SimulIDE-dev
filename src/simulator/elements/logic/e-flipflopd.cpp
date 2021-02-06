/***************************************************************************
 *   Copyright (C) 2016 by santiago González                               *
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

#include "e-flipflopd.h"
#include "simulator.h"
#include "circuit.h"

eFlipFlopD::eFlipFlopD( QString id )
          : eLogicDevice( id )
{
}
eFlipFlopD::~eFlipFlopD() {}

void eFlipFlopD::stamp()
{
    m_Q0 = 0;
    eNode* enode = m_input[1]->getEpin(0)->getEnode();         // Set pin
    if( enode ) enode->voltChangedCallback( this );
    
    enode = m_input[2]->getEpin(0)->getEnode();              // Reset pin
    if( enode ) enode->voltChangedCallback( this );

    if( m_etrigger != Trig_Clk )
    {
        eNode* enode = m_input[0]->getEpin(0)->getEnode();
        if( enode ) enode->voltChangedCallback( this );
    }
    eLogicDevice::stamp();
}

void eFlipFlopD::voltChanged()
{
    // Get Clk to don't miss any clock changes
    bool clkAllow = (getClockState() == Clock_Allow);

    bool set   = getInputState( 1 );
    bool reset = getInputState( 2 );

    if( set || reset)   m_Q0 = set;
    else if( clkAllow ) m_Q0 = getInputState( 0 );

    m_nextOutVal = m_Q0? 1:2;
    sheduleOutPuts();
}

void eFlipFlopD::setSrInv( bool inv )
{
    m_srInv = inv;
    m_input[1]->setInverted( inv ); // Set
    m_input[2]->setInverted( inv ); // Reset
    
    Circuit::self()->update();
}
