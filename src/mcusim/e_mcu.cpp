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

#include "e_mcu.h"
#include "mcucore.h"
#include "mcuportctrl.h"
#include "mcupin.h"
#include "mcuwdt.h"
#include "usartmodule.h"
#include "usartrx.h"
#include "mcuvref.h"
#include "mcusleep.h"
#include "simulator.h"
#include "basedebugger.h"
#include "editorwindow.h"

eMcu* eMcu::m_pSelf = NULL;

eMcu::eMcu( QString id )
    : DataSpace()
    , eElement( id )
    , m_interrupts( this )
{
    m_pSelf = this;

    cpu = NULL;
    m_wdt = NULL;
    m_vrefModule = NULL;
    m_sleepModule = NULL;
    m_ctrlPort = NULL;
    m_clkPin = NULL;

    m_cPerInst = 1;
    setFreq( 16*1e6 );

    m_flashSize = 0;
    m_wordSize  = 2;

    m_romSize   = 0;

    m_firmware = "";
    m_device   = "";
    m_debugger = NULL;
    m_debugging = false;

    m_ramTable = new RamTable( NULL, this );
    m_ramTable->hide();
}

eMcu::~eMcu()
{
    if( cpu ) delete cpu;
    m_interrupts.remove();
    for( McuModule* module : m_modules ) delete module;
    if( m_pSelf == this ) m_pSelf= NULL;
}

void eMcu::stamp()
{
    reset();
    m_state = mcuRunning;
    //m_state = mcuStopped;

    /*Simulator::self()->cancelEvents( this );
    if( m_clkPin ) m_clkPin->changeCallBack( this );  // External clock
    else           Simulator::self()->addEvent( m_psCycle, this );*/
}

void eMcu::voltChanged()  // External clock
{
    if( m_state == mcuRunning ) cpu->runClock( m_clkPin->getInpState() );
}

void eMcu::runEvent()
{
    if( m_state == mcuStopped ) return;
    if( m_debugging )
    {
        if( cyclesDone > 1 ) cyclesDone -= 1;
        else                 m_debugger->stepDebug();
        Simulator::self()->addEvent( m_psCycle, this );
    }
    else if( m_state >= mcuRunning )
    {
        stepCpu();
        Simulator::self()->addEvent( cyclesDone*m_psCycle, this );
}   }

void eMcu::reset()
{
    m_cycle = 0;
    cyclesDone = 0;

    for( McuModule* module : m_modules ) module->reset();
    cpu->reset();
    m_interrupts.resetInts();
    DataSpace::initialize();
}

void eMcu::stepCpu()
{
    if( cpu->PC < m_flashSize )
    {
        if( m_state == mcuRunning ) cpu->runDecoder();
        m_interrupts.runInterrupts();
    }
    else m_state = mcuStopped; /// TODO: Crash

    m_cycle += cyclesDone;
}

void eMcu::setDebugger( BaseDebugger* deb )
{
    m_debugger = deb;
    m_ramTable->setDebugger( deb );
}

void eMcu::cpuReset( bool r )
{
    Simulator::self()->cancelEvents( this );
    if( r ){
        reset();
        m_state = mcuStopped;
    }else{
        m_state = mcuRunning;
        if( m_clkPin ) m_clkPin->changeCallBack( this );  // External clock
        else           Simulator::self()->addEvent( m_psCycle, this );
    }
}

void eMcu::sleep( bool s )
{
    if( !m_sleepModule->enabled() ) return;

    int mode = -1;
    if( s )     // Go to Sleep
    {
        mode = m_sleepModule->mode();
    }

    for( McuModule* module : m_modules ) module->sleep( mode );
}

uint32_t eMcu::getStack() { return cpu->GET_STACK(); }

int eMcu::status() { return getRamValue( m_sregAddr ); }

int eMcu::pc() { return cpu->PC; }

void eMcu::setFreq( double freq )
{
    if     ( freq < 0  )  freq = 0;
    else if( freq > 100*1e6 ) freq = 100*1e6;

    m_freq = freq;
    m_psCycle = 1e12*(m_cPerInst/m_freq); // Set Simulation cycles per Instruction cycle
}

void eMcu::setEeprom( QVector<int>* eep )
{
    int size = m_romSize;
    if( eep->size() < size ) size = eep->size();

    for( int i=0; i<size; ++i ) setRomValue( i, eep->at(i) );
}

McuTimer* eMcu::getTimer( QString name )
{
    McuTimer* timer = m_timerList.value( name );
    if( !timer ) qDebug() << "ERROR: NULL Timer:"<< name;
    return timer;
}

McuPort* eMcu::getPort( QString name )
{
    McuPort* port = m_portList.value( name );
    /// if( !port ) qDebug() << "ERROR: NULL Port:"<< name;
    return port;
}

McuPin* eMcu::getCtrlPin( QString pinName )
{
    if( !m_ctrlPort ) return NULL;
    if( pinName.isEmpty() ) return NULL;
    return m_ctrlPort->getPin( pinName );
}

McuPin* eMcu::getPin( QString pinName )
{
    if( pinName.isEmpty() ) return NULL;
    QString portName = pinName.left( 5 );
    McuPort* port = getPort( portName );
    if( !port ) return NULL;

    QString pinStr = pinName;
    pinStr.remove( portName );

    McuPin* pin = port->getPinN( pinStr.toInt() );
    if( !pin ) qDebug() << "ERROR: NULL Pin:"<< pinName;
    return pin;
}

bool eMcu::setCfgWord( uint16_t addr, uint16_t data )
{
    if( m_cfgWords.contains( addr ) )
    {
        m_cfgWords[addr] = data;
        qDebug() <<"    Loaded Config Word at:"<<addr<<data;
        return true;
    }
    return false;
}

uint16_t eMcu::getCfgWord( uint16_t addr )
{
    if( addr ) return m_cfgWords.value( addr );
    return m_cfgWords.values().first();
}

void eMcu::wdr() { if( m_wdt ) m_wdt->reset(); }

void eMcu::enableInterrupts( uint8_t en )
{
    if( en > 1 ) en = 1;
    m_interrupts.enableGlobal( en );
}

McuVref* eMcu::vrefModule() { return m_vrefModule; }
//McuSleep* eMcu::sleepModule() { return m_sleepModule; }

