/***************************************************************************
 *   Copyright (C) 2020 by santiago González                               *
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

#include "mcuinterrupts.h"
#include "mcucore.h"
#include "e_mcu.h"

Interrupt::Interrupt( QString name, uint16_t vector, eMcu* mcu )
{
    m_mcu = mcu;
    m_name = name;
    m_vector = vector;
    m_autoClear = false;
    m_remember = false;
    m_nextInt = NULL;

    m_ram = mcu->getRam();
}
Interrupt::~Interrupt(){}

void Interrupt::reset()
{
    m_mode    = 0;
    m_enabled = 0;
    m_raised  = false;
}

void Interrupt::enableFlag( uint8_t en )
{
    if( m_enabled == en ) return;
    m_enabled = en;

    if( en & m_raised && m_remember ) // If not enabled m_remember it until reenabled
        m_interrupts->addToPending( this ); // Add to pending interrupts
}

void Interrupt::clearFlag()
{
    if( !m_raised ) return;
    m_raised = false;

    m_ram[m_flagReg] &= ~m_flagMask; // Clear Interrupt flag

    m_interrupts->remFromPending( this );
}

void Interrupt::flagCleared( uint8_t )
{
    if( !m_raised ) return;
    m_raised = false;
    m_interrupts->remFromPending( this );
}

void Interrupt::writeFlag( uint8_t v ) // Clear flag by writting 1 to it
{
    if( v & m_flagMask ) clearFlag();
}

void Interrupt::raise( uint8_t v )
{
    if( m_raised ) return;
    m_raised = true;

    m_ram[m_flagReg] |= m_flagMask; // Set Interrupt flag

    if( m_enabled )
    {
        m_interrupts->addToPending( this ); // Add to pending interrupts
        if( !m_callBacks.isEmpty() ) { for( McuModule* mod : m_callBacks ) mod->callBack(); }
}   }

void Interrupt::execute()
{
    m_mcu->cpu->CALL_ADDR( m_vector );
}

void Interrupt::exitInt() // Exit from this interrupt
{
    if( m_autoClear ) clearFlag();
    if( !m_exitCallBacks.isEmpty() ) { for( McuModule* mod : m_exitCallBacks ) mod->callBack(); }
}

void Interrupt::callBack( McuModule* mod, bool call ) // Add Modules to be called at Interrupt raise
{
    if( call )
    { if( !m_callBacks.contains( mod ) ) m_callBacks.append( mod ); }
    else m_callBacks.removeAll( mod );
}

void Interrupt::exitCallBack( McuModule* mod, bool call )
{
    if( call )
    { if( !m_exitCallBacks.contains( mod ) ) m_exitCallBacks.append( mod ); }
    else m_exitCallBacks.removeAll( mod );
}

//------------------------               ------------------------
//---------------------------------------------------------------

Interrupts::Interrupts( eMcu* mcu )
{
    m_mcu = mcu;
}
Interrupts::~Interrupts(){}

void Interrupts::resetInts()
{
    m_enabled = 0;
    m_active  = NULL;
    m_pending = NULL;
    m_running = NULL;

    for( QString inte : m_intList.keys() ) m_intList.value( inte )->reset();
}

void Interrupts::retI()
{
    if( !m_active ) { qDebug() << "Interrupts::retI Error: No active Interrupt"; return; }
    m_active->exitInt();

    if( m_running ) // Some interrupt was interrupted by this one
    {
        Interrupt* running = m_running;
        m_running = m_running->m_nextInt;   // Remove from running list
        addToPending( running );            // Add to pending Interrupts
    }
    m_active = NULL;
}

void Interrupts::runInterrupts()
{
    if( !m_enabled ) return; // Global Interrupts disabled
    if( m_enabled > 1 ){ m_enabled -= 1; return; }// Execute interrupts some cycles later

    if( !m_pending ) return; // No Interrupts pending to execute;
    if( m_active ) // An interrupt is running,
    {
        if( m_pending->priority() > m_active->priority() )// Only interrupt other Interrupts with lower priority
        {
            m_active->m_nextInt = m_running;
            m_running = m_active;            // An interrupt being interrupted, add to running list.
        }
        else return;
    }
    m_pending->execute();
    m_active = m_pending;
}

void Interrupts::remove()
{
    for( QString inte : m_intList.keys() ) delete m_intList.value( inte );
}

void Interrupts::addToPending( Interrupt* newInt )
{
    Interrupt* preInt = NULL;
    Interrupt* posInt = m_pending;
    while( posInt )
    {
        if( posInt == newInt ) // Interrupt already in the list
        { return; } // ERROR
        if( newInt->priority() <= posInt->priority() )  // High priority first & last In, las Out
        {                                               // Keep iterating
            preInt = posInt;
            posInt = posInt->m_nextInt;
        }
        else break;
    }
    newInt->m_nextInt = posInt;
    if( preInt ) preInt->m_nextInt = newInt;
    if( !posInt ) m_pending = newInt;
}

void Interrupts::remFromPending( Interrupt* remInt )
{
    Interrupt* preInt = NULL;
    Interrupt* posInt = m_pending;
    while( posInt )
    {
        if( posInt == remInt )
        {
            if( preInt ) preInt->m_nextInt = posInt->m_nextInt;
            else         m_pending = posInt->m_nextInt;
            break;
        }
        preInt = posInt;
        posInt= posInt->m_nextInt;
}   }
