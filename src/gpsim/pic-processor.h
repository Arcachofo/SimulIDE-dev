/*
   Copyright( C) 1998-2000 T. Scott Dattalo
   Copyright( C) 2013      Roy R. Rankin

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

#ifndef __PIC_PROCESSORS_H__
#define __PIC_PROCESSORS_H__

#include <stdio.h>
#include <iostream>

#include "gpsim_classes.h"
#include "processor.h"
#include "pic-registers.h"
#include "14bit-registers.h"
#include "watchdog.h"

class EEPROM;
class instruction;
class Register;
class SfrReg;
class pic_register;
class ConfigMemory;
class PicTrisRegister;
class PicLatchRegister;
class IO_SignalControl;

class pic_processor : public Processor
{
    public:
      uint config_word;      // as read from hex or cod file

      uint pll_factor;       // 2^pll_factor is the speed boost the PLL adds to the instruction execution rate.

      WDT   wdt;
      INDF  *indf;
      FSR   *fsr;
      Stack *stack;

      StatusReg *status;
      
      WREG   *Wreg;          // Used when W is a normal register
      PCL    *pcl;
      PCLATH *pclath;
      TMR0   tmr0;
      int    num_of_gprs;

      EEPROM* eeprom;       // set to NULL for PIC's that don't have a data EEPROM

      void add_SfrReg(Register *reg, uint addr,
                            RegisterValue por_value=RegisterValue(0,0),
                            const char *new_name=0,
                            bool warn_dup = true);
                            
      void add_SfrRegR(SfrReg *reg, uint addr,
                            RegisterValue por_value=RegisterValue(0,0),
                            const char *new_name=0,
                            bool warn_dup = true);
                            
      void delete_SfrReg(Register *pReg);
      void remove_SfrReg(Register *pReg);

      virtual Instruction* disasm( uint address,uint inst)=0;
      virtual void create_config_memory() = 0;
      virtual void tris_instruction(uint tris_register) {return;}

      virtual void enter_sleep();
      virtual void exit_sleep();
      virtual bool exit_wdt_sleep() { return true; } // WDT wakes sleep
      virtual bool swdten_active() { return true; } // WDTCON can enable WDT
      bool is_sleeping();

      virtual void interrupt() { return; }
      /// TEMPORARY - consolidate the various bp.set_interrupt() calls to one function:
      void BP_set_interrupt();
      void pm_write();

      virtual ConfigMemory * getConfigMemory(){ return m_configMemory;}
      virtual bool set_config_word(uint address, uint cfg_word);
      virtual uint get_config_word(uint address);
      virtual int get_config_index(uint address);
      virtual uint config_word_address() const {return 0x2007;}
      
      virtual void reset(RESET_TYPE r);

      virtual void create();

      virtual PROCESSOR_TYPE isa(){ return _PIC_PROCESSOR_; }
      virtual PROCESSOR_TYPE base_isa(){ return _PIC_PROCESSOR_; }
      virtual uint access_gprs() { return 0; }
      virtual uint bugs() { return 0; }    // default is no errata

      /* The program_counter class calls these two functions to get the upper bits of the PC
       * for branching( e.g. goto) or modify PCL instructions( e.g. addwf pcl,f) */
      virtual uint get_pclath_branching_jump()=0;
      virtual uint get_pclath_branching_modpcl()=0;

      virtual void option_new_bits_6_7(uint)=0;
      virtual void put_option_reg(uint) {}

      virtual void set_eeprom(EEPROM *e);
      virtual EEPROM *get_eeprom() { return( eeprom); }
      virtual void createMCLRPin(int pkgPinNumber);
      virtual void assignMCLRPin(int pkgPinNumber);
      virtual void unassignMCLRPin();
      virtual void osc_mode(uint );
      virtual void set_config3h(int64_t x){;}
      virtual string string_config3h(int64_t x){return string("fix string_config3h");}

      // Activity States reflect what the processor is currently doing
      //( The breakpoint class formally implemented this functionality).
      enum eProcessorActivityStates {
        ePAActive,      // Normal state
        ePAIdle,        // Processor is held in reset
        ePASleeping,    // Processor is sleeping
        ePAInterrupt,   // do we need this?
        ePAPMWrite      // Processor is busy performing a program memory write
      };
      eProcessorActivityStates getActivityState() { return m_ActivityState; }

      pic_processor(const char *_name=0);
      virtual ~pic_processor();

      void set_osc_pin_Number(uint i, uint val, PinModule *pm) {if( i < 4){osc_pin_Number[i] = val; m_osc_Monitor[i] = pm;}}
      unsigned char get_osc_pin_Number(uint i) {return( i<4)?osc_pin_Number[i]:253;}
      PinModule * get_osc_PinMonitor(uint i) { return( i<4)?m_osc_Monitor[i]:0; }

      void set_clk_pin(uint pkg_Pin_Number,
                           PinModule* PinMod,
                           const char* name,
                           bool in,
                    PicPortRegister *m_port = 0,
                    PicTrisRegister *m_tris = 0,
                    PicLatchRegister *m_lat = 0
                                      );
      void clr_clk_pin(uint pkg_Pin_Number, PinModule *PinMod,
                    PicPortRegister *m_port = 0,
                    PicTrisRegister *m_tris = 0,
                    PicLatchRegister *m_lat = 0
                     );

      virtual void set_int_osc(bool val){ internal_osc = val;}
      virtual bool get_int_osc(){ return internal_osc; }
      virtual void set_pplx4_osc(bool val){ PPLx4 = val;}
      virtual bool get_pplx4_osc(){ return PPLx4; }

      virtual void Wput(uint);
      virtual uint Wget();

    protected:
      ConfigMemory *m_configMemory;
      eProcessorActivityStates m_ActivityState;
      
      // Most midrange PIC's have a dedicated MCLR pin.
      // For the ones that don't, m_MCLR will be null.
      IOPIN* m_MCLR;
      IOPIN* m_MCLR_Save;
      int   m_MCLR_pin;
      PinMonitor *m_MCLRMonitor;
      
      unsigned char osc_pin_Number[4];
      PinModule *m_osc_Monitor[4];
      bool internal_osc;        // internal RC oscilator enabled on Config Word
      bool PPLx4;                // 4x PPL enabled on Config Word
      PeripheralSignalSource *clksource;
      SignalControl* clkcontrol;
      uint64_t sleep_time;
      ClockPhase* save_pNextPhase;
      ClockPhase* save_CurrentPhase;
};

#define cpu_pic (( pic_processor*)cpu)

// Bit field of known silicon bugs
#define BUG_NONE        0
#define BUG_DAW         0x00000001


//------------------------------------------------------------------------
// Base Class for configuration memory
//
// The configuration memory is only a tiny portion of the overall processor
// program memory space( only 1-word on the mid range devices). So, explicit
// attributes are created for each memory configuration word. Since the meaning
// of configuration memory varies from processor to processor, it is up to
// each process to derive from this class.

class ConfigWord : public Integer
{
    public:
      ConfigWord(const char *_name, uint default_val, pic_processor *pCpu, uint addr, bool EEw=true);
                 
      //virtual void get(char *buffer, int buf_size);
      virtual void get(int64_t &i);
      uint ConfigWordAdd() { return m_addr; }
      bool isEEWritable() { return EEWritable;}

    protected:
      pic_processor *m_pCpu;
      uint m_addr;
      bool EEWritable;
};

class ConfigMemory
{
    public:
      ConfigMemory(pic_processor *pCpu, uint nWords);
      ~ConfigMemory();
      
      int addConfigWord(uint addr, ConfigWord *);
      ConfigWord *getConfigWord(uint addr);
      int getnConfigWords() { return m_nConfigWords; }

    protected:
      pic_processor *m_pCpu;
      ConfigWord **m_ConfigWords;
      uint m_nConfigWords;
};
#endif
