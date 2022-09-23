/***************************************************************************
 *   Copyright (C) 2020 by Santiago González                               *
 *                                                                         *
 ***( see copyright.txt file at root folder )*******************************/

#include "e_mcu.h"
#include "cpubase.h"
//#include "mcuportctrl.h"
#include "mcuport.h"
#include "mcupin.h"
#include "ioport.h"
#include "iopin.h"
#include "mcuwdt.h"
#include "usartmodule.h"
#include "usartrx.h"
#include "mcuvref.h"
#include "mcusleep.h"
#include "simulator.h"
#include "basedebugger.h"
#include "editorwindow.h"

eMcu* eMcu::m_pSelf = NULL;

eMcu::eMcu( Component* comp, QString id )
    : DataSpace()
    , eElement( id )
    , m_interrupts( this )
{
    m_component = comp;

    cpu = NULL;
    m_wdt = NULL;
    m_clkPin = NULL;
    m_vrefModule = NULL;
    m_sleepModule = NULL;

    m_freq = 0;
    m_cPerInst = 1;

    m_wordSize  = 2;
    m_flashSize = 0;
    m_romSize   = 0;
    m_ramSize   = 0;

    m_firmware = "";
    m_device   = "";
    m_debugger = NULL;
    m_debugging = false;

    m_ramTable = new RamTable( NULL, this, false );
    m_ramTable->hide();

    m_cpuTable = new RamTable( NULL, this, true );
    m_cpuTable->hide();
}

eMcu::~eMcu()
{
    if( cpu ) delete cpu;
    m_interrupts.remove();
    for( McuModule* module : m_modules ) delete module;
    if( m_pSelf == this ) m_pSelf = NULL;
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
    //if( m_state == mcuRunning ) cpu->extClock( m_clkPin->getInpState() );
    if( m_state == mcuStopped ) return;
    if( m_debugging )
    {
        if( cyclesDone > 1 ) cyclesDone -= 1;
        else                 m_debugger->stepDebug();
        //Simulator::self()->addEvent( m_psCycle, this );
    }
    else if( m_state >= mcuRunning /*&& m_freq > 0*/ )
    {
        cpu->extClock( m_clkPin->getInpState() );
        //stepCpu();
        //Simulator::self()->addEvent( cyclesDone*m_psCycle, this );
    }
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
    else if( m_state >= mcuRunning && m_freq > 0 )
    {
        stepCpu();
        Simulator::self()->addEvent( cyclesDone*m_psCycle, this );
}   }

void eMcu::stepCpu()
{
    if( !m_flashSize || cpu->getPC() < m_flashSize )
    {
        if( m_state == mcuRunning ) cpu->runStep();
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

void eMcu::reset()
{
    m_cycle = 0;
    cyclesDone = 0;

    for( McuModule* module : m_modules ) module->reset();
    cpu->reset();
    m_interrupts.resetInts();
    DataSpace::initialize();
}

void eMcu::hardReset( bool r )
{
    Simulator::self()->cancelEvents( this );
    if( m_clkPin ) m_clkPin->changeCallBack( this, !r );  // External clock

    if( r ){
        reset();
        m_state = mcuStopped;
    }else{
        m_state = mcuRunning;
        if( m_freq > 0 ) Simulator::self()->addEvent( m_psCycle, this );
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

void eMcu::setFreq( double freq )
{
    if     ( freq < 0       ) freq = 0;
    else if( freq > 100*1e6 ) freq = 100*1e6;

    if( freq > 0 )
    {
        m_psCycle = 1e12*(m_cPerInst/freq); // Set Simulation cycles per Instruction cycle
        if( m_freq == 0 && m_state >= mcuRunning )// Previously stopped by freq = 0
        {
            Simulator::self()->cancelEvents( this );
            if( !m_clkPin   ) Simulator::self()->addEvent( m_psCycle, this );
        }
    }
    m_freq = freq;
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
    McuPort* port = m_mcuPorts.value( name );
    /// if( !port ) qDebug() << "ERROR: NULL Port:"<< name;
    return port;
}

McuPin* eMcu::getPin( QString pinName )
{
    if( pinName.isEmpty() ) return NULL;
    McuPin* pin = NULL;

    for( McuPort* port : m_mcuPorts )
    {
        pin = port->getPin( pinName );
        if( pin ) break;
    }
    if( !pin )
        qDebug() << "ERROR: eMcu::getPin NULL Pin:"<< pinName;
    return pin;
}

IoPort* eMcu::getIoPort( QString name )
{
    return m_ioPorts.value( name );
}

IoPin*  eMcu::getIoPin( QString pinName )
{
    if( pinName.isEmpty() ) return NULL;
    IoPin* pin = NULL;

    for( IoPort* port : m_ioPorts )
    {
        pin = port->getPin( pinName );
        if( pin ) break;
    }
    if( !pin )
        pin = getPin( pinName );

    if( !pin )
        qDebug() << "ERROR: eMcu::getIoPin NULL Pin:"<< pinName;
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

