/*
   Copyright (C) 1998-2004 Scott Dattalo

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

#ifndef __VALUE_H__
#define __VALUE_H__

#include <string>
using namespace std;

class Processor;

//------------------------------------------------------------------------
//
/// Value - the base class that supports types
///
/// Everything that can hold a value is derived from the Value class.
/// The primary purpose of this is to provide external objects (like
/// the gui) an abstract way of getting the value of diverse things
/// like registers, program counters, cycle counters, etc.
///
/// In addition, expressions of Values can be created and operated
/// on.

class Value
{
    public:
      Value();
      Value(const char *name, Processor *pM=0);
      virtual ~Value();

      string name_str;

      virtual uint get_leftVal() {return 0;}
      virtual uint get_rightVal() {return 0;}

      /// Value 'set' methods provide a mechanism for casting values to the
      /// the type of this value. If the type cast is not supported in a
      /// derived class, an Error will be thrown.

      virtual void set(const char *cP,int len=0);
      virtual void set(double);
      virtual void set(int64_t);
      virtual void set(int);
      virtual void set(bool);
      virtual void set(Value *);

      /// Value 'get' methods provide a mechanism of casting Value objects
      /// to other value types. If the type cast is not supported in a
      /// derived class, an Error will be thrown.
      
      virtual void get(bool &b);
      virtual void get(int &);
      virtual void get(uint64_t &);
      virtual void get(int64_t &);
      virtual void get(double &);
      virtual void get(char *, int len);

      inline operator int64_t() {
        int64_t i;
        get(i);
        return i;
      }

      inline operator int() {
        int64_t i;
        get(i);
        return (int)i;
      }

      inline operator uint() {
        int64_t i;
        get(i);
        return (uint)i;
      }

      inline Value & operator =(int i) {
        set(i);
        return *this;
      }

      inline Value & operator =(uint i) {
        set((int)i);
        return *this;
      }
      /// copy - return an object that is identical to this one.
      virtual Value *copy();

      // Some Value types that are used for symbol classes
      // contain a gpsimValue type that have update listeners.
      virtual void update(); // {}
      virtual Value* evaluate() { return copy(); }

      virtual void set_cpu(Processor *new_cpu);
      Processor* get_cpu() const;

    protected:
      Processor* cpu; // A pointer to the module that owns this value.
};


/*****************************************************************
 * Now we introduce classes for the basic built-in data types.
 * These classes are created by extending the Value class.  For
 * convenience, they all must instantiate a getVal() method that
 * returns valueof the object in question as a simple value of
 * the base data type.  For example, invoking getVal() on a
 * Boolean oject must return a simple 'bool' value.
 */
/*****************************************************************/


//------------------------------------------------------------------------
/// Integer - built in gpsim type for a 64-bit integer.

class Integer : public Value 
{
    public:

      Integer(const Integer &new_value);
      explicit Integer(int64_t new_value);
      Integer(const char *_name, int64_t new_value);
      static bool       Parse(const char *pValue, int64_t &iValue);
      static Integer *  NewObject(const char *_name, const char *pValue);

      virtual ~Integer();

      virtual string toString();
      string toString(const char* format);
      static string toString(int64_t value);
      static string toString(const char* format, int64_t value);

      virtual void get(int64_t &i);
      virtual void get(double &d);
      virtual void get(char *, int len);

      virtual void set(int64_t v);
      virtual void set(int);
      virtual void set(double d);
      virtual void set(Value *);
      virtual void set(const char *cP,int len=0);

      static void setDefaultBitmask(int64_t bitmask);

      inline void setBitmask(int64_t bitmask) {
        this->bitmask = bitmask;
      }

      inline int64_t getBitmask() {
        return bitmask;
      }

      int64_t getVal() { return value; }

      virtual Value *copy();
      /// copy the object value to a user char array
      virtual char *toString(char *, int len);
      virtual char *toBitStr(char *, int len);

      static Integer* typeCheck(Value* val, string valDesc);
      static Integer* assertValid(Value* val, string valDesc, int64_t valMin);
      static Integer* assertValid(Value* val, string valDesc, int64_t valMin, int64_t valMax);

      inline operator int64_t() {
        int64_t i;
        get(i);
        return i;
      }

      inline operator uint64_t() {
        int64_t i;
        get(i);
        return (uint64_t)i;
      }

      inline operator bool() {
        int64_t i;
        get(i);
        return i != 0;
      }

      inline operator int() {
        int64_t i;
        get(i);
        return (int)i;
      }

      inline operator uint() {
        int64_t i;
        get(i);
        return (uint)i;
      }

      inline Integer & operator =(const Integer &i) {
        Integer & ii = (Integer &)i;
        int64_t iNew = (int64_t)ii;
        set(iNew);
        bitmask = i.bitmask;
        return *this;
      }

      inline Integer & operator =(int i) {
        set(i);
        return *this;
      }

      inline Integer & operator =(uint i) {
        set((int)i);
        return *this;
      }

      inline Integer & operator &=(int iValue) {
        int64_t i;
        get(i);
        set((int)i & iValue);
        return *this;
      }

      inline Integer & operator |=(int iValue) {
        int64_t i;
        get(i);
        set((int)i | iValue);
        return *this;
      }

      inline Integer & operator +=(int iValue) {
        int64_t i;
        get(i);
        set((int)i + iValue);
        return *this;
      }

      inline Integer & operator ++(int) {
        int64_t i;
        get(i);
        set((int)i + 1);
        return *this;
      }

      inline Integer & operator --(int) {
        int64_t i;
        get(i);
        set((int)i - 1);
        return *this;
      }

      inline Integer & operator <<(int iShift) {
        int64_t i;
        get(i);
        set(i << iShift);
        return *this;
      }

      inline bool operator !() {
        int64_t i;
        get(i);
        return i == 0;
      }

    private:
      int64_t value;
      int64_t bitmask; // Used for display purposes
      static int64_t def_bitmask;
};

inline bool operator!=(Integer &iLValue, Integer &iRValue) {
  return (int64_t)iLValue != (int64_t)iRValue;
}


#endif // __VALUE_H__
