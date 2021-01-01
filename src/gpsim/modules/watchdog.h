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


#ifndef __PIC_WDT_H__
#define __PIC_WDT_H__

#include "gpsim_classes.h"
#include "trigger.h"

class pic_processor;

//---------------------------------------------------------
// Watch Dog Timer

class WDT : public TriggerObject
{
    public:
      WDT( pic_processor*, double _timeout);
      void put(uint new_value);
      virtual void initialize(bool enable, bool _use_t0_prescale = true);
      virtual void swdten(bool enable);
      void set_timeout(double);
      virtual void set_prescale(uint);
      virtual void set_postscale(uint);
      virtual void reset(RESET_TYPE r);
      void clear();
      virtual void callback();
      virtual void update();
      virtual void callback_print();
      void set_breakpoint(uint bpn);
      bool hasBreak() { return breakpoint != 0;}

    protected:
      pic_processor *cpu;           // The cpu to which this wdt belongs.

      uint breakpoint,
           prescale,
           postscale;

      uint64_t future_cycle;

      double timeout;   // When no prescaler is assigned
      bool   wdte;
      bool   warned;
      bool   cfgw_enable;  // Enabled from Configureation word
      bool   use_t0_prescale;
};

#endif
