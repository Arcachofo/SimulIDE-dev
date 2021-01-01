/*
   Copyright (C) 1998-2000 Scott Dattalo

This file is part of the libgpsim library of gpsim

This library is free software; you can redistribute it and/or
modify it under the terms of the GNU Lesser General Public
License as published by the Free Software Foundation; either
version 2.1 of the License, or (at your option) any later version.

This library is distributed in the hope that it will be useful,
but WITHOUT ANY WARRANTY; without even the implied warranty of
MERCHANTABILITY or FITNESS FOR A PARTICULAR PURPOSE.  See the GNU
Lesser General Public License for more details.

You should have received a copy of the GNU Lesser General Public
License along with this library; if not, see
<http://www.gnu.org/licenses/lgpl-2.1.html>.
*/


#include <stdio.h>
#include <iostream>
#include <iomanip>

#include "14bit-processors.h"
#include "pic-registers.h"

//#define DEBUG
#if defined(DEBUG)
#define Dprintf(arg) {printf("0x%06" PRINTF_GINT64_MODIFIER "X %s() ",cycles.get(),__FUNCTION__); printf arg; }
#else
#define Dprintf(arg) {}
#endif

Program_Counter::Program_Counter(const char *name, Processor *pM )
               : Value( name, pM )
{
  reset_address = 0;
  value = 0;
  pclath_mask = 0x1800;    // valid pclath bits for branching in 14-bit cores
  instruction_phase = 0;
}

Program_Counter::~Program_Counter()
{
}

//--------------------------------------------------
// increment - update the program counter. All non-branching instructions pass through here.
//
void Program_Counter::increment()
{
  value = (value + 1);
  if( value == memory_size ) // Some processors start at highest memory and roll over
  {
        printf("%s PC=0x%x == memory size 0x%x\n", __FUNCTION__, value, memory_size);
        value = 0;
  }
  else if( value > memory_size ) // assume this is a mistake
  {
        printf("%s PC=0x%x >= memory size 0x%x\n", __FUNCTION__, value, memory_size);
        cpu->halt();
  }
  update_pcl(); // Update PCL sfr to reflect current PC

  cpu_pic->m_phaseCurr->setNextPhase( cpu_pic->m_phase1 );
}

//--------------------------------------------------
// update_pcl - Updates the PCL from within the Program_Counter class.
// There is a separate method for this as the Program_Counter counts
// instructions (words) while the PCL can also point to bytes on
// 16 bit devices. So the PCL on 16-bit devices is always the double
// as the current Program_Counter

void Program_Counter::update_pcl()
{
  // For 12/14 bit devices the PCL will simply get set to the
  // current "value" of Program_Counter

  // Update pcl. Note that we don't want to pcl.put() because that
  // will trigger a break point if there's one set on pcl. (A read/write
  // break point on pcl should not be triggered by advancing the program
  // counter).
  cpu_pic->pcl->value.put(value & 0xff);
}

//--------------------------------------------------
// skip - Does the same thing that increment does, except that it records the operation
// in the trace buffer as a 'skip' instead of a 'pc update'.

void Program_Counter::skip()
{
  if ((value + 2) >= memory_size)
  {
    printf("%s PC=0x%x >= memory size 0x%x\n", __FUNCTION__, value, memory_size);
    cpu->halt();
  }
  else cpu_pic->m_phase2->firstHalf( value + 2 );
}

//--------------------------------------------------
// set - The next instruction is at an arbitrary location. This method is used
// by the command line parser--the GUI uses put_value directly.
//
void Program_Counter::set(Value *v)
{
    int i;
    v->get(i);
    put_value( i );
}

void Program_Counter::get(char *buffer, int buf_size)
{
  if( buffer ) snprintf(buffer, buf_size, "%u (0x%x)", value, value);
}

//--------------------------------------------------
// jump - update the program counter. All branching instructions except computed gotos
//        and returns go through here.

void Program_Counter::jump(uint new_address)
{
  Dprintf(("PC=0x%x new 0x%x\n",value,new_address));

  // Use the new_address and the cached pclath (or page select bits for 12 bit cores)
  // to generate the destination address:

  // see Update pcl comment in Program_Counter::increment()

  if (new_address >= memory_size)
  {
    printf("%s PC=0x%x >= memory size 0x%x\n", __FUNCTION__, new_address, memory_size);
    cpu->halt();
  }
  else cpu_pic->m_phase2->firstHalf(new_address);
}

//--------------------------------------------------
// interrupt - update the program counter. Like a jump, except pclath is ignored.

void Program_Counter::interrupt(uint new_address)
{
  if (new_address >= memory_size)
  {
    printf("%s PC=0x%x >= memory size 0x%x\n", __FUNCTION__, new_address, memory_size);
    cpu->halt();
  }
  else cpu_pic->m_phase2->firstHalf(new_address);
}

//--------------------------------------------------
// computed_goto - update the program counter. Anytime the pcl register is written to
//                 by the source code we'll pass through here.

void Program_Counter::computed_goto(uint new_address)
{
  // Use the new_address and the cached pclath (or page select bits for 12 bit cores)
  // to generate the destination address:

  value = new_address | cpu_pic->get_pclath_branching_modpcl() ;
  if (value >= memory_size)
  {
    printf("%s PC=0x%x >= memory size 0x%x\n", __FUNCTION__, value, memory_size);
    cpu->halt();
  }

  // Update PCL. As this is different for 12/14 and 16 bit devices
  // this will get handled by a method on its own so it is possible
  // to cope with different mappings PC-->PCL (direct, <<1, etc.)
  update_pcl();

  // The instruction modifying the PCL will also increment the program counter.
  // So, pre-compensate the increment with a decrement:
  value--;

  // The computed goto is a 2-cycle operation. The first cycle occurs within
  // the instruction (i.e. via the ::increment() method). The second cycle occurs here:
  cpu_pic->m_phase2->advance();
}

//--------------------------------------------------
// new_address - write a new value to the program counter. All returns pass through here.

void Program_Counter::new_address(uint new_address)
{
  if (new_address >= memory_size)
  {
    printf("%s PC=0x%x >= memory size 0x%x\n", __FUNCTION__, new_address, memory_size);
    cpu->halt();
  }
  else cpu_pic->m_phase2->firstHalf(new_address);
}

//--------------------------------------------------
// get_next - get the next address that is just pass the current one
//            (used by 'call' to obtain the return address)

uint Program_Counter::get_next()
{
  uint new_address = value + cpu_pic->program_memory[value]->instruction_size();

  if (new_address >= memory_size)
  {
    printf("%s PC=0x%x >= memory size 0x%x\n", __FUNCTION__, new_address, memory_size);
    cpu->halt();
  }
  return( new_address);
}


//--------------------------------------------------
// put_value - Change the program counter without affecting the cycle counter
//             (This is what's called if the user changes the pc.)

void Program_Counter::put_value(uint new_value)
{
#define PCLATH_MASK              0x1f

  if (new_value >= memory_size)
  {
    printf("%s PC=0x%x >= memory size 0x%x\n", __FUNCTION__, new_value, memory_size);
    cpu->halt();
  }
  value = new_value;
  cpu_pic->pcl->value.put(value & 0xff);
  cpu_pic->pclath->value.put((new_value >> 8) & PCLATH_MASK);

  cpu_pic->pcl->update();
  cpu_pic->pclath->update();
  update();
}

void Program_Counter::reset()
{
  value = reset_address;
  value = (value >= memory_size) ? value - memory_size : value;
  cpu_pic->m_phase2->firstHalf( value );
}

//--------------------------------------------------
// member functions for the OPTION base class
//--------------------------------------------------
OPTION_REG::OPTION_REG(Processor *pCpu, const char *pName)
          : SfrReg(pCpu, pName )
{
  por_value = RegisterValue(0xff,0);
  wdtr_value = RegisterValue(0xff,0);
  // The chip reset will place the proper value here.
  value = RegisterValue(0,0);  // por_value;
}

void OPTION_REG::initialize() // make sure intial por_value does it's stuff
{
    cpu_pic->tmr0.new_prescale();
    cpu_pic->wdt.set_postscale( (value.get() & PSA) ? (value.get() & ( PS2 | PS1 | PS0 )) : 0);
    cpu_pic->option_new_bits_6_7(value.get() & (T0CS | BIT6 | BIT7));
}

void OPTION_REG::put(uint new_value)
{
  uint old_value = value.get();
  value.put(new_value);

  // Now check the rest of the tmr0 bits.
  if( (value.get() ^ old_value) & (T0CS | T0SE | PSA | PS2 | PS1 | PS0))
    cpu_pic->tmr0.new_prescale();

  if( (value.get() ^ old_value) & (PSA | PS2 | PS1 | PS0))
    cpu_pic->wdt.set_postscale( (value.get() & PSA) ? (value.get() & ( PS2 | PS1 | PS0 )) : 0);

  if( (value.get() ^ old_value) & (T0CS | BIT6 | BIT7))
    cpu_pic->option_new_bits_6_7(value.get() & (T0CS | BIT6 | BIT7));

}

void OPTION_REG::reset(RESET_TYPE r)
{
  putRV(por_value);
}

// On 14bit enhanced cores the prescaler does not affect the watchdog
OPTION_REG_2::OPTION_REG_2(Processor *pCpu, const char *pName )
            : OPTION_REG(pCpu, pName )
{
}

void OPTION_REG_2::initialize()
{
    cpu_pic->tmr0.new_prescale();
    cpu_pic->option_new_bits_6_7(value.get() & (T0CS | BIT6 | BIT7));
}

void OPTION_REG_2::put(uint new_value)
{
  uint old_value = value.get();
  value.put(new_value);

  // Now check the rest of the tmr0 bits.
  if( (value.get() ^ old_value) & (T0CS | T0SE | PSA | PS2 | PS1 | PS0))
    cpu_pic->tmr0.new_prescale();

  if( (value.get() ^ old_value) & (T0CS | BIT6 | BIT7))
    cpu_pic->option_new_bits_6_7(value.get() & (T0CS | BIT6 | BIT7));
}
