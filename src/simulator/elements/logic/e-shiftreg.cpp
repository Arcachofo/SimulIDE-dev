/***************************************************************************
 *   Copyright (C) 2012 by santiago González                               *
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

#include "e-shiftreg.h"
#include "simulator.h"
#include "e-source.h"

eShiftReg::eShiftReg( QString id )
         : eLogicDevice( id )
{
    // input0: DS    serial data input
    // input1: MR    master reset (active LOW)

    initialize();
}
eShiftReg::~eShiftReg()
{
}

void eShiftReg::initialize()
{
    eLogicDevice::initialize();
}

void eShiftReg::stamp()
{
    eNode* enode = m_input[1]->getEpin(0)->getEnode(); // m_input[1] = Reset pin
    if( enode ) enode->voltChangedCallback( this );

    eLogicDevice::stamp();
}

void eShiftReg::voltChanged()
{
    eLogicDevice::updateOutEnabled();

    bool clkRising = (eLogicDevice::getClockState() == Clock_Rising);// Get Clk to don't miss any clock changes
    bool     reset = eLogicDevice::getInputState( 1 );

    if( reset ) m_nextOutVal = 0;        // Reset shift register
    else if( clkRising )                 // Clock rising edge
    {
        m_nextOutVal <<= 1;

        bool data = eLogicDevice::getInputState( 0 );
        if( data ) m_nextOutVal += 1;
    }
    sheduleOutPuts();
}

void eShiftReg::setResetInv( bool inv )
{
    m_resetInv = inv;
    m_input[1]->setInverted( inv );
}
