/*
   Copyright( C) 1998 T. Scott Dattalo

This file is part of the libgpsim library of gpsim

This library is free software; you can redistribute it and/or
modify it under the terms of the GNU Lesser General Public
License as published by the Free Software Foundation; either
version 2.1 of the License, or( at your option) any later version.

This library is distributed in the hope that it will be useful,
but WITHOUT ANY WARRANTY; without even the implied warranty of
MERCHANTABILITY or FITNESS FOR A PARTICULAR PURPOSE.  See the GNU
Lesser General Public License for more details.

You should have received a copy of the GNU Lesser General Public
License along with this library; if not, see
<http://www.gnu.org/licenses/lgpl-2.1.html>.
*/

#include <typeinfo>
#include <stdio.h>
#ifdef _WIN32
#include "unistd.h"
#else
#include <unistd.h>
#endif
#ifndef _MSC_VER
#include <sys/time.h>
#endif
#include <time.h>
#include <iostream>
#include <iomanip>
#include <string>

#include "pic-processor.h"
#include "pic-registers.h"
#include "pic_list.h"

//========================================================================
//
// pic_processor
//
// This file contains all( most?) of the code that simulates those features
// common to all pic microcontrollers.
//
//========================================================================

pic_processor::pic_processor(const char *_name )
    : Processor(_name),
      wdt(this, 18.0e-3),indf(0),fsr(0), stack(0), status(0),
      Wreg(0), pcl(0), pclath(0),
      tmr0(this,"tmr0" ),
      m_configMemory(0),
      m_MCLR(0), m_MCLR_Save(0), m_MCLRMonitor(0),
      PPLx4(false), clksource(0), clkcontrol(0)
{
    m_phase1     = new PhaseExec1( this );
    m_phase2     = new PhaseExec2( this );
    m_phaseInter = new PhaseInter( this );
    m_phaseIdle  = new PhaseIdle( this );
    m_phaseCurr  = m_phase1;

    eeprom = 0;

    pll_factor = 0;

    Integer::setDefaultBitmask(0xff);

    // Test code for logging to disk:
    for( int i=0; i<4; i++ ) osc_pin_Number[i] = 254;
}

pic_processor::~pic_processor()
{
    delete_SfrReg(Wreg);
    delete_SfrReg(pcl);

    delete_SfrReg(pclath);
    delete_SfrReg(status);
    delete_SfrReg(indf);

    delete stack;

    delete m_phase1;
    delete m_phase2;
    delete m_phaseInter;
    delete m_phaseIdle;

    delete m_configMemory;

    if( m_MCLR)        m_MCLR->setMonitor(0);
    if( m_MCLR_Save)   m_MCLR_Save->setMonitor(0);
    if( m_MCLRMonitor) delete m_MCLRMonitor;

    if( clksource)  delete clksource;
    if( clkcontrol) delete clkcontrol;
}

//-------------------------------------------------------------------
//
//    create
//
//  The purpose of this member function is to 'create' a pic processor.
// Since this is a base class member function, only those things that
// are common to all pics are created.

void pic_processor::create( )
{
    init_program_memory( program_memory_size() );
    init_register_memory( register_memory_size() );

    pc->set_cpu( this );

    Wreg   = new WREG( this, "W" );
    pcl    = new PCL( this, "pcl" );
    pclath = new PCLATH( this, "pclath" );
    status = new StatusReg( this );
    indf   = new INDF( this, "indf" );

    register_bank = &registers[0];  // Define the active register bank

    create_config_memory();
}

//-------------------------------------------------------------------
//
// add_SfrReg
//
// The purpose of this routine is to add one special function register
// to the file registers. If the sfr has a physical address( like the
// status or tmr0 registers) then a pointer to that register will be
// placed in the file register map.

// FIXME It doesn't make any sense to initialize the por_value here!
// FIXME The preferred way is to initialize all member data in their
// FIXME parent's constructor.

void pic_processor::add_SfrReg( Register *reg, uint addr,
                                     RegisterValue por_value,
                                     const char *new_name,
                                     bool warn_dup )
{
    reg->set_cpu(this);
    if(addr < register_memory_size())
    {
        if( registers[addr])
        {
            if( registers[addr]->isa() == Register::INVALID_REGISTER)
            {
                delete registers[addr];
                registers[addr] = reg;
            }
        }
        else registers[addr] = reg;

        reg->address = addr;
        reg->alias_mask = 0;

        if(new_name) reg->new_name(new_name);
    }
    reg->value       = por_value;
    reg->por_value   = por_value;  /// FIXME why are we doing this?
    reg->initialize();
}

// Use this function when register is initialized on WDT reset to
// same value as a POR.
void pic_processor::add_SfrRegR(SfrReg *reg, uint addr,
                                      RegisterValue por_value,
                                      const char *new_name,
                                      bool warn_dup )
{
    add_SfrReg( reg, addr, por_value, new_name, warn_dup );
    reg->wdtr_value = por_value;
}


// This both deletes the register from the registers array,
// but also deletes the register object.

void pic_processor::delete_SfrReg( Register *pReg )
{
    if( pReg )
    {
        uint a = pReg->getAddress();

        if( 0) cout << __FUNCTION__ << " addr = 0x"<<hex<<a <<" reg " << pReg->name_str<<endl;

        if( a<nRegisters && registers[a] == pReg ) delete_file_registers(a,a);
        else delete pReg;
    }
}

// This is the inverse of add_SfrReg and does not delete the register.

void pic_processor::remove_SfrReg(Register *ppReg)
{
    if( ppReg)
    {
        uint a = ppReg->getAddress();
        if( a == AN_INVALID_ADDRESS ) return;
        if( registers[a] == ppReg )   delete_file_registers(a,a,true);
    }
}

bool pic_processor::set_config_word( uint address, uint cfg_word )
{
    int i = get_config_index(address);

    if( i >= 0)
    {
        m_configMemory->getConfigWord(i)->set((int)cfg_word);
        if( i==0 )
        {
            config_word = cfg_word;
        }
        return true;
    }
    return false;
}

uint pic_processor::get_config_word(uint address)
{
    int i= get_config_index(address);

    if( i >= 0 ) return m_configMemory->getConfigWord(i)->getVal();

    return 0xffffffff;
}

int pic_processor::get_config_index(uint address)
{
    if( m_configMemory)
    {
        for(int i = 0; i < m_configMemory->getnConfigWords(); i++)
        {
            if( m_configMemory->getConfigWord(i))
            {
                if( m_configMemory->getConfigWord(i)->ConfigWordAdd() == address)
                {
                    return i;
                }
            }
        }
    }
    return -1;
}

//------------------------------------------------------------------------
// ConfigMemory - Base class
ConfigWord::ConfigWord(const char*_name, uint default_val, pic_processor *pCpu, uint addr, bool EEw)
    : Integer(_name, default_val ), m_pCpu(pCpu), m_addr(addr),
      EEWritable(EEw)
{ }

void ConfigWord::get(int64_t &i)
{
    Integer::get(i);
}

//------------------------------------------------------------------------
ConfigMemory::ConfigMemory(pic_processor *pCpu, uint nWords)
    : m_pCpu(pCpu)
    , m_nConfigWords(nWords)
{
    if( nWords > 0 && nWords < 100)
    {
        m_ConfigWords = new ConfigWord*[nWords];

        for( uint i=0; i<nWords; i++ ) m_ConfigWords[i] = 0;
    }
}

ConfigMemory::~ConfigMemory()
{
    delete [] m_ConfigWords;
}

int ConfigMemory::addConfigWord(uint addr, ConfigWord *pConfigWord)
{
    if( addr < m_nConfigWords)
    {
        m_ConfigWords[addr] = pConfigWord;
        return 1;
    }
    delete pConfigWord;
    return 0;
}

ConfigWord *ConfigMemory::getConfigWord(uint addr)
{
    return addr < m_nConfigWords ? m_ConfigWords[addr] : 0;
}
//-------------------------------------------------------------------
class MCLRPinMonitor : public PinMonitor
{
public:
    MCLRPinMonitor(pic_processor *pCpu);
    ~MCLRPinMonitor() {}

    virtual void setDrivenState(char);
    virtual void setDrivingState(char) {}
    virtual void set_nodeVoltage(double) {}
    virtual void putState(char) {}
    virtual void setDirection() {}

private:
    pic_processor *m_pCpu;
    char m_cLastResetState;
};

MCLRPinMonitor::MCLRPinMonitor(pic_processor *pCpu)
    : m_pCpu(pCpu),
      m_cLastResetState('I')  // I is not a valid state. It's used here for 'I'nitialization
{ }

void MCLRPinMonitor::setDrivenState(char newState)
{
    if( newState =='0' || newState =='w')
    {
        m_cLastResetState = '0';
        m_pCpu->reset(MCLR_RESET);
    }

    if( newState =='1' || newState =='W')
    {
        if( m_cLastResetState == '0') m_pCpu->reset(EXIT_RESET);

        m_cLastResetState = '1';
    }
}

//-------------------------------------------------------------------
void pic_processor::createMCLRPin(int pkgPinNumber)
{
    if( m_MCLR) cout << "BUG?: assigning multiple MCLR pins: " << __FILE__ << dec << " " << __LINE__ << endl;

    m_MCLR = new IOPIN( "MCLR", OPEN_COLLECTOR ) ;
    assign_pin( pkgPinNumber, m_MCLR );

    m_MCLRMonitor = new MCLRPinMonitor(this);
    m_MCLR->setMonitor(m_MCLRMonitor);
}

//-------------------------------------------------------------------
// This function is called instead of createMCLRPin where the pin
// is already defined, but the configuration word has set the function
// to MCLR

void pic_processor::assignMCLRPin(int pkgPinNumber)
{
    if( m_MCLR == NULL )
    {
        m_MCLR_pin = pkgPinNumber;
        m_MCLR = new IOPIN( "MCLR", OPEN_COLLECTOR ) ;

        m_MCLR_Save = get_pin( pkgPinNumber );
        assign_pin( pkgPinNumber,m_MCLR );

        m_MCLRMonitor = new MCLRPinMonitor(this);
        m_MCLR->setMonitor( m_MCLRMonitor );
    }
    else if( m_MCLR != get_pin( pkgPinNumber ) )
    {
        cout << "BUG?: assigning multiple MCLR pins: "
             << dec << pkgPinNumber << " " << __FILE__ <<  " "
             << __LINE__ << endl;
    }
}
//-------------------------------------------------------------------
// This function sets the pin currently set as MCLR back to its original function
void pic_processor::unassignMCLRPin()
{
    if( m_MCLR_Save )
    {
        assign_pin( m_MCLR_pin, m_MCLR_Save );

        if( m_MCLR )
        {
            m_MCLR->setMonitor(0);
            m_MCLR = NULL;
            
            if( m_MCLRMonitor )
            {
                delete m_MCLRMonitor;
                m_MCLRMonitor = NULL;
            }
        }
    }
}
//--------------------------------------------------
//
class IO_SignalControl : public SignalControl
{
public:
    IO_SignalControl(char _dir){ direction = _dir; }
    ~IO_SignalControl(){}
    virtual char getState() { return direction; }
    virtual void release() {}
    void setState(char _dir) { direction = _dir; }

private:
    char direction;
};

// This function sets a label on a pin and if PinMod is defined
// removes its control from it's port register
//
void pic_processor::set_clk_pin(uint pkg_Pin_Number,
                                PinModule *PinMod,
                                const char * name,
                                bool in,
                                PicPortRegister *m_port,
                                PicTrisRegister *m_tris,
                                PicLatchRegister *m_lat)
{
    if( PinMod )
    {
        if( m_port)
        {
            uint mask = m_port->getEnableMask();
            mask &= ~(1<< PinMod->getPinNumber());
            m_port->setEnableMask(mask);
            
            if( m_tris ) m_tris->setEnableMask(mask);
            if( m_lat )  m_lat->setEnableMask(mask);
        }
        if( !clksource)
        {
            clksource  = new PeripheralSignalSource(PinMod);
            clkcontrol = new IO_SignalControl(in ? '1' : '0');
        }
        PinMod->setSource( clksource );
        PinMod->setControl( clkcontrol );
        PinMod->updatePinModule();
    }
}

// This function reverses the effects of the previous function
void pic_processor::clr_clk_pin(uint pkg_Pin_Number,
                                PinModule *PinMod,
                                PicPortRegister *m_port,
                                PicTrisRegister *m_tris,
                                PicLatchRegister *m_lat)
{
    if( PinMod)
    {
        if( m_port)
        {
            uint mask = m_port->getEnableMask();
            mask |=( 1<< PinMod->getPinNumber());
            m_port->setEnableMask(mask);
            
            if( m_tris) m_tris->setEnableMask(mask);
            if( m_lat)  m_lat->setEnableMask(mask);
        }
        PinMod->setSource(0);
        PinMod->setControl(0);
        PinMod->updatePinModule();
    }
}

void pic_processor::osc_mode(uint value)
{
    IOPIN *m_pin;
    uint pin_Number =  get_osc_pin_Number(0);

    if( pin_Number < 253)  m_pin = get_pin( pin_Number );

    if(( pin_Number =  get_osc_pin_Number(1)) < 253
            &&( m_pin = get_pin( pin_Number )) )
    {
        pll_factor = 0;
        if( value < 5 )
        {
            set_clk_pin(pin_Number, m_osc_Monitor[1], "OSC2", true);
        }
        else if(value == 6 )
        {
            pll_factor = 2;
            set_clk_pin(pin_Number, m_osc_Monitor[1], "CLKO", false);
        }
        else clr_clk_pin(pin_Number, m_osc_Monitor[1]);
    }
}

void pic_processor::Wput(uint value)
{
    Wreg->put(value);
}

uint pic_processor::Wget()
{
    return Wreg->get();
}

void pic_processor::set_eeprom( EEPROM *e )
{
    eeprom = e;
}

void pic_processor::enter_sleep() // processor is about to go to sleep, update the status register.
{
    status->put_TO(1);
    status->put_PD(0);

    sleep_time = m_cycle;
    wdt.update();
    pc->increment();
    save_pNextPhase = m_phaseCurr->getNextPhase();
    save_CurrentPhase = m_phaseCurr;
    m_phaseCurr->setNextPhase( m_phaseIdle );
    m_phaseCurr = m_phaseIdle;
    m_phaseCurr->setNextPhase( m_phaseIdle );
    m_ActivityState = ePASleeping;
}

void pic_processor::exit_sleep()
{
    if( m_cycle == sleep_time )// If enter and exit sleep at same clock cycle, restore execute state
    {
        m_phaseCurr = save_CurrentPhase;
        m_phaseCurr->setNextPhase( save_pNextPhase );
    }
    else
    {
        m_phaseCurr = m_phase1;
        m_phaseCurr->setNextPhase( m_phase1 );
    }
    m_ActivityState = ePAActive;
}

bool pic_processor::is_sleeping()
{
    return m_ActivityState == ePASleeping;
}

void pic_processor::BP_set_interrupt()
{
    m_phaseInter->firstHalf();
}

void pic_processor::pm_write( ) // program memory write
{
    m_ActivityState = ePAPMWrite;

    do incrementCycle();     // burn cycles until we're through writing
    while( have_pm_write() );
}

void pic_processor::reset( RESET_TYPE r )// Reset the pic on desired reset type.
{
    m_halted = true;
    resetRegisters( r );
    m_halted = false;

    stack->reset( r );
    wdt.reset( r );
    pc->reset();
    //m_activeCB.clear(); // Usart not working here
    //m_cycle = 0;
    //qDebug() << "pic_processor::reset"<<m_activeCB.uniqueKeys();

    switch( r )
    {
    case POR_RESET:
        //cout << "POR reset\n";
        m_phaseCurr = m_phaseCurr ? m_phaseCurr : m_phase1;
        m_ActivityState = ePAActive;
        break;

    case SOFT_RESET:
        cout << "Reset due to Software reset instruction\n";
        m_phaseCurr = m_phase1;
        m_phaseCurr->setNextPhase( m_phase1 );
        m_ActivityState = ePAActive;
        break;

    case MCLR_RESET:
        cout << "MCLR reset\n";
        m_phaseCurr = m_phaseIdle;
        m_phaseCurr->setNextPhase( m_phaseIdle );
        m_ActivityState = ePAIdle;
        break;

    case IO_RESET:
        cout << "IO reset\n";
        m_phaseCurr = m_phase1;
        m_phaseCurr->setNextPhase( m_phase1 );
        m_ActivityState = ePAActive;
        break;

    case WDT_RESET:
        cout << "Reset on Watch Dog Timer expire\n";
        m_phaseCurr = m_phaseCurr ? m_phaseCurr : m_phase1;
        m_phaseCurr->setNextPhase( m_phase1 );
        m_ActivityState = ePAActive;
        break;

    case EXIT_RESET:        // MCLR reset has cleared
        cout <<"MCLR exit, resume execution\n";
        m_phaseCurr = m_phase1;
        m_phaseCurr->setNextPhase(m_phase1);
        m_ActivityState = ePAActive;
        return;
        break;

    case STKOVF_RESET:
        cout << "Reset on Stack overflow\n";
        m_phaseCurr = m_phaseCurr ? m_phaseCurr : m_phaseIdle;
        m_phaseCurr->setNextPhase(m_phaseIdle);
        m_ActivityState = ePAActive;
        break;

    case STKUNF_RESET:
        cout << "Reset on Stack undeflow\n";
        m_phaseCurr = m_phaseCurr ? m_phaseCurr : m_phaseIdle;
        m_phaseCurr->setNextPhase(m_phaseIdle);
        m_ActivityState = ePAActive;
        break;

    default:
        printf("pic_processor::reset unknow reset type %d\n", r);
        m_ActivityState = ePAActive;
        break;
    }
}
