/*
   Copyright (C) 1998-2003 Scott Dattalo

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

#ifndef __REGISTERS_H__
#define __REGISTERS_H__

#include <string>
using namespace std;

class Processor;

#include "gpsim_classes.h"
#include "value.h"

#define AN_INVALID_ADDRESS 0xffffffff

//---------------------------------------------------------
// RegisterValue class
//
// This class is used to represent the value of registers.
// It also defines which bits have been initialized and which are valid.

class RegisterValue
{
    public:

      uint data;  // The actual numeric value of the register.
      uint init;  // bit mask of initialized bits.

      RegisterValue()
      {
        data = 0;
        init = 0xff;  // assume 8-bit wide, uninitialized registers
      }

      RegisterValue(uint d, uint i) :
        data(d), init(i)
      {
      }

      RegisterValue(const RegisterValue &value) :
      data(value.data), init(value.init)
      {
      }

      inline bool initialized() { return init == 0; }
      inline uint get() { return data; }
      inline void put(uint d) { data = d; }
      inline void put(uint d, uint i) { data = d; init = i; }
      inline uint geti() { return init; }
      inline void puti(uint i) { init = i; }

      inline void operator = (RegisterValue rv) { data = rv.data; init = rv.init; }
      inline operator uint () { return data; }
      inline operator int () { return (int)data; }
      bool operator == (const RegisterValue &rv) const
      { return data == rv.data && init == rv.init; }
      bool operator != (const RegisterValue &rv) const
      { return data != rv.data || init != rv.init; }
      void operator >>= (uint val) { data >>= val; init >>= val; }
};


/// Register - base class for gpsim registers.
/// The Register class is used by processors and modules to
/// to create memory maps and special function registers.

class Register : public Value
{
    public:

      enum REGISTER_TYPES
        {
          INVALID_REGISTER,
          GENERIC_REGISTER,
          FILE_REGISTER,
          SFR_REGISTER,
          BP_REGISTER
        };

      RegisterValue value;

      uint address;

      // If non-zero, the alias_mask describes all address at which
      // this file register appears. The assumption (that is true so
      // far for all pic architectures) is that the aliased register
      // locations differ by one bit. For example, the status register
      // appears at addresses 0x03 and 0x83 in the 14-bit core.
      // Consequently, alias_mask = 0x80 and address (above) is equal
      // to 0x03.

      uint alias_mask;

      RegisterValue por_value;  // power on reset value

      uint mValidBits;  // = 255 for 8-bit registers, = 65535 for 16-bit registers.

    public:
      Register( Processor* cpu, const char* pName );
      virtual ~Register();

      virtual uint get(); // get - method for accessing the register's contents.
      virtual void put(uint new_value);// put - method for writing a new value to the register.

      /// put_value - is the same as put(), but some extra stuff like
      /// interfacing to the gui is done. (It's more efficient than
      /// burdening the run time performance with (unnecessary) gui calls.)
      virtual void put_value(uint new_value);
      virtual uint get_value() { return(value.get()); }// get_value - same as get(), but no trace is performed

      /// getRV - get the whole register value - including the info of the three-state bits.
      virtual RegisterValue getRV()
      {
        value.data = get();
        return value;
      }

      /// putRV - write a new value to the register.
      /// \deprecated {use SimPutAsRegisterValue()}
      ///
      virtual void putRV(RegisterValue rv)
      {
        value.init = rv.init;
        put( rv.data );
      }

      /// getRV_notrace and putRV_notrace are analogous to getRV and putRV
      /// except that the action (in the derived classes) will not be
      /// traced. The primary reason for this is to allow the gui to
      /// refresh it's windows without having the side effect of filling up the trace buffer
      virtual RegisterValue getRV_notrace()
      {
        value.data = value.get();
        return value;
      }
      virtual void putRV_notrace(RegisterValue rv)
      {
        value.init = rv.init;
        put_value(rv.data);
      }

      /// set --- cast another Value object type into a register type
      /// this is used primarily by expression and stimuli processing
      /// (the put() methods are used by the processors).
      /// FIXME -- consolidate the get, set, and put methods
      virtual void set(Value *);
      virtual void get(int64_t &i); // get(int64_t &i) --- ugh.

      virtual void initialize() { }

      /// get3StateBit - returns the 3-state value of a bit
      /// if a bit is known then a '1' or '0' is returned else,
      /// a '?' is returned. No check is performed to ensure
      /// that only a single bit is checked, thus it's possible
      /// to get the state of a group of bits using this method.
      virtual char get3StateBit( uint bitMask )
      {
        RegisterValue rv = getRV_notrace();
        return (rv.init & bitMask) ? '?' : ((rv.data & bitMask) ? '1' : '0');
      }

      virtual REGISTER_TYPES isa() {return GENERIC_REGISTER;}
      virtual void reset( RESET_TYPE r ) { return; }

      /// The setbit function is not really intended for general purpose
      /// registers. Instead, it is a place holder which is over-ridden by the IO ports.
      virtual void setbit(uint bit_number, bool new_value);

      virtual uint getAddress() { return address; }
      virtual void setAddress(uint addr) { address = addr; }
      Register *getReplaced() { return m_replaced; }

      virtual void new_name(string &);
      virtual void new_name(const char *);

    protected:
      // A pointer to the register that this register replaces.
      // This is used primarily by the breakpoint code.
      Register *m_replaced;
};


//---------------------------------------------------------
// define a special 'invalid' register class. Accessess to
// to this class' value get 0

class InvalidRegister : public Register
{
    public:

      InvalidRegister(Processor*, const char *pName);

      void put(uint new_value);
      uint get();
      virtual REGISTER_TYPES isa() {return INVALID_REGISTER;}
      virtual Register *getReg()   {return 0; }
};


//---------------------------------------------------------
// Base class for a special function register.
class BitSink;

class SfrReg : public Register
{
    public:
       SfrReg( Processor*, const char* pName );

      RegisterValue wdtr_value; // wdt or mclr reset value

      virtual REGISTER_TYPES isa() {return SFR_REGISTER;}
      virtual void initialize() {}

      virtual void reset(RESET_TYPE r);

      // The assign and release BitSink methods don't do anything
      // unless derived classes redefine them. Their intent is to
      // provide an interface to the BitSink design - a design that
      // allows clients to be notified when bits change states.

      virtual bool assignBitSink(uint bitPosition, BitSink *) {return false;}
      virtual bool releaseBitSink(uint bitPosition, BitSink *) {return false;}
};



//---------------------------------------------------------
// Program Counter
//
class Program_Counter : public Value
{
    public:
      uint value;              /* pc's current value */
      uint memory_size;
      uint pclath_mask;        /* pclath confines PC to banks */
      uint instruction_phase;

      Program_Counter( const char *name, Processor *pM);
      ~Program_Counter();
      virtual void increment();
      virtual void skip();
      virtual void jump(uint new_value);
      virtual void interrupt(uint new_value);
      virtual void computed_goto(uint new_value);
      virtual void new_address(uint new_value);
      virtual void put_value(uint new_value);
      virtual void update_pcl();
      virtual void get(char *buffer, int buf_size);
      virtual uint get_value() { return value; }
      virtual uint get_PC() { return value; }

      virtual void set_PC(uint new_value) {
        value = new_value;
        this->update();
      }

      /// set --- cast another Value object type into a program counter register type
      /// this is used primarily by expression and stimuli processing
      /// (the put() methods are used by the processors).
      /// FIXME -- consolidate the get, set, and put methods
      virtual void set(Value *);

      void set_reset_address(uint _reset_address){ reset_address = _reset_address; }
      void reset();

      virtual uint get_next();

    protected:
      uint reset_address;      // Value pc gets at reset
};


//------------------------------------------------------------------------
// BitSink
//
// A BitSink is an object that can direct bit changes in an SFR to some
// place where they're needed. The purpose is to abstract the interface
// between special bits and the various peripherals.
//
// A client wishing to be notified whenever an SFR bit changes states
// will create a BitSink object and pass its pointer to the SFR. The
// client will also tell the SFR which bit this applies to. Now, when
// the bit changes states in the SFR, the SFR will call the setSink() method.

class BitSink
{
    public:
      virtual ~BitSink() {}

      virtual void setSink(bool) = 0;
};

#endif // __REGISTERS__
