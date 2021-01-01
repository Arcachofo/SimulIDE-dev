/*
   Copyright (C) 1998 T. Scott Dattalo

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
#include <string>
#include <cstdio>

#include "pic-processor.h"
#include "14bit-registers.h"

#include "pic-instructions.h"
#include "12bit-instructions.h"
#include "16bit-instructions.h"
#include "16bit-processors.h"
#include "16bit-registers.h"

Branching::Branching( Processor *pCpu, uint opco, uint address)
         : Instruction(pCpu, opco, address),
           destination_index(0),
           absolute_destination_index(0)
{
}

void Branching::decode( Processor *pCpu, uint opco)
{
  opcode = opco;
  cpu = pCpu;

  switch(cpu16->base_isa())
  {
    case  _PIC18_PROCESSOR_:
      destination_index = (opco & 0xff)+1;
      absolute_destination_index = (cpu16->getCurrentDisasmIndex() + destination_index) & 0xfffff;

      if(opco & 0x80)
        {
          absolute_destination_index -= 0x100;
          destination_index = 0x100 - destination_index;
        }
      break;

    case  _PIC17_PROCESSOR_:
      cout << "Which instructions go here?\n";
      break;

    default:
      cout << "ERROR: (Branching) the processor is not defined\n";
      break;
  }
}

char *Branching::name(char *return_str, int len)
{
  snprintf(return_str, len,"%s\t$%c0x%x\t;(0x%x)",
           name_str.c_str(),
           (opcode & 0x80) ? '-' : '+',
           (destination_index & 0x7f)<<1,
           absolute_destination_index<<1);

  return(return_str);
}

//--------------------------------------------------
multi_word_instruction::multi_word_instruction( Processor *pCpu, uint opco, uint address)
                      : Instruction(pCpu,  opco,address),
                        word2_opcode(0),
                        PMaddress(0),
                        PMindex(0),
                        initialized(false)
{
}

multi_word_branch::multi_word_branch( Processor *pCpu, uint opco, uint address)
                 : multi_word_instruction(pCpu,  opco, address),
                   destination_index(0)
{
}
void multi_word_branch::runtime_initialize()
{
    if(cpu16->program_memory[PMindex+1] != &cpu16->bad_instruction)
    {
        word2_opcode = cpu16->program_memory[PMindex+1]->get_opcode();

        if((word2_opcode & 0xf000) != 0xf000)
        {
          cout << "16bit-instructions.cc multiword instruction error\n";
          return;
        }
        // extract the destination address from the two-word opcode
        destination_index = ((word2_opcode & 0xfff)<<8) | (opcode & 0xff);
        initialized = true;
    }
}

char * multi_word_branch::name(char *return_str,int len)
{
  if(!initialized) runtime_initialize();

  snprintf(return_str,len,"%s\t0x%05x", name_str.c_str(), destination_index<<1);
  return(return_str);
}

//---------------------------------------------------------
ADDULNK::ADDULNK( Processor *pCpu, uint opco, const char *pName, uint address)
  : Instruction(pCpu,  opco,address)
{
  m_lit = opcode & 0x3f;
  name_str = pName;
}

char *ADDULNK::name(char *return_str,int len)
{
  snprintf(return_str,len,"%s\t0x%x", name_str.c_str(), m_lit);
  return(return_str);
}

void ADDULNK::execute()
{
  if (cpu16->extended_instruction())
  {
      if (opcode & 0x100)
           cpu16->ind2.put_fsr(cpu16->ind2.get_fsr_value() - m_lit); // SUBULNK
      else cpu16->ind2.put_fsr(cpu16->ind2.get_fsr_value() + m_lit); // ADDULNK
  }
  else
  {
      printf("Error %s extended instruction not supported, check XINST\n", (opcode&0x100)?"SUBULNK":"ADDULNK");
      cpu16->halt();
  }
  cpu16->pc->new_address(cpu16->stack->pop());
}

//---------------------------------------------------------
ADDFSR16::ADDFSR16(Processor *pCpu, uint opco, const char *pName, uint address)
        : Instruction(pCpu,  opco,address)
{
  m_fsr = (opcode>>6)&3;
  m_lit = opcode & 0x3f;
  switch(m_fsr) {
  case 0:
    ia = &cpu16->ind0;
    break;

  case 1:
    ia = &cpu16->ind1;
    break;

  case 2:
    ia = &cpu16->ind2;
    break;

  case 3:
    ia = &cpu16->ind2;
  }
  name_str = pName;
}

char *ADDFSR16::name(char *return_str,int len)
{
  snprintf(return_str,len,"%s\t%u,0x%x", name_str.c_str(), m_fsr, m_lit);
  return(return_str);
}


void ADDFSR16::execute()
{
  if (cpu16->extended_instruction())
  {
      // Apply pending update.
      ia->fsr_value += ia->fsr_delta;
      ia->fsr_delta = 0;

      if (opcode & 0x100)
           ia->put_fsr(ia->get_fsr_value() - m_lit);  //SUBFSR
      else ia->put_fsr(ia->get_fsr_value() + m_lit);  //ADDFSR
  }
  else
  {
      printf("Error %s extended instruction not supported, check XINST\n", (opcode&0x100)?"SUBFSR":"ADDFSR");
      cpu16->halt();
  }
  cpu16->pc->increment();
}

//--------------------------------------------------
void CALLW16::execute()
{
  if (cpu16->extended_instruction())
  {
      if(cpu16->stack->push(cpu16->pc->get_next()))
      {
            cpu16->pcl->put(cpu16->Wget());
            cpu16->pc->increment();
      }
      else cpu16->pc->jump(0); // stack overflow reset
  }
  else
  {
      printf("Error %s extended instruction not supported, check XINST\n", "CALLW");
      cpu16->halt();
  }
}

//--------------------------------------------------
PUSHL::PUSHL(Processor *pCpu, uint opco, uint address)
  :Instruction (pCpu, opco, address),
   m_lit(opco & 0xff)
{
  name_str = "pushl";
}

char *PUSHL::name(char *return_str,int len)
{

  snprintf(return_str,len,"%s\t0x%x",
           name_str.c_str(),m_lit);
  return(return_str);
}

void PUSHL::execute()
{
  if (cpu16->extended_instruction()) cpu16->ind2.postdec.put(m_lit);
  else
  {
      printf("Error %s extended instruction not supported, check XINST\n", "PUSHL");
      cpu16->halt();
  }
  cpu16->pc->increment();
}

//--------------------------------------------------

MOVSF::MOVSF( Processor* pCpu, uint opco, uint address)
  : multi_word_instruction(pCpu, opco,address)
{
  opcode = opco;
  cpu = pCpu;
  PMaddress = cpu16->getCurrentDisasmAddress();
  PMindex   = cpu16->getCurrentDisasmIndex();
  initialized = false;
  destination = 0;
  source = opcode & 0x7f;

  if (opcode & 0x80) name_str = "movss";
  else               name_str = "movsf";
}

void MOVSF::runtime_initialize()
{
  if(cpu_pic->program_memory[PMindex+1])
    {
      word2_opcode = cpu_pic->program_memory[PMindex+1]->get_opcode();

      if((word2_opcode & 0xf000) != 0xf000)
        {
          cout << "16bit-instructions.cc MOVSF error\n";
          return;
        }
      destination = word2_opcode & ((opcode & 0x80) ? 0x7f : 0xfff);
      initialized = true;
    }
}

char *MOVSF::name(char *return_str,int len)
{
  if(!initialized) runtime_initialize();

  if (opcode & 0x80)
    snprintf(return_str,len,"%s\t[0x%x],[0x%x]", name_str.c_str(), source, destination);
  else
    snprintf(return_str,len,"%s\t[0x%x],%s", name_str.c_str(), source, cpu_pic->registers[destination]->name_str.c_str());

  return(return_str);
}


void MOVSF::execute()
{
  if (cpu16->extended_instruction())
  {
      if(!initialized) runtime_initialize();

      uint source_addr = cpu16->ind2.plusk_fsr_value(source);

      uint r =  cpu_pic->registers[source_addr]->get();
      cpu16->pc->skip();

      uint destination_addr = (opcode & 0x80) ?
              cpu16->ind2.plusk_fsr_value(destination) : destination;

      cpu_pic->registers[destination_addr]->put(r);
  }
  else
  {
      printf("Error %s extended instruction not supported, check XINST\n", (opcode & 0x80)?"MOVSS":"MOVSF");
      cpu16->halt();
  }
}

//--------------------------------------------------
void ADDLW16::execute()
{
  uint old_value,new_value;

  new_value = (old_value = cpu16->Wget()) + L;

  cpu16->Wput(new_value & 0xff);
  cpu16->status->put_Z_C_DC_OV_N(new_value, old_value, L);

  cpu16->pc->increment();
}

//--------------------------------------------------
void ADDWF16::execute()
{
    uint new_value,src_value,w_value;

    if( access ) source = cpu_pic->register_bank[register_address];
    else if (cpu16->extended_instruction() && register_address < 0x60)
      source = cpu_pic->registers[register_address + cpu16->ind2.fsr_value];
    else source = cpu_pic->registers[register_address];

    new_value = (src_value = source->get_value()) + (w_value = cpu16->Wget());

    if(destination) // Store the result
    {
      source->put(new_value & 0xff);      // Result goes to source
      cpu16->status->put_Z_C_DC_OV_N(new_value, src_value, w_value);
    }
    else
    {
      cpu16->Wput(new_value & 0xff);
      cpu16->status->put_Z_C_DC_OV_N(new_value, w_value, src_value);
    }
    cpu16->pc->increment();
}

//--------------------------------------------------

void ADDWFC16::execute()
{
  uint new_value,src_value,w_value;

  if( access ) source = cpu_pic->register_bank[register_address];
  else if (cpu16->extended_instruction() && register_address < 0x60)
      source = cpu_pic->registers[register_address + cpu16->ind2.fsr_value];
  else source = cpu_pic->registers[register_address];

  new_value = (src_value = source->get()) + (w_value = cpu16->Wget()) +
    ((cpu16->status->value.get() & STATUS_C) ? 1 : 0);

  // Store the result
  if(destination) source->put(new_value & 0xff);      // Result goes to source
  else            cpu16->Wput(new_value & 0xff);

  cpu16->status->put_Z_C_DC_OV_N(new_value, src_value, w_value);
  cpu16->pc->increment();
}

//--------------------------------------------------

void ANDLW16::execute()
{
    uint new_value;

    new_value = cpu16->Wget() & L;

    cpu16->Wput(new_value);
    cpu16->status->put_N_Z(new_value);

    cpu16->pc->increment();
}

//--------------------------------------------------

void ANDWF16::execute()
{
  uint new_value;

  if( access ) source = cpu_pic->register_bank[register_address];
  else if (cpu16->extended_instruction() && register_address < 0x60)
      source = cpu_pic->registers[register_address + cpu16->ind2.fsr_value];
  else source = cpu_pic->registers[register_address];

  new_value = source->get() & cpu16->Wget();

  if(destination) source->put(new_value);      // Result goes to source
  else            cpu16->Wput(new_value);

  cpu16->status->put_N_Z(new_value);
  cpu16->pc->increment();
}

//--------------------------------------------------

BC::BC( Processor* pCpu, uint opco, uint address)
  : Branching(pCpu, opco, address)
{
    decode( pCpu, opco );
    name_str = "bc";
}

void BC::execute()
{
  if(cpu16->status->value.get() & STATUS_C)
       cpu16->pc->jump(absolute_destination_index);
  else cpu16->pc->increment();
}

//--------------------------------------------------

BN::BN( Processor* pCpu, uint opco, uint address)
  : Branching(pCpu, opco, address)
{
    decode( pCpu, opco );
    name_str = "bn";
}

void BN::execute()
{
  if(cpu16->status->value.get() & STATUS_N)
       cpu16->pc->jump(absolute_destination_index);
  else cpu16->pc->increment();
}

//--------------------------------------------------
BNC::BNC( Processor* pCpu, uint opco, uint address)
  : Branching(pCpu, opco, address)
{
    decode( pCpu, opco );
    name_str ="bnc";
}

void BNC::execute()
{
  if(cpu16->status->value.get() & STATUS_C)
       cpu16->pc->increment();
  else cpu16->pc->jump(absolute_destination_index);
}

//--------------------------------------------------
BNN::BNN( Processor* pCpu, uint opco, uint address)
  : Branching(pCpu, opco, address)
{
  decode( pCpu, opco );
  name_str ="bnn";
}

void BNN::execute()
{
  if(cpu16->status->value.get() & STATUS_N)
       cpu16->pc->increment();
  else cpu16->pc->jump(absolute_destination_index);
}

//--------------------------------------------------
BNOV::BNOV( Processor* pCpu, uint opco, uint address)
  : Branching(pCpu, opco, address)
{
  decode( pCpu, opco );
  name_str ="bnov";
}

void BNOV::execute()
{
  if(cpu16->status->value.get() & STATUS_OV)
       cpu16->pc->increment();
  else cpu16->pc->jump(absolute_destination_index);
}

//--------------------------------------------------
BNZ::BNZ( Processor* pCpu, uint opco, uint address)
  : Branching(pCpu, opco, address)
{
  decode( pCpu, opco );
  name_str ="bnz";
}

void BNZ::execute()
{
  if(cpu16->status->value.get() & STATUS_Z)
       cpu16->pc->increment();
  else cpu16->pc->jump(absolute_destination_index);
}

//--------------------------------------------------
BOV::BOV( Processor* pCpu, uint opco, uint address)
  : Branching(pCpu, opco, address)
{
  decode( pCpu, opco );
  name_str = "bov";
}

void BOV::execute()
{
  if(cpu16->status->value.get() & STATUS_OV)
       cpu16->pc->jump(absolute_destination_index);
  else cpu16->pc->increment();
}

//--------------------------------------------------
BRA16::BRA16( Processor* pCpu, uint opco, uint address)
     : Instruction(pCpu, opco, address)
{
    destination_index = (opco & 0x7ff)+1;
    absolute_destination_index = (cpu16->getCurrentDisasmIndex() + destination_index) & 0xfffff;

    if(opco & 0x400)
    {
      absolute_destination_index -= 0x800;
      destination_index = 0x800 - destination_index;
    }
    name_str ="bra";
}

void BRA16::execute()
{
  cpu16->pc->jump(absolute_destination_index);

}

char* BRA16::name(char *return_str, int len)
{
  snprintf(return_str, len, "%s\t$%c0x%x\t;(0x%05x)",
          name_str.c_str(),
          (opcode & 0x400) ? '-' : '+',
          (destination_index & 0x7ff)<<1,
          absolute_destination_index<<1);

  return return_str;
}

//--------------------------------------------------
void BSF16::execute()
{
  if( access ) reg = cpu_pic->register_bank[register_address];
  else if (cpu16->extended_instruction() && register_address < 0x60)
       reg = cpu_pic->registers[register_address + cpu16->ind2.fsr_value];
  else reg = cpu_pic->registers[register_address];

  reg->put(reg->get_value() | mask);    // Must not use reg->value.get() as it breaks indirects

  cpu16->pc->increment();
}

//--------------------------------------------------
void BCF16::execute()
{
  if( access ) reg = cpu_pic->register_bank[register_address];
  else if (cpu16->extended_instruction() && register_address < 0x60)
       reg = cpu_pic->registers[register_address + cpu16->ind2.fsr_value];
  else reg = cpu_pic->registers[register_address];

  reg->put(reg->get_value() & mask);    // Must not use reg->value.get() as it breaks indirects

  cpu16->pc->increment();
}

//--------------------------------------------------
void BTFSC16::execute()
{
  if( access ) reg = cpu_pic->register_bank[register_address];
  else if (cpu16->extended_instruction() && register_address < 0x60)
       reg = cpu_pic->registers[register_address + cpu16->ind2.fsr_value];
  else reg = cpu_pic->registers[register_address];

  uint result = mask & reg->get();

  if(!result) cpu_pic->pc->skip();       // Skip next instruction
  else        cpu_pic->pc->increment();
}

//--------------------------------------------------
void BTFSS16::execute()
{
  if( access ) reg = cpu_pic->register_bank[register_address];
  else if (cpu16->extended_instruction() && register_address < 0x60)
       reg = cpu_pic->registers[register_address + cpu16->ind2.fsr_value];
  else reg = cpu_pic->registers[register_address];

  uint result = mask & reg->get();

  if(result) cpu_pic->pc->skip();                  // Skip next instruction
  else cpu_pic->pc->increment();
}

//--------------------------------------------------
BTG::BTG( Processor* pCpu, uint opco, uint address)
  : Bit_op(pCpu, opco,address)
{
  decode( pCpu, opco );
  name_str ="btg";
}

void BTG::execute()
{
  if( access ) reg = cpu_pic->register_bank[register_address];
  else if (cpu16->extended_instruction() && register_address < 0x60)
       reg = cpu_pic->registers[register_address + cpu16->ind2.fsr_value];
  else reg = cpu_pic->registers[register_address];

  reg->put(reg->get() ^ mask);

  cpu16->pc->increment();
}

//--------------------------------------------------
BZ::BZ( Processor* pCpu, uint opco, uint address)
  : Branching(pCpu, opco, address)
{
  decode( pCpu, opco );
  name_str ="bz";
}

void BZ::execute()
{
  if(cpu16->status->value.get() & STATUS_Z)
       cpu16->pc->jump(absolute_destination_index);
  else cpu16->pc->increment();
}

//--------------------------------------------------
CALL16::CALL16( Processor* pCpu, uint opco, uint address)
  : multi_word_branch(pCpu, opco, address)
{
  fast = (opco & 0x100) ? true : false;
  cpu = pCpu;
  PMaddress = cpu16->getCurrentDisasmAddress();
  PMindex = cpu16->getCurrentDisasmIndex();
  initialized = false;

  name_str ="call";
}

void CALL16::execute()
{
  if(!initialized) runtime_initialize();

  if (cpu16->stack->push(cpu16->pc->get_next()))
  {
    if(fast) cpu16->fast_stack.push();
    cpu16->pc->jump(destination_index);
  }
  else cpu16->pc->jump(0);   // stack overflow reset
}

char *CALL16::name(char  *return_str,int len)
{
  if(!initialized) runtime_initialize();

  snprintf(return_str,len,"call\t0x%05x%s", destination_index<<1, ((fast) ? ",f" : " "));

  return(return_str);
}

//--------------------------------------------------
void COMF16::execute()
{
  uint new_value;

  if( access ) source = cpu_pic->register_bank[register_address];
  else if (cpu16->extended_instruction() && register_address < 0x60)
       source = cpu_pic->registers[register_address + cpu16->ind2.fsr_value];
  else source = cpu_pic->registers[register_address];

  new_value = source->get() ^ 0xff;

  if(destination) source->put(new_value);      // Result goes to source
  else            cpu16->Wput(new_value);

  cpu16->status->put_N_Z(new_value);
  cpu16->pc->increment();
}

//--------------------------------------------------
CPFSEQ::CPFSEQ( Processor* pCpu, uint opco, uint address)
  : Register_op(pCpu, opco, address)
{
  decode( pCpu, opco );
  name_str = "cpfseq";
}

void CPFSEQ::execute()
{
  if( access ) source = cpu_pic->register_bank[register_address];
  else if (cpu16->extended_instruction() && register_address < 0x60)
      source = cpu_pic->registers[register_address + cpu16->ind2.fsr_value];
  else
      source = cpu_pic->registers[register_address];

  if(source->get() == cpu16->Wget())
    cpu16->pc->skip();                  // Skip next instruction
  else
    cpu16->pc->increment();
}

//--------------------------------------------------
CPFSGT::CPFSGT( Processor* pCpu, uint opco, uint address)
  : Register_op(pCpu, opco, address)
{
  decode( pCpu, opco );
  name_str = "cpfsgt";
}

void CPFSGT::execute()
{
  if( access )
      source = cpu_pic->register_bank[register_address];
  else if (cpu16->extended_instruction() && register_address < 0x60)
      source = cpu_pic->registers[register_address + cpu16->ind2.fsr_value];
  else
      source = cpu_pic->registers[register_address];

  if(source->get() > cpu16->Wget())
    cpu16->pc->skip();                  // Skip next instruction
  else
    cpu16->pc->increment();
}

//--------------------------------------------------
CPFSLT::CPFSLT( Processor* pCpu, uint opco, uint address)
  : Register_op(pCpu, opco, address)
{
  decode( pCpu, opco );
  name_str = "cpfslt";
}

void CPFSLT::execute()
{
  if( access ) source = cpu_pic->register_bank[register_address];
  else if (cpu16->extended_instruction() && register_address < 0x60)
      source = cpu_pic->registers[register_address + cpu16->ind2.fsr_value];
  else
      source = cpu_pic->registers[register_address];

  if(source->get() < cpu16->Wget())
    cpu16->pc->skip();                  // Skip next instruction
  else
    cpu16->pc->increment();
}

void CLRF16::execute()
{
  if( access )
      cpu_pic->register_bank[register_address]->put(0);
  else if (cpu16->extended_instruction() && register_address < 0x60)
      cpu_pic->registers[register_address + cpu16->ind2.fsr_value]->put(0);
  else
      cpu_pic->registers[register_address]->put(0);

  cpu16->status->put_Z(1);

  cpu16->pc->increment();
}

//--------------------------------------------------
DAW::DAW( Processor* pCpu, uint opco, uint address)
  : Instruction(pCpu, opco, address)
{
  decode( pCpu, opco );
  name_str = "daw";
}

void DAW::execute()
{
  uint new_value;

  new_value = cpu16->Wget();
  if(((new_value & 0x0f) > 0x9) || (cpu16->status->value.get() & STATUS_DC))
    new_value += 0x6;

  if(((new_value & 0xf0) > 0x90) || (cpu16->status->value.get() & STATUS_C))
    new_value += 0x60;

  cpu16->Wput(new_value & 0xff);
  if ( new_value>0xff )
      cpu16->status->put_C(1);
  else if ( cpu16->bugs() & BUG_DAW )
      cpu16->status->put_C(0);

  cpu16->pc->increment();

}
//--------------------------------------------------

void DECF16::execute()
{

  if( access )
      source = cpu_pic->register_bank[register_address];
  else if (cpu16->extended_instruction() && register_address < 0x60)
      source = cpu_pic->registers[register_address + cpu16->ind2.fsr_value];
  else
      source = cpu_pic->registers[register_address];

  uint src_value = source->get();
  uint new_value = src_value - 1;

  if(destination)
    source->put(new_value & 0xff);      // Result goes to source
  else
    cpu16->Wput(new_value & 0xff);

  //  cpu16->status->put_N_Z(new_value);
  cpu16->status->put_Z_C_DC_OV_N_for_sub(new_value,src_value,1);

  cpu16->pc->increment();

}
//--------------------------------------------------

void DECFSZ16::execute()
{
  uint new_value;

  if( access )
      source = cpu_pic->register_bank[register_address];
  else if (cpu16->extended_instruction() && register_address < 0x60)
      source = cpu_pic->registers[register_address + cpu16->ind2.fsr_value];
  else
      source = cpu_pic->registers[register_address];

  new_value = (source->get() - 1)&0xff;

  if(destination)
    source->put(new_value);      // Result goes to source
  else
    cpu16->Wput(new_value);

  if(0==new_value)
    cpu16->pc->skip();                  // Skip next instruction
  else
    cpu16->pc->increment();

}
//--------------------------------------------------

DCFSNZ::DCFSNZ( Processor* pCpu, uint opco, uint address)
  : Register_op(pCpu, opco, address)
{
  decode( pCpu, opco );
  name_str = "dcfsnz";
}

void DCFSNZ::execute()
{
  uint new_value;

  if( access )
      source = cpu_pic->register_bank[register_address];
  else if (cpu16->extended_instruction() && register_address < 0x60)
      source = cpu_pic->registers[register_address + cpu16->ind2.fsr_value];
  else
      source = cpu_pic->registers[register_address];

  new_value = (source->get() - 1)&0xff;

  if(destination)
    source->put(new_value);      // Result goes to source
  else
    cpu16->Wput(new_value);

  if(0!=new_value)
    cpu16->pc->skip();                  // Skip next instruction
  else
    cpu16->pc->increment();

}
//--------------------------------------------------

GOTO16::GOTO16( Processor* pCpu, uint opco, uint address)
  : multi_word_branch(pCpu, opco, address)
{
  PMaddress = cpu16->getCurrentDisasmAddress();
  PMindex   = cpu16->getCurrentDisasmIndex();
  initialized = false;

  name_str = "goto";
}

void GOTO16::execute()
{
  if(!initialized)
    runtime_initialize();

  cpu16->pc->jump(destination_index);

}
//--------------------------------------------------

void INCF16::execute()
{
  uint new_value, src_value;


  if( access )
      source = cpu_pic->register_bank[register_address];
  else if (cpu16->extended_instruction() && register_address < 0x60)
      source = cpu_pic->registers[register_address + cpu16->ind2.fsr_value];
  else
      source = cpu_pic->registers[register_address];

  src_value = source->get();
  new_value = (src_value + 1);

  if(destination)
    {
      source->put(new_value & 0xff);      // Result goes to source
      cpu16->status->put_Z_C_DC_OV_N(new_value, src_value, 1);
    }
  else
    {
      cpu16->Wput(new_value & 0xff);
      cpu16->status->put_Z_C_DC_OV_N(new_value, 1, src_value);
    }

  cpu16->pc->increment();

}
//--------------------------------------------------

void INCFSZ16::execute()
{
  uint new_value;

  if( access )
      source = cpu_pic->register_bank[register_address];
  else if (cpu16->extended_instruction() && register_address < 0x60)
      source = cpu_pic->registers[register_address + cpu16->ind2.fsr_value];
  else
      source = cpu_pic->registers[register_address];

  new_value = (source->get() + 1)&0xff;

  if(destination)
    source->put(new_value);      // Result goes to source
  else
    cpu16->Wput(new_value);

  if(0==new_value)
    cpu16->pc->skip();                  // Skip next instruction
  else
    cpu16->pc->increment();

}
//--------------------------------------------------

INFSNZ::INFSNZ( Processor* pCpu, uint opco, uint address)
  : Register_op(pCpu, opco, address)
{
  decode( pCpu, opco );
  name_str = "infsnz";
}

void INFSNZ::execute()
{
  uint new_value;

  if( access )
      source = cpu_pic->register_bank[register_address];
  else if (cpu16->extended_instruction() && register_address < 0x60)
      source = cpu_pic->registers[register_address + cpu16->ind2.fsr_value];
  else
      source = cpu_pic->registers[register_address];

  new_value = (source->get() + 1)&0xff;

  if(destination)
    source->put(new_value);      // Result goes to source
  else
    cpu16->Wput(new_value);

  if(0!=new_value)
    cpu16->pc->skip();                  // Skip next instruction
  else
    cpu16->pc->increment();
}
//--------------------------------------------------

void IORLW16::execute()
{
  uint new_value;

  new_value = cpu16->Wget() | L;

  cpu16->Wput(new_value);
  cpu16->status->put_N_Z(new_value);

  cpu16->pc->increment();
}
//--------------------------------------------------

void IORWF16::execute()
{
  uint new_value;

  if( access )
      source = cpu_pic->register_bank[register_address];
  else if (cpu16->extended_instruction() && register_address < 0x60)
      source = cpu_pic->registers[register_address + cpu16->ind2.fsr_value];
  else
      source = cpu_pic->registers[register_address];

  new_value = source->get() | cpu16->Wget();

  if(destination)
    source->put(new_value);      // Result goes to source
  else
    cpu16->Wput(new_value);

  cpu16->status->put_N_Z(new_value);

  cpu16->pc->increment();
}
//--------------------------------------------------
LCALL16::LCALL16( Processor* pCpu, uint opco, uint address)
  : multi_word_branch(pCpu, opco, address)
{
//    opcode = opco;
//    fast = opco & 0x100;
//    cpu = pCpu;
//    address = cpu16->current_disasm_address;
//    initialized = 0;

  name_str = "lcall";
}

void LCALL16::execute()
{

//    if(!initialized)
//      runtime_initialize();

//    cpu16->stack->push(cpu16->pc->get_next());
//    if(fast)
//      cpu16->fast_stack.push();

//    cpu16->pc->jump(destination);

}

char *LCALL16::name(char  *return_str,int len)
{
//    if(!initialized)
//      runtime_initialize();

  snprintf(return_str,len,"lcall\t0x%05x%s", destination_index<<1, ((fast) ? ",f" : " "));

  return(return_str);
}
//--------------------------------------------------

LFSR::LFSR( Processor* pCpu, uint opco, uint address)
  : multi_word_instruction(pCpu, opco, address)
{

  PMaddress = cpu16->getCurrentDisasmAddress();
  PMindex   = cpu16->getCurrentDisasmIndex();
  initialized = false;

  fsr = (opcode & 0x30)>>4;
  switch(fsr)
    {
    case 0:
      ia = &cpu16->ind0;
      break;

    case 1:
      ia = &cpu16->ind1;
      break;

    case 2:
      ia = &cpu16->ind2;
      break;

    case 3:
      cout << "LFSR decode error, fsr is 3 and should only be 0,1, or 2\n";
      ia = &cpu16->ind0;
    }

  name_str = "lfsr";
}

void LFSR::runtime_initialize()
{
  if(cpu_pic->program_memory[PMindex+1])
    {
      word2_opcode = cpu_pic->program_memory[PMindex+1]->get_opcode();

      if((word2_opcode & 0xff00) != 0xf000)
        {
          cout << "16bit-instructions.cc LFSR error\n";
          return;
        }
      k = ( (opcode & 0xf)<<8) | (word2_opcode & 0xff);
      initialized = true;
    }
}

char *LFSR::name(char *return_str,int len)
{
  if(!initialized) runtime_initialize();

  snprintf(return_str,len,"%s\t%u,0x%x",
           name_str.c_str(),
           fsr, k);

  return(return_str);
}

void LFSR::execute()
{
  if(!initialized) runtime_initialize();

  ia->put_fsr(k);

  cpu16->pc->skip();

}
//--------------------------------------------------

void MOVF16::execute()
{
  uint source_value;

  if( access ) source = cpu_pic->register_bank[register_address];

  else if (cpu16->extended_instruction() && register_address < 0x60)
      source = cpu_pic->registers[register_address + cpu16->ind2.fsr_value];
  else
      source = cpu_pic->registers[register_address];

  source_value = source->get();

  // Store the result

  if(destination) source->put(source_value);
  else            cpu16->Wput(source_value);

  cpu16->status->put_N_Z(source_value);
  cpu16->pc->increment();
}

//--------------------------------------------------

MOVFF::MOVFF( Processor* pCpu, uint opco, uint address)
  : multi_word_instruction(pCpu, opco, address)
{
  PMaddress = cpu16->getCurrentDisasmAddress();
  PMindex   = cpu16->getCurrentDisasmIndex();
  initialized = false;
  destination = 0;
  source = opcode & 0xfff;

  name_str = "movff";
}

void MOVFF::runtime_initialize()
{
  if(cpu_pic->program_memory[PMindex+1])
    {
      word2_opcode = cpu_pic->program_memory[PMindex+1]->get_opcode();

      if((word2_opcode & 0xf000) != 0xf000)
        {
          cout << "16bit-instructions.cc MOVFF error\n";
          return;
        }
      destination = word2_opcode & 0xfff;
      initialized = true;
    }
}

char *MOVFF::name(char *return_str,int len)
{
  if(!initialized) runtime_initialize();

  snprintf(return_str,len,"%s\t%s,%s", name_str.c_str(),
           cpu_pic->registers[source]->name_str.c_str(),
           cpu_pic->registers[destination]->name_str.c_str());

  return(return_str);
}

void MOVFF::execute()
{
  if(!initialized) runtime_initialize();

  uint r =  cpu_pic->registers[source]->get();

  cpu_pic->registers[destination]->put(r);

  cpu16->pc->skip();
}
//--------------------------------------------------

MOVFP::MOVFP( Processor* pCpu, uint opco, uint address)
  : multi_word_instruction(pCpu, opco, address)
{
  name_str = "movfp";
}

void MOVFP::runtime_initialize()
{
}

char *MOVFP::name(char *return_str, int len)
{
  snprintf(return_str,len,"%s\t%s,%s",
           name_str.c_str(),
           cpu_pic->registers[source]->name_str.c_str(),
           cpu_pic->registers[destination]->name_str.c_str());
  return(return_str);
}

void MOVFP::execute()
{
}
//--------------------------------------------------

MOVLB16::MOVLB16( Processor* pCpu, uint opco, uint address)
  : Literal_op(pCpu, opco, address)
{
  decode( pCpu, opco );
  name_str = "movlb";
}

void MOVLB16::execute()
{
  cpu16->registers[cpu16->bsr.address]->put(L);
  cpu16->pc->increment();
}
//--------------------------------------------------

MOVLR::MOVLR( Processor* pCpu, uint opco, uint address)
  : Literal_op(pCpu, opco, address)
{
  name_str = "movlr";
}

void MOVLR::execute()
{
}
//--------------------------------------------------

MOVPF::MOVPF( Processor* pCpu, uint opco,uint address)
  : multi_word_instruction(pCpu, opco,address)
{
  name_str = "movpf";
}

void MOVPF::runtime_initialize()
{
}

char *MOVPF::name(char *return_str,int len)
{
  snprintf(return_str,len,"%s\t%s,%s", name_str.c_str(),
           cpu_pic->registers[source]->name_str.c_str(),
           cpu_pic->registers[destination]->name_str.c_str());

  return(return_str);
}

void MOVPF::execute()
{
}
//--------------------------------------------------

MOVWF16::MOVWF16(Processor *pCpu, uint opco, uint address)
  : MOVWF(pCpu,opco, address)
{
}

void MOVWF16::execute()
{
  if( access )
      source = cpu_pic->register_bank[register_address];
  else if (cpu16->extended_instruction() && register_address < 0x60)
      source = cpu_pic->registers[register_address + cpu16->ind2.fsr_value];
  else
      source = cpu_pic->registers[register_address];

  source->put(cpu16->Wget());

  cpu16->pc->increment();
}
//--------------------------------------------------

MULLW::MULLW( Processor* pCpu, uint opco, uint address)
  : Literal_op(pCpu, opco,address)
{
  decode( pCpu, opco );

  name_str = "mullw";
}

void MULLW::execute()
{
  uint value;

  value = (0xff & cpu16->Wget()) * L;

  cpu16->prodl.put(value &0xff);
  cpu16->prodh.put((value>>8) &0xff);

  cpu16->pc->increment();
}
//--------------------------------------------------

MULWF::MULWF( Processor* pCpu, uint opco, uint address)
  : Register_op(pCpu, opco, address)
{
  decode( pCpu, opco );
  name_str = "mulwf";
}

void MULWF::execute()
{
  uint value;

  if( access )
      source = cpu_pic->register_bank[register_address];
  else if (cpu16->extended_instruction() && register_address < 0x60)
      source = cpu_pic->registers[register_address + cpu16->ind2.fsr_value];
  else
      source = cpu_pic->registers[register_address];

  //It's not necessary to '&' the get()'s with 0xff, but it doesn't
  //hurt either.
  value = (0xff & cpu16->Wget()) * (0xff & source->get());

  cpu16->prodl.put(value &0xff);
  cpu16->prodh.put((value>>8) &0xff);

  cpu16->pc->increment();
}
//--------------------------------------------------

NEGF::NEGF( Processor* pCpu, uint opco, uint address)
  : Register_op(pCpu, opco, address)
{
  decode( pCpu, opco );
  name_str = "negf";
}

void NEGF::execute()
{
  uint new_value,src_value;

  if( access )
      source = cpu_pic->register_bank[register_address];
  else if (cpu16->extended_instruction() && register_address < 0x60)
      source = cpu_pic->registers[register_address + cpu16->ind2.fsr_value];
  else
      source = cpu_pic->registers[register_address];

  src_value = source->get();
  new_value = 1 + ~src_value;        // two's complement

  source->put(new_value&0xff);

  cpu16->status->put_Z_C_DC_OV_N_for_sub(new_value,0,src_value);

  cpu16->pc->increment();
}
//--------------------------------------------------

NEGW::NEGW( Processor* pCpu, uint opco, uint address)
  : Register_op(pCpu, opco,address)
{
    name_str = "negw";
}

void NEGW::execute()
{
    cout << "negw is not implemented???";
}
//--------------------------------------------------

POP::POP( Processor* pCpu, uint opco, uint address)
   : Instruction(pCpu, opco, address)
{
  decode( pCpu, opco );
  name_str = "pop";
}

void POP::execute()
{
  cpu16->stack->pop();  // discard TOS
  cpu16->pc->increment();
}
//--------------------------------------------------

PUSH::PUSH( Processor* pCpu, uint opco, uint address)
    : Instruction(pCpu, opco, address)
{
  decode( pCpu, opco );
  name_str = "push";
}

void PUSH::execute()
{

  if (cpu16->stack->push(cpu16->pc->get_next()))
      cpu16->pc->increment();
  else        // stack overflow reset
      cpu16->pc->jump(0);
}

//--------------------------------------------------
RCALL::RCALL( Processor* pCpu, uint opco, uint address)
     : Instruction(pCpu, opco, address)
{
  destination_index = (opco & 0x7ff)+1;
  if(opco & 0x400)
    destination_index -= 0x800;

  absolute_destination_index = (cpu16->getCurrentDisasmIndex() + destination_index) & 0xfffff;

  name_str = "rcall";
}

void RCALL::execute()
{
  if(cpu16->stack->push(cpu16->pc->get_next()))
      cpu16->pc->jump(absolute_destination_index);
  else        // stack overflow reset
      cpu16->pc->jump(0);
}

char * RCALL::name(char *return_str,int len)
{
  snprintf(return_str,len,"%s\t$%c0x%x\t;(0x%05x)", name_str.c_str(),
           (destination_index < 0) ? '-' : '+',
           (destination_index & 0x7ff)<<1,
           absolute_destination_index<<1);

  return(return_str);
}

void RETFIE16::execute()
{
  cpu16->pc->new_address(cpu16->stack->pop());
  if(fast) cpu16->fast_stack.pop();
  //cout << "retfie: need to enable interrupts\n";

  cpu16->intcon.in_interrupt = false;
  cpu16->intcon.put_value(cpu16->intcon.value.get()); //test for new interrupts
}

char *RETFIE16::name(char  *return_str,int len)
{
  if(fast) snprintf(return_str,len,"retfie\tfast");
  else     snprintf(return_str,len,"retfie");

  return(return_str);
}

void RETURN16::execute()
{
  cpu16->pc->new_address(cpu16->stack->pop());
  if(fast) cpu16->fast_stack.pop();
}

char *RETURN16::name(char  *return_str,int len)
{
  if(fast) snprintf(return_str,len,"return\tfast");
  else     snprintf(return_str,len,"return");

  return(return_str);
}

RLCF::RLCF( Processor* pCpu, uint opco, uint address)
  : Register_op(pCpu, opco, address)
{
  decode( pCpu, opco );
  name_str = "rlcf";
}

void RLCF::execute()
{
  uint new_value;

  if( access )
      source = cpu_pic->register_bank[register_address];
  else if (cpu16->extended_instruction() && register_address < 0x60)
      source = cpu_pic->registers[register_address + cpu16->ind2.fsr_value];
  else
      source = cpu_pic->registers[register_address];

  new_value = (source->get() << 1) | cpu16->status->get_C();

  if(destination) source->put(new_value&0xff);      // Result goes to source
  else            cpu16->Wput(new_value&0xff);

  cpu16->status->put_Z_C_N(new_value);
  cpu16->pc->increment();
}

RLNCF::RLNCF( Processor* pCpu, uint opco, uint address)
  : Register_op(pCpu, opco, address)
{
  decode( pCpu, opco );
  name_str = "rlncf";
}

void RLNCF::execute()
{
  uint new_value,src_value;

  if( access )
      source = cpu_pic->register_bank[register_address];
  else if (cpu16->extended_instruction() && register_address < 0x60)
      source = cpu_pic->registers[register_address + cpu16->ind2.fsr_value];
  else
      source = cpu_pic->registers[register_address];

  src_value = source->get();
  new_value = (src_value << 1) | ( (src_value & 0x80) ? 1 : 0);

  if(destination) source->put(new_value&0xff);      // Result goes to source
  else            cpu16->Wput(new_value&0xff);

  cpu16->status->put_N_Z(new_value);
  cpu16->pc->increment();
}
//--------------------------------------------------

RRCF::RRCF( Processor* pCpu, uint opco, uint address)
  : Register_op(pCpu, opco, address)
{
  decode( pCpu, opco );
  name_str = "rrcf";
}

void RRCF::execute()
{
  uint new_value,src_value;

  if( access )
      source = cpu_pic->register_bank[register_address];
  else if (cpu16->extended_instruction() && register_address < 0x60)
      source = cpu_pic->registers[register_address + cpu16->ind2.fsr_value];
  else
      source = cpu_pic->registers[register_address];

  src_value = source->get() & 0xff;
  new_value = (src_value >> 1) | (cpu16->status->get_C() ? 0x80 : 0);

  if(destination) source->put(new_value&0xff);      // Result goes to source
  else            cpu16->Wput(new_value&0xff);

  cpu16->status->put_Z_C_N(new_value | ((src_value & 1) ? 0x100 : 0) );
  cpu16->pc->increment();
}
//--------------------------------------------------

RRNCF::RRNCF( Processor* pCpu, uint opco, uint address)
  : Register_op(pCpu, opco, address)
{
  decode( pCpu, opco );
  name_str = "rrncf";
}

void RRNCF::execute()
{
  uint new_value,src_value;

  if( access )
      source = cpu_pic->register_bank[register_address];
  else if (cpu16->extended_instruction() && register_address < 0x60)
      source = cpu_pic->registers[register_address + cpu16->ind2.fsr_value];
  else
      source = cpu_pic->registers[register_address];

  src_value = source->get() & 0xff;
  new_value = (src_value >> 1) | ( (src_value & 1) ? 0x80 : 0);

  if(destination) source->put(new_value&0xff);      // Result goes to source
  else            cpu16->Wput(new_value&0xff);

  cpu16->status->put_N_Z(new_value | ((src_value & 1) ? 0x100 : 0) );
  cpu16->pc->increment();
}
//--------------------------------------------------

SETF::SETF( Processor* pCpu, uint opco, uint address)
  : Register_op(pCpu, opco, address)
{
  decode( pCpu, opco );
  name_str = "setf";
}

void SETF::execute()
{
  if( access )
      source = cpu_pic->register_bank[register_address];
  else if (cpu16->extended_instruction() && register_address < 0x60)
      source = cpu_pic->registers[register_address + cpu16->ind2.fsr_value];
  else
      source = cpu_pic->registers[register_address];

  source->put(0xff);

  cpu16->pc->increment();
}
//--------------------------------------------------

void SLEEP16::execute()
{
  cpu_pic->enter_sleep();
}
//--------------------------------------------------

void SUBLW16::execute()
{
  uint new_value,old_value;

  new_value = L - (old_value = cpu16->Wget());

  cpu16->Wput(new_value & 0xff);

  cpu16->status->put_Z_C_DC_OV_N_for_sub(new_value, L, old_value);

  cpu16->pc->increment();
}
//--------------------------------------------------

SUBFWB::SUBFWB( Processor* pCpu, uint opco, uint address)
  : Register_op(pCpu, opco, address)
{
  decode( pCpu, opco );
  name_str = "subfwb";
}

void SUBFWB::execute()
{
  uint new_value,src_value,w_value;

  if( access ) source = cpu_pic->register_bank[register_address];
  else if (cpu16->extended_instruction() && register_address < 0x60)
      source = cpu_pic->registers[register_address + cpu16->ind2.fsr_value];
  else
      source = cpu_pic->registers[register_address];

  new_value = (w_value = cpu16->Wget()) - (src_value = source->get()) -
    (1 - cpu16->status->get_C());

  if(destination) source->put(new_value & 0xff);
  else            cpu16->Wput(new_value & 0xff);

  cpu16->status->put_Z_C_DC_OV_N_for_sub(new_value, w_value, src_value);
  cpu16->pc->increment();
}

//--------------------------------------------------

void SUBWF16::execute()
{
  uint new_value,src_value,w_value;

  if( access )
      source = cpu_pic->register_bank[register_address];
  else if (cpu16->extended_instruction() && register_address < 0x60)
      source = cpu_pic->registers[register_address + cpu16->ind2.fsr_value];
  else
      source = cpu_pic->registers[register_address];

  new_value = (src_value = source->get()) - (w_value = cpu16->Wget());

  if(destination) source->put(new_value & 0xff);
  else            cpu16->Wput(new_value & 0xff);

  cpu16->status->put_Z_C_DC_OV_N_for_sub(new_value, src_value, w_value);
  cpu16->pc->increment();
}
//--------------------------------------------------

void SUBWFB16::execute()
{
  uint new_value,src_value,w_value;

  if( access )
      source = cpu_pic->register_bank[register_address];
  else if (cpu16->extended_instruction() && register_address < 0x60)
      source = cpu_pic->registers[register_address + cpu16->ind2.fsr_value];
  else
      source = cpu_pic->registers[register_address];

  new_value = (src_value = source->get()) - (w_value = cpu16->Wget()) -
    (1 - cpu16->status->get_C());

  if(destination) source->put(new_value & 0xff);
  else            cpu16->Wput(new_value & 0xff);

  cpu16->status->put_Z_C_DC_OV_N_for_sub(new_value, src_value, w_value);
  cpu16->pc->increment();
}
//--------------------------------------------------

void SWAPF16::execute()
{
  uint src_value;

  if( access )
      source = cpu_pic->register_bank[register_address];
  else if (cpu16->extended_instruction() && register_address < 0x60)
      source = cpu_pic->registers[register_address + cpu16->ind2.fsr_value];
  else
      source = cpu_pic->registers[register_address];

  src_value = source->get();

  if(destination) source->put( ((src_value >> 4) & 0x0f) | ( (src_value << 4) & 0xf0) );
  else            cpu_pic->Wput( ((src_value >> 4) & 0x0f) | ( (src_value << 4) & 0xf0) );

  cpu16->pc->increment();
}


TBLRD::TBLRD( Processor* pCpu, uint opco, uint address)
     : Instruction(pCpu, opco, address)
{
  decode( pCpu, opco );
  name_str = "tblrd";
}

char *TBLRD::name(char *return_str,int len)
{
  const char *index_modes[4] = {"*","*+","*-","+*"};

  snprintf(return_str,len,"%s\t%s", name_str.c_str(), index_modes[opcode&0x3]);

  return(return_str);
}

void TBLRD::execute()
{
  if((opcode & 3)==3) cpu16->tbl.increment();

  cpu16->tbl.read();

  if((opcode & 3)==1)      cpu16->tbl.increment();
  else if((opcode & 3)==2) cpu16->tbl.decrement();

  cpu16->pc->increment();
}
//--------------------------------------------------

TBLWT::TBLWT( Processor* pCpu, uint opco, uint address)
     : Instruction(pCpu, opco, address)
{
  decode( pCpu, opco );
  name_str = "tblwt";
}

char *TBLWT::name(char *return_str,int len)
{
  const char *index_modes[4] = {"*","*+","*-","+*"};

  snprintf(return_str,len,"%s\t%s", name_str.c_str(), index_modes[opcode&0x3]);

  return(return_str);
}

void TBLWT::execute()
{
  if((opcode & 3)==3) cpu16->tbl.increment();

  cpu16->tbl.write();

  if     ((opcode & 3)==1) cpu16->tbl.increment();
  else if((opcode & 3)==2) cpu16->tbl.decrement();

  cpu16->pc->increment();
}

TLRD::TLRD( Processor* pCpu, uint opco, uint address)
    : Instruction(pCpu, opco, address)
{
  name_str = "tlrd";
}

char *TLRD::name(char *return_str,int len)
{
  const char *index_modes[4] = {"*","*+","*-","+*"};

  snprintf(return_str,len,"%s\t%s", name_str.c_str(), index_modes[opcode&0x3]);

  return(return_str);
}

void TLRD::execute()
{
}

TLWT::TLWT( Processor* pCpu, uint opco, uint address)
    : Instruction(pCpu, opco, address)
{
  decode( pCpu, opco );
  name_str = "tlwt";
}

char *TLWT::name(char *return_str,int len)
{
  const char *index_modes[4] = {"*","*+","*-","+*"};

  snprintf(return_str,len,"%s\t%s", name_str.c_str(), index_modes[opcode&0x3]);

  return(return_str);
}

void TLWT::execute()
{
}

TSTFSZ::TSTFSZ( Processor* pCpu, uint opco, uint address)
      : Register_op( pCpu, opco, address)
{
  decode( pCpu, opco );
  name_str = "tstfsz";
}

void TSTFSZ::execute()
{
  if( access ) source = cpu_pic->register_bank[register_address];
  else if (cpu16->extended_instruction() && register_address < 0x60)
      source = cpu_pic->registers[register_address + cpu16->ind2.fsr_value];
  else
      source = cpu_pic->registers[register_address];

  if( 0 == (source->get() & 0xff) ) cpu16->pc->skip();                  // Skip next instruction
  else                              cpu16->pc->increment();
}
//--------------------------------------------------

void XORLW16::execute()
{
  uint new_value;

  new_value = cpu16->Wget() ^ L;

  cpu16->Wput(new_value);
  cpu16->status->put_N_Z(new_value);

  cpu16->pc->increment();
}
//--------------------------------------------------

void XORWF16::execute()
{
  uint new_value;

  if( access ) source = cpu_pic->register_bank[register_address];
  else if (cpu16->extended_instruction() && register_address < 0x60)
      source = cpu_pic->registers[register_address + cpu16->ind2.fsr_value];
  else
      source = cpu_pic->registers[register_address];

  new_value = source->get() ^ cpu16->Wget();

  if(destination) source->put(new_value);      // Result goes to source
  else            cpu16->Wput(new_value);

  cpu16->status->put_N_Z(new_value);
  cpu16->pc->increment();
}
