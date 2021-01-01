/*
   Copyright (C) 1998-2003 T. Scott Dattalo

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
/****************************************************************
*                                                             *
*  Modified 2018 by Santiago Gonzalez    santigoro@gmail.com  *
*                                                             *
*****************************************************************/

#ifndef __PROCESSOR_H__
#define __PROCESSOR_H__

#include "gpsim_classes.h"
#include "pic-instructions.h"
#include "registers.h"
#include "stimuli.h"
#include "cpu_clock.h"

class ProcessorConstructor;

class Processor : public CpuClock
{
public:
    Processor(const char* _name=0, const char *_desc=0 );
    virtual ~Processor();

    const char* name_str;

    QHash<int, IOPIN*> m_pinList;
    int get_pin_count() { return m_pinList.size(); }
    void assign_pin( int n, IOPIN* pin ) { m_pinList[n] = pin; }
    void destroy_pin( int n );
    IOPIN* get_pin( int i ) { return m_pinList.value(i); }

    Register** registers;    /// Processor RAM
    Register** register_bank;/// Currently selected RAM bank
    uint nRegisters;

    Instruction** program_memory;/// Program memory - where instructions are stored.
    BadInstruction bad_instruction; // Processor's 'bad_instruction' object
    virtual Instruction* ConstructInvalidInstruction(Processor* cpu, uint address, uint new_opcode)
    { return new BadInstruction( cpu, address, new_opcode ); }

    uint m_uPageMask;
    uint m_uAddrMask;

    Program_Counter* pc; // Program Counter

    // Creation and manipulation of registers
    void resetRegisters( RESET_TYPE r );
    void create_invalid_registers ();
    void add_file_registers( uint start_address, uint end_address, uint alias_offset);
    void delete_file_registers( uint start_address, uint end_address, bool bRemoveWithoutDelete=false);
    void alias_file_registers( uint start_address, uint end_address, uint alias_offset);

    virtual void init_register_memory(uint memory_size);
    virtual uint register_memory_size () const = 0;

    virtual uint CalcJumpAbsoluteAddress( uint uInstAddr, uint uDestAddr) { return uDestAddr; }
    virtual uint CalcCallAbsoluteAddress( uint uInstAddr, uint uDestAddr) { return uDestAddr; }

    // Creation and manipulation of Program Memory
    virtual void init_program_memory( uint memory_size );
    virtual void init_program_memory( uint address, uint value );
    virtual void init_program_memory_at_index(uint address, uint value );
    virtual void init_program_memory_at_index(uint address, const unsigned char* , int nBytes);

    virtual uint program_memory_size(void) const {return 0;}
    virtual uint program_address_limit(void) const { return map_pm_index2address(program_memory_size());}
    virtual uint get_program_memory_at_address( uint address );
    virtual void set_out_of_range_pm( uint address, uint value );

    virtual int  map_rm_address2index( int address ) { return address;}
    virtual int  map_rm_index2address( int index ) { return index;}
    virtual int  map_pm_address2index( int address ) const { return address;}
    virtual int  map_pm_index2address( int index ) const { return index;}

    virtual Instruction* disasm( uint address,uint inst)=0; // Symbolic debugging

    // Execution control
    void stepCpuClock(){ m_phaseCurr = m_phaseCurr->advance(); }
    void executePc()
    {
      if( pc->value < program_memory_size()) program_memory[pc->value]->execute();
      else
      {
          //cout << "Program counter not valid " << hex << pc->value << endl;
          halt();
      }
    }
    virtual void exit_sleep() { fputs("RRR exit_sleep\n", stderr); }
    virtual void interrupt(void) = 0 ;

    bool getBrkOnBadRegRead() { return m_BrkOnBadRegRead; }
    bool getBrkOnBadRegWrite() { return m_BrkOnBadRegWrite; }

    // Configuration control
    virtual bool set_config_word( uint address, uint cfg_word)=0;
    virtual uint get_config_word( uint address ) = 0;
    virtual uint config_word_address( void ) {return 0;}
    virtual int  get_config_index( uint address )=0;

    virtual void reset(RESET_TYPE r) = 0;    // Processor reset

    virtual double get_Vdd() { return m_vdd; }
    virtual void   set_Vdd( double v ) { m_vdd = v; update_vdd();}
    virtual void   update_vdd();
    double m_vdd;

    virtual void create( void ) = 0;
    static Processor* construct( void );
    ProcessorConstructor*  m_pConstructorObject;

protected:
    uint  m_ProgMemSize;
};

//-------------------------------------------------------------------
// ProcessorConstructor -- a class to handle all of gpsim's supported processors

class ProcessorConstructor
{
public:
    typedef Processor* (*tCpuContructor) (const char* _name);

    ProcessorConstructor( tCpuContructor _cpu_constructor,
                          const char* name1, const char* name2,
                          const char* name3=0, const char* name4=0);

    virtual ~ProcessorConstructor(){}

    virtual Processor* ConstructProcessor( const char* opt_name=0 );

    static list <ProcessorConstructor*>* GetList();
    static list<ProcessorConstructor*>*  processor_list;
    static Processor* CreatePic( const char* type );

#define nProcessorNames 4 // The processor name (plus upto three aliases).
    const char* names[nProcessorNames];

protected:
    tCpuContructor cpu_constructor; // A pointer to a function that when called will construct a processor
};

#endif
