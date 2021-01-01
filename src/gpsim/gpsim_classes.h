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

/*
 * gpsim_classes.h
 *
 * This include file contains most of the class names defined in gpsim
 * It's used to define forward references to classes and help alleviate
 * include file dependencies.
 */

#ifndef __GPSIM_CLASSES_H__
#define __GPSIM_CLASSES_H__

typedef unsigned int uint;


enum RESET_TYPE
{
  POR_RESET,          // Power-on reset
  WDT_RESET,          // Watch Dog timer timeout reset
  IO_RESET,           // I/O pin  reset
  MCLR_RESET,         // MCLR (Master Clear) reset
  SOFT_RESET,         // Software initiated reset
  BOD_RESET,          // Brown out detection reset
  SIM_RESET,          // Simulation Reset
  EXIT_RESET,         // Leaving Reset, resuming normal execution.
  OTHER_RESET,        //
  STKUNF_RESET,		  // Stack undeflow reset
  STKOVF_RESET		  // Statck overflow reset

};

enum IOPIN_TYPE
{
  INPUT_ONLY,          // e.g. MCLR
  BI_DIRECTIONAL,      // most iopins
  BI_DIRECTIONAL_PU,   // same as bi_directional, but with pullup resistor. e.g. portb
  OPEN_COLLECTOR       // bit4 in porta on the 18 pin midrange devices.
};

enum PROCESSOR_TYPE
{
  _PIC_PROCESSOR_,
  _14BIT_PROCESSOR_,
  _14BIT_E_PROCESSOR_, // 14bit enhanced processor
  _12BIT_PROCESSOR_,
  _PIC17_PROCESSOR_,
  _PIC18_PROCESSOR_,
  _P10F200_,
  _P10F202_,
  _P10F204_,
  _P10F206_,
  _P10F220_,
  _P10F222_,
  _P10F320_,
  _P10LF320_,
  _P10F322_,
  _P10LF322_,
  _P12C508_,
  _P12C509_,
  _P12F508_,
  _P12F509_,
  _P12F510_,
  _P12F629_,
  _P12F675_,
  _P12F683_,
  _P12F1822_,
  _P12LF1822_,
  _P12F1840_,
  _P12LF1840_,
  _P16C84_,
  _P16CR83_,
  _P16CR84_,
  _P12CE518_,
  _P12CE519_,
  _P16F83_,
  _P16F84_,
  _P16C71_,
  _P16C712_,
  _P16C716_,
  _P16C54_,
  _P16C55_,
  _P16C56_,
  _P16C61_,
  _P16C62_,
  _P16C62A_,
  _P16CR62_,
  _P16F505_,
  _P16F627_,
  _P16F628_,
  _P16F630_,
  _P16F631_,
  _P16F648_,
  _P16F676_,
  _P16F677_,
  _P16F684_,
  _P16F685_,
  _P16F687_,
  _P16F689_,
  _P16F690_,
  _P16C63_,
  _P16C64_,
  _P16C64A_,
  _P16CR64_,
  _P16C65_,
  _P16C65A_,
  _P16C72_,
  _P16C73_,
  _P16C74_,
  _P16F73_,
  _P16F74_,
  _P16F716_,
  _P16F87_,
  _P16F88_,
  _P16F818_,
  _P16F819_,
  _P16F871_,
  _P16F873_,
  _P16F873A_,
  _P16F874_,
  _P16F874A_,
  _P16F876_,
  _P16F876A_,
  _P16F877_,
  _P16F877A_,
  _P16F882_,
  _P16F883_,
  _P16F884_,
  _P16F886_,
  _P16F887_,
  _P16F913_,
  _P16F914_,
  _P16F916_,
  _P16F917_,
  _P16F1788_,
  _P16F1503_,
  _P16LF1503_,
  _P16LF1788_,
  _P16F1789_,
  _P16F1823_,
  _P16LF1823_,
  _P16F1825_,
  _P16LF1825_,
  _P17C7xx_,
  _P17C75x_,
  _P17C752_,
  _P17C756_,
  _P17C756A_,
  _P17C762_,
  _P17C766_,
  _P18Cxx2_,
  _P18C2x2_,
  _P18C242_,
  _P18F242_,
  _P18F248_,
  _P18F258_,
  _P18F448_,
  _P18F458_,
  _P18C252_,
  _P18F252_,
  _P18C442_,
  _P18C452_,
  _P18F442_,
  _P18F452_,
  _P18F1220_,
  _P18F1320_,
  _P18F14K22_,
  _P18F2221_,
  _P18F2321_,
  _P18F2420_,
  _P18F2455_,
  _P18F2520_,
  _P18F2525_,
  _P18F2550_,
  _P18F26K22_,
  _P18F2620_,
  _P18F4221_,
  _P18F4321_,
  _P18F4420_,
  _P18F4455_,
  _P18F4520_,
  _P18F4550_,
  _P18F4620_,
  _P18F6520_,
};

#endif //  __GPSIM_CLASSES_H__
