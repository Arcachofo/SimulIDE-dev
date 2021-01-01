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


#include <stdio.h>
#include <iostream>
#include <iomanip>
#include <cstdio>
#include <typeinfo>

#include <QDebug>

#include "processor.h"
#include "value.h"


//------------------------------------------------------------------------
Value::Value()
     : cpu(0)
{
}

Value::Value(const char*_name, Processor *pMod)
     : cpu( pMod )
{
}

Value::~Value()
{
}
void Value::update()
{
}

void Value::set(const char *cP,int i)
{
  qDebug() <<" cannot assign string to a ";
}
void Value::set(double d)
{
  qDebug() <<" cannot assign a double to a ";
}
void Value::set(int64_t i)
{
  qDebug() <<" cannot assign an integer to a ";
}
void Value::set(bool v)
{
  qDebug() <<" cannot assign a boolean to a ";
}

void Value::set(int i)
{
  int64_t i64 = i;
  set(i64);
}

void Value::set(Value *v)
{
  qDebug() <<" cannot assign a Value to a ";
}

void Value::get(int64_t &i)
{
  qDebug() << " cannot be converted to an integer ";
}

void Value::get(int &i)
{
  int64_t i64;
  get(i64);
  i = (int) i64;
}

void Value::get(uint64_t &i)
{
  // FIXME - casting a signed int to an uint -- probably should issue a warning
  int64_t i64;
  get(i64);
  i = (int64_t) i64;
}

void Value::get(bool &b)
{
  qDebug() <<" cannot be converted to a boolean";
}

void Value::get(double &d)
{
  qDebug() <<" cannot be converted to a double ";
}

// get as a string - no error is thrown if the derived class
// does not provide a method for converting to a string -
// instead we'll return a bogus value.

void Value::get(char *buffer, int buf_size)
{
  if(buffer) strncpy(buffer,"INVALID",buf_size);
}

Value *Value::copy()
{
  qDebug() <<" cannot copy ";
  return 0l;
}

Processor* Value::get_cpu() const
{
  return cpu;
}

void Value::set_cpu( Processor* new_cpu )
{
  cpu = new_cpu;
}

/*****************************************************************
 * The Integer class.
 */
Integer::Integer( const Integer &new_value ) 
       : Value()
{
  Integer & nv = (Integer&)new_value;
  nv.get(value);
  bitmask = new_value.bitmask;
}

Integer::Integer(int64_t newValue)
{
  value = newValue;
  bitmask = def_bitmask;
}

Integer::Integer(const char *_name, int64_t newValue )
  : Value(_name )
{
  value = newValue;
  bitmask = def_bitmask;
}

int64_t Integer::def_bitmask = 0xffffffff;

Integer::~Integer()
{
}

void Integer::setDefaultBitmask(int64_t bitmask) 
{
  def_bitmask = bitmask;
}

Value *Integer::copy()
{
  int64_t i;
  get(i);
  return new Integer(i);
}

void Integer::set(double d)
{
  int64_t i = (int64_t)d;
  set(i);
}

void Integer::set(int64_t i)
{
  value = i;
  //if(get_xref())
  //  get_xref()->set(i);
}
void Integer::set(int i)
{
  int64_t ii = i;
  set(ii);
}
void Integer::set(Value *v)
{
  int64_t iv = 0;
  if (v)  v->get(iv);

  set(iv);
}

void Integer::set(const char *buffer, int buf_size)
{
  if(buffer) 
  {
    int64_t i;
    if(Parse(buffer, i)) set(i);
  }
}

bool Integer::Parse(const char *pValue, int64_t &iValue) {
    if(::isdigit(*pValue)) 
    {
      if(strchr(pValue, '.'))  return false;
      else          return sscanf(pValue, "%li", &iValue) == 1;
    }
    else if(*pValue == '$' && ::isxdigit(*(pValue+1))) 
    {
      // hexidecimal integer
      char szHex[10] = "0x";
      strcat(&szHex[0], pValue + 1);
      return sscanf(szHex, "%li" , &iValue) == 1;
    }
    return false;
}

Integer * Integer::NewObject(const char *_name, const char *pValue )
{
  int64_t iValue;
  if(Parse(pValue, iValue)) return new Integer(_name, iValue );

  return NULL;
}


void Integer::get(int64_t &i)
{
  i = value;
}

void Integer::get(double &d)
{
  int64_t i;
  get(i);
  d = (double)i;
}

void Integer::get(char *buffer, int buf_size)
{
  if(buffer) {

    int64_t i;
    get(i);
    long long int j = i;
    snprintf(buffer,buf_size,"%" "ll" "d",j);
  }

}

string Integer::toString()
{
  return "";
}

string Integer::toString(const char* format)
{
  char cvtBuf[1024];

  int64_t i;
  get(i);

  snprintf(cvtBuf,sizeof(cvtBuf), format, i);
  return (string(&cvtBuf[0]));
}


string Integer::toString(const char* format, int64_t value)
{
  char cvtBuf[1024];

  snprintf(cvtBuf,sizeof(cvtBuf), format, value);
  return (string(&cvtBuf[0]));
}

string Integer::toString(int64_t value)
{
  char cvtBuf[1024];
  long long int v=value;
  snprintf(cvtBuf,sizeof(cvtBuf), "%" "ll" "d", v);
  return (string(&cvtBuf[0]));
}

char *Integer::toString(char *return_str, int len)
{
  return return_str;
}
char *Integer::toBitStr(char *return_str, int len)
{
  if(return_str) {
    int64_t i;
    get(i);
    int j=0;
    int mask=1<<31;
    for( ; mask ; mask>>=1, j++)
      if(j<len) return_str[j] = ( (i & mask) ? 1 : 0);

    if(j<len) return_str[j]=0;
  }
  return return_str;
}

Integer* Integer::typeCheck(Value* val, string valDesc)
{
  if (typeid(*val) != typeid(Integer)) {
    qDebug() <<"Not an Integer";
  }

  // This static cast is totally safe in light of our typecheck, above.
  return((Integer*)(val));
}

Integer* Integer::assertValid(Value* val, string valDesc, int64_t valMin)
{
  Integer* iVal;
  int64_t i;

  iVal = Integer::typeCheck(val, valDesc);
  iVal->get(i);

  if (i < valMin) {
    qDebug() << " must be greater than " + QString::number(valMin) +
                    ", saw " + QString::number(i);
  }

  return(iVal);
}

Integer* Integer::assertValid(Value* val, string valDesc, int64_t valMin, int64_t valMax)
{
  Integer* iVal;
  int64_t i;

  iVal = (Integer::typeCheck(val, valDesc));

  iVal->get(i);

  if ((i < valMin) || (i>valMax)) {
    qDebug() <<" must be be in the range [" + QString::number(valMin) + ".." +
                    QString::number(valMax) + "], saw " + QString::number(i);
  }

  return(iVal);
}

