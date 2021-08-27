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

    m_ram = mcu->getRam();
}
Interrupt::~Interrupt(){}

void Interrupt::reset()
{
    m_mode   = 0;
    m_raised = 0;
    m_enabled = 0;
}

void Interrupt::enableFlag( uint8_t en )
{
    m_enabled = en;
    if( en & m_raised && m_remember ) // If not enabled m_remember it until reenabled
        m_interrupts->addToPending( m_priority, this ); // Add to pending interrupts
}

void Interrupt::clearFlag()
{
    m_ram[m_flagReg] &= ~m_flagMask; // Clear Interrupt flag
    m_raised = false;
}

void Interrupt::writeFlag( uint8_t v )
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
        m_interrupts->addToPending( m_priority, this ); // Add to pending interrupts

        if( !m_callBacks.isEmpty() ) { for( McuModule* mod : m_callBacks ) mod->callBack(); }
    }
}

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
    m_enabled = 0;
    m_active = NULL;
}
Interrupts::~Interrupts(){}

void Interrupts::retI()
{
    m_active->exitInt();

    if( m_running.size() ) // Some interrupt was interrupted by this one
    {
        auto it = prev( m_running.end() );
        //m_active = it->second;   // Set interrupted Interrupt as active
        m_running.erase( it );   // Remove current Interrupt from running list
        m_pending.emplace( m_active->priority(), m_active ); // Add to pending Interrupts
    }
    //else
        m_active = NULL;
}

/*void Interrupts::enableGlobal( uint8_t en )
{
    /// if( m_enGlobal && !en ) resetInts();
    m_enGlobal = en;
}*/

void Interrupts::runInterrupts()
{
    if( !m_enabled ) return; // Global Interrupts disabled
    if( m_enabled > 1 )      // Execute interrupts some cycles later
    {
        m_enabled -= 1;
        return;
    }
    if( m_pending.size() == 0 ) return; // No Interrupts to run

    for( auto itt=m_pending.end(); itt!=m_pending.begin();  )
    {
        auto it=prev(itt);
        Interrupt* interrupt = it->second;

        if( !interrupt->enabled() )
        {
            m_pending.erase( it ); // Remove Interrupt from pending list
        }
        else if( interrupt->raised() )
        {
            uint8_t priority = it->first;
            if( m_active ) // Only interrupt other Interrupts with lower priority
            {
                if( priority <= m_active->priority() ) return;
                m_running.emplace( m_active->priority(), m_active ); // An interrupt being interrupted, add to running list.
            }
            m_pending.erase( it ); // Remove Interrupt from pending list
            interrupt->execute();
            m_active = interrupt;
            return;
}   }   }

void Interrupts::resetInts()
{
    m_active = NULL;
    m_pending.clear();
    m_running.clear();

    for( QString  inte : m_intList.keys() ) m_intList.value( inte )->reset();
}

void Interrupts::remove()
{
    for( QString  inte : m_intList.keys() ) delete m_intList.value( inte );
}

void Interrupts::addToPending( uint8_t pri, Interrupt* i )
{
    /// Interrupt are stored in std::multimap
    /// by priority order, highest at end
    m_pending.emplace( pri, i );
}


