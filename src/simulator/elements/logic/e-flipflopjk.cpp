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

#include <QDebug>

#include "e-flipflopjk.h"
#include "simulator.h"
#include "circuit.h"

eFlipFlopJK::eFlipFlopJK( QString id )
           : eLogicDevice( id )
{
}
eFlipFlopJK::~eFlipFlopJK() {}

void eFlipFlopJK::stamp()
{
    eNode* enode = m_input[2]->getEpin(0)->getEnode();         // Set pin
    if( enode ) enode->voltChangedCallback( this );
    
    enode = m_input[3]->getEpin(0)->getEnode();              // Reset pin
    if( enode ) enode->voltChangedCallback( this );

    if( m_etrigger != Trig_Clk )
    {
        for( uint i=0; i<2; i++ )
        {
            eNode* enode = m_input[i]->getEpin(0)->getEnode();
            if( enode ) enode->voltChangedCallback( this );
        }
    }
    eLogicDevice::stamp();
}

void eFlipFlopJK::voltChanged()
{
    bool clkAllow = (eLogicDevice::getClockState() == Clock_Allow); // Get Clk to don't miss any clock changes

    if( eLogicDevice::getInputState( 2 )==true )     // Master Set
    {
        m_Q0 = true;         // Q
        m_Q1 = false;        // Q'
    }
    else if( eLogicDevice::getInputState( 3 )==true ) // Master Reset
    {
        m_Q0 = false;         // Q
        m_Q1 = true;          // Q'
    }
    else if( clkAllow )                              // Allow operation
    {
        bool J = eLogicDevice::getInputState( 0 );
        bool K = eLogicDevice::getInputState( 1 );
        bool Q = m_output[0]->out();
        
        bool state = (J && !Q) || (!K && Q) ;

        m_Q0 = state ;       // Q
        m_Q1 = !state;       // Q'
    }
    Simulator::self()->addEvent( m_propDelay, this );
}

void eFlipFlopJK::runEvent()
{
    m_output[0]->setTimedOut( m_Q0 );      // Q
    m_output[1]->setTimedOut( m_Q1 );      // Q'
}

void eFlipFlopJK::setSrInv( bool inv )
{
    m_srInv = inv;
    m_input[2]->setInverted( inv );                   // Set
    m_input[3]->setInverted( inv );                   // Reset
    
    Circuit::self()->update();
}
