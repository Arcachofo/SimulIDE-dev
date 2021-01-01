/*
   Copyright( C) 1998-2003 T. Scott Dattalo

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
/****************************************************************
*                                                               *
*  Modified 2018 by Santiago Gonzalez    santigoro@gmail.com    *
*                                                               *
*****************************************************************/

/*
  stuff that needs to be fixed:

  Register aliasing
  The "invalid instruction" in program memory.
*/

#include "processor.h"
#include "pic-processor.h"

Processor::Processor(const char* _name, const char*_desc)
    : bad_instruction( 0, 0x3fff, 0 )
    , pc(0)
{
    name_str = _name;
    registers = 0l;
    m_cycle = 0;
    m_nextBreak = 0xFFFFFFFF;

    m_pConstructorObject = 0;
    m_ProgMemSize = 0;

    m_uPageMask    = 0x00;
    m_uAddrMask    = 0xff;

    //set_Vdd( 5.0 );
}

Processor::~Processor()
{
    for( uint i=0; i<nRegisters; i++ ) // Delete invalid registers
    {
        InvalidRegister* pReg = dynamic_cast<InvalidRegister*>( registers[i]);
        if( pReg )
        {
            delete registers[i];
            registers[i]= 0;
        }
    }
    delete []registers;

    for( uint i = 0; i < m_ProgMemSize; i++ )
    {
        if( program_memory[i] != &bad_instruction ) delete program_memory[i];
    }
    delete []program_memory;
}

void Processor::destroy_pin( int n )
{
    if( m_pinList.contains( n ))
    {
        delete m_pinList.value( n );
        m_pinList[n] = 0;
    }
}

void Processor::init_register_memory( uint memory_size ) // Allocate an array for holding register objects.
{
    registers = new Register*[memory_size];
    nRegisters = memory_size;

    if( registers == 0 ) qDebug() << "Out of memory - PIC register space";

    // For processors with banked memory, the register_bank corresponds to the
    // active bank. Let this point to the beginning of the register array for now.
    register_bank = registers;

    for( uint i=0; i<memory_size; i++ ) registers[i] = 0;
}

void Processor::resetRegisters( RESET_TYPE r )
{
    for( uint i=0; i<nRegisters; i++ ) registers[i]->reset(r);
}

//   The purpose of this function is to complete the initialization
// of the file register memory by placing an instance of an 'invalid
// file register' at each 'invalid' memory location. Most of PIC's
// do not use the entire address space available, so this routine fills the voids.

void Processor::create_invalid_registers()
{
    uint addr;

    // Note, each invalid register is given its own object. This enables
    // the simulation code to efficiently capture any invalid register
    // access. Furthermore, it's possible to set break points on
    // individual invalid file registers. By default, gpsim halts whenever
    // there is an invalid file register access.

    for( addr=0; addr<register_memory_size(); addr+=map_rm_index2address(1) )
    {
        uint index = map_rm_address2index(addr);

        if( !registers[index] )
        {
            char nameBuff[100];
            snprintf(nameBuff,sizeof(nameBuff), "INVREG_%X",addr);

            registers[index] = new InvalidRegister(this, nameBuff);
            registers[index]->setAddress(addr);
        }
    }
}

// Allocate memory for the general purpose registers.

void Processor::add_file_registers( uint start_address, uint end_address, uint alias_offset)
{
    char str[100];
    for( uint j = start_address; j <= end_address; j++ )
    {
        if( registers[j] &&( registers[j]->isa() == Register::INVALID_REGISTER))
            delete registers[j];
        
        else if( registers[j])
            cout << __FUNCTION__ << " Already register " << registers[j]->name_str << " at 0x" << hex << j <<endl;

        //The default register name is simply its address
        snprintf( str, sizeof(str), "REG%03X", j);
        registers[j] = new Register(this, str);

        if( alias_offset)
        {
            registers[j + alias_offset] = registers[j];
            registers[j]->alias_mask = alias_offset;
        }
        else registers[j]->alias_mask = 0;

        registers[j]->setAddress(j);
    }
}

void Processor::delete_file_registers( uint start_address, uint end_address, bool bRemoveWithoutDelete )
{
    //  FIXME - this function is bogus.
    // The aliased registers do not need to be searched for - the alias mask
    // can tell at what addresses a register is aliased.

#define SMALLEST_ALIAS_DISTANCE  32
#define ALIAS_MASK (SMALLEST_ALIAS_DISTANCE-1)

    for( uint j=start_address; j<=end_address; j++)
    {
        if(registers[j])
        {
            Register *thisReg = registers[j];
            Register *replaced = thisReg->getReplaced();

            if(thisReg->alias_mask) // This register appears in more than one place. Let's find all of its aliases.
            {
                for( uint i=j&ALIAS_MASK; i<nRegisters; i+=SMALLEST_ALIAS_DISTANCE)
                    if(thisReg == registers[i])
                        registers[i] = 0;
            }
            registers[j] = 0;

            if( !bRemoveWithoutDelete)
            {
                if( replaced) delete replaced;
                delete thisReg;
            }
        }
    }
}

//  The purpose of this member function is to alias the
// general purpose registers.

void Processor::alias_file_registers( uint start_address, uint end_address, uint alias_offset )
{
    // FIXME -- it'd probably make better sense to keep a list of register addresses at which a particular register appears.

    for( uint j = start_address; j <= end_address; j++)
    {
        if( alias_offset &&( j+alias_offset < nRegisters))
        {
            if( registers[j + alias_offset])
            {
                if( registers[j + alias_offset] == registers[j]) printf("alias_file_register Duplicate alias %s from 0x%x to 0x%x \n",registers[j + alias_offset]->name_str.c_str(), j, j+alias_offset);
                else delete registers[j + alias_offset];
            }
            registers[j + alias_offset] = registers[j];

            if( registers[j]) registers[j]->alias_mask = alias_offset;
        }
    }
}

// The purpose of this member function is to allocate memory for the
// pic's code space. The 'memory_size' parameter tells how much memory is to be allocated
//
//  The following is not correct for 18f2455 and 18f4455 processors so test has been disabled( RRR)
//
//  AND it should be an integer of the form of 2^n.
// If the memory size is not of the form of 2^n, then this routine will
// round up to the next integer that is of the form 2^n.
//
//   Once the memory has been allocated, this routine will initialize
// it with the 'bad_instruction'. The bad_instruction is an instantiation
// of the instruction class that chokes gpsim if it is executed. Note that
// each processor owns its own 'bad_instruction' object.

void Processor::init_program_memory( uint memory_size )
{
    pc->memory_size = memory_size; // The memory_size_mask is used by the branching instructions

    // Initialize 'program_memory'. 'program_memory' is a pointer to an array of
    // pointers of type 'instruction'. This is where the simulated instructions
    // are stored.
    program_memory = new Instruction*[memory_size];
    if( program_memory == 0 ) qDebug()<<"Out of memory for program space";

    m_ProgMemSize = memory_size;

    bad_instruction.set_cpu(this);
    for( uint i = 0; i < memory_size; i++) program_memory[i] = &bad_instruction;
}

// The purpose of this member fucntion is to instantiate an Instruction
// object in the program memory. If the opcode is invalid, then a 'bad_instruction'
// is inserted into the program memory instead. If the address is beyond
// the program memory address space, then it may be that the 'opcode' is
// is in fact a configuration word.
//
void Processor::init_program_memory( uint address, uint value )
{
    uint uIndex = map_pm_address2index(address);

    if( !program_memory) qDebug()<< "ERROR: internal bug " << __FILE__ << ":" << __LINE__;

    if(uIndex < program_memory_size())
    {
        if(program_memory[uIndex] != 0 && program_memory[uIndex]->isa() != Instruction::BAD_INSTRUCTION)
        {
            delete program_memory[uIndex];           // this should not happen
        }
        program_memory[uIndex] = disasm( address,value );
        if(program_memory[uIndex] == 0) program_memory[uIndex] = &bad_instruction;
    }
    else if( set_config_word(address, value) ) {;}
    else set_out_of_range_pm( address,value );  // could be e2prom
}

void Processor::init_program_memory_at_index( uint uIndex, uint value )
{
    init_program_memory( map_pm_index2address(uIndex), value );
}

void Processor::init_program_memory_at_index( uint uIndex, const unsigned char *bytes, int nBytes )
{
    for( int i=0; i<nBytes/2; i++ )
        init_program_memory_at_index( uIndex+i,( ((uint)bytes[2*i+1])<<8)  | bytes[2*i] );
}

uint Processor::get_program_memory_at_address( uint address ) // Fetch the rom contents at a particular address.
{
    uint uIndex = map_pm_address2index(address);

    return( uIndex < program_memory_size() && program_memory[uIndex])
            ? program_memory[uIndex]->get_opcode() : 0xffffffff;
}

void Processor::set_out_of_range_pm( uint address, uint value )
{
    cout << "Warning::Out of range address " << address << " value " << value << endl;
    cout << "Max allowed address is 0x" << hex <<( program_address_limit()-1) << '\n';
}

void Processor::update_vdd() // If Vdd is changed, fix up the digital high low thresholds
{
    for( int i=1; i<=get_pin_count(); i++)
    {
        IOPIN* pin = get_pin( i );
        if( pin ) pin->set_digital_threshold( m_vdd );
    }
}

Processor* Processor::construct(void)
{
    cout << " Can't create a generic processor\n";
    return 0;
}

//========================================================================

list<ProcessorConstructor*>* ProcessorConstructor::processor_list;

ProcessorConstructor::ProcessorConstructor( tCpuContructor _cpu_constructor,
                                            const char *name1, const char *name2,
                                            const char *name3, const char *name4)
{
    cpu_constructor = _cpu_constructor;  // Pointer to the processor constructor
    names[0] = name1;                    // First name
    names[1] = name2;                    //  and three aliases...
    names[2] = name3;
    names[3] = name4;

    GetList()->push_back(this); // Add the processor to the list of supported processors:
}

Processor* ProcessorConstructor::ConstructProcessor(const char *opt_name)
{
    if( opt_name && strlen(opt_name)) return cpu_constructor( opt_name );
    return cpu_constructor( names[2] );
}

list<ProcessorConstructor*>* ProcessorConstructor::GetList()
{
    if( processor_list == NULL) processor_list = new list <ProcessorConstructor*>;
    return processor_list;
}

// Search through the list of supported processors for the one matching 'name' and construct Processor
Processor* ProcessorConstructor::CreatePic( const char *name )
{
    list<ProcessorConstructor*>::iterator processor_iterator;
    list<ProcessorConstructor*>* pl = GetList();

    for( processor_iterator = pl->begin(); processor_iterator != pl->end(); ++processor_iterator )
    {
        ProcessorConstructor *p = *processor_iterator;
        for( int j=0; j<nProcessorNames; j++ )
            if( p->names[j] && strcmp( name,p->names[j] ) == 0)
                return p->ConstructProcessor( name );
    }
    return 0;
}

