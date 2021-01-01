
/*
   Copyright (C) 1998-2000 T. Scott Dattalo

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

#ifndef __PIC_REGISTERS_H__
#define __PIC_REGISTERS_H__

#include "gpsim_classes.h"
#include "registers.h"

class OPTION_REG : public  SfrReg
{
    public:

    enum
      {
        PS0    = 1<<0,
        PS1    = 1<<1,
        PS2    = 1<<2,
        PSA    = 1<<3,
        T0SE   = 1<<4,
        T0CS   = 1<<5,
        BIT6   = 1<<6,
        BIT7   = 1<<7
      };

      uint prescale;

      OPTION_REG(Processor *pCpu, const char *pName);

      inline uint get_prescale() { return value.get() & (PS0 | PS1 | PS2); }
      inline uint get_psa() { return value.get() & PSA; }
      inline uint get_t0cs() { return value.get() & T0CS; }
      inline uint get_t0se() { return value.get() & T0SE; }

      virtual void put(uint new_value);
      virtual void reset(RESET_TYPE r);
      virtual void initialize() override;
};

class OPTION_REG_2 : public OPTION_REG // For use on 14bit enhanced cores
{
    public:
      OPTION_REG_2(Processor *pCpu, const char *pName );

      virtual void put(uint new_value);
      virtual void initialize() override;
};

#endif
