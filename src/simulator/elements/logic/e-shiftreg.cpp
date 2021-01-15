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

eShiftReg::eShiftReg( QString id, int latchClk, int serOut ) 
         : eLogicDevice( id )
{
    // input0: DS    serial data input
    // input1: MR    master reset (active LOW)
    // packagePin15-QH' 
    
    m_latchClock = false;
    m_changed    = false;
    m_reset      = false;
    
    m_latchClockPin = 0l;
    m_serOutPin     = 0l;
    
    if( latchClk > 0 ) createLatchClockPin();
    if( serOut > 0 )   createSerOutPin();

    initialize();
}
eShiftReg::~eShiftReg()
{
    delete m_latchClockPin;
    delete m_serOutPin;
}

void eShiftReg::initialize()
{
    m_shiftReg.reset();
    m_latch.reset();
    m_latchClock = false;
    m_changed    = false;
    m_setSerOut  = false;
    m_resSerOut  = false;
    
    if( m_serOutPin )                              // Set Serial Out Pin
    {
        m_serOutPin->setOut( false );
        m_serOutPin->stampOutput();
    }
    eLogicDevice::initialize();
}

void eShiftReg::stamp()
{
    eNode* enode = m_input[1]->getEpin(0)->getEnode(); // m_input[1] = Reset pin
    if( enode ) enode->voltChangedCallback( this );

    if( m_latchClockPin )
    {
        enode = m_latchClockPin->getEpin(0)->getEnode();
        if( enode ) enode->voltChangedCallback( this );
    }
    eLogicDevice::stamp();
}

void eShiftReg::voltChanged()
{
    eLogicDevice::updateOutEnabled();

    bool clkRising = (eLogicDevice::getClockState() == Clock_Rising);// Get Clk to don't miss any clock changes
    double    volt = m_input[1]->getEpin(0)->getVolt();// Reset pin volt.
    bool     reset = eLogicDevice::getInputState( 1 );// m_input[1] = Reset
    bool  addEvent = false;

    if( reset != m_reset  )
    {
        m_reset = reset;
        if( reset )
        {
            m_shiftReg.reset();                  // Reset shift register  
            if( m_serOutPin ) m_resSerOut = true;// Reset Serial Out Pin
            addEvent = true;
        }
        m_changed = true;                      // Shift Register changed
    }
    else if( clkRising && !reset )                  // Clock rising edge
    {
        // Shift bits 7-1
        for( int i=7; i>0; --i )m_shiftReg[i] = m_shiftReg[i-1];
        
        if( m_serOutPin )  m_setSerOut = true;     // Set Serial Out Pin

        // Read data input pin & put in reg bit0
        volt = m_input[0]->getVolt();      // Reset pin volt.
        m_shiftReg[0] = (volt > m_inputHighV);     // input0: data input

        m_changed = true;                      // Shift Register changed
        addEvent = true;
    }
    
    if( m_latchClockPin )                // find Latch Clock rising edge
    {
        bool lastLcClock = m_latchClock;

        double volt = m_latchClockPin->getVolt();      // Clock pin volt.

        if     ( volt > m_inputHighV ) m_latchClock = true;
        else if( volt < m_inputLowV )  m_latchClock = false;

        if( m_latchClock && !lastLcClock && m_changed )
        {
            for( int i=0; i<8; ++i )   // Store Shift-reg in Output Latch
            {
                bool data = m_shiftReg[i];
                m_latch[i] = data;
            }
            m_changed = false;
            addEvent = true;
        }
    }else if( m_changed )
    {
        m_changed = false;
        addEvent = true;
    }
    if( addEvent ) Simulator::self()->addEvent( m_propDelay, this );
}

void eShiftReg::runEvent()
{
    if( ! m_changed )
    {
        if( m_latchClockPin )
            for( int i=0; i<8; ++i )  setOut( i, m_latch[i] );
        else
            for( int i=0; i<8; ++i )  setOut( i, m_shiftReg[i] );
    }
    if( m_resSerOut )
    {
        m_serOutPin->setOut( false );
        m_serOutPin->stampOutput();
        m_resSerOut = false;
    }
    if( m_setSerOut )
    {
        m_serOutPin->setOut( m_shiftReg[7] );
        m_serOutPin->stampOutput();
        m_setSerOut = false;
    }
}

void eShiftReg::createLatchClockPin()
{
    ePin* epin      = new ePin( m_elmId+"latchClockPin", 0 );
    m_latchClockPin = new eSource( m_elmId+"eSourceLatchClock", epin );
    m_latchClockPin->setImp( m_inputImp );
}

void eShiftReg::createSerOutPin()
{
    ePin* epin  = new ePin( m_elmId+"serOutPin", 0 );
    m_serOutPin = new eSource( m_elmId+"eSourceSerOutPin", epin );
    m_serOutPin->setVoltHigh( m_outHighV );
    m_serOutPin->setImp( m_outImp );
}

void eShiftReg::setResetInv( bool inv )
{
    m_resetInv = inv;
    m_input[1]->setInverted( inv );
}
