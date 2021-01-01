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

#if !defined(__HEXUTILS_H__)
#define __HEXUTILS_H__

#include "stdio.h"
#include "processor.h"

class HexLoader
{
    public:
      enum {
        SUCCESS                       =  0,
        ERR_BAD_FILE                  = -1,
      };
      
      HexLoader();
      int readihex16 (Processor *pProcessor, FILE * file);

    private:
      unsigned char checksum;
      bool isBigEndian;

      int           getachar (FILE * file);
      unsigned char getbyte  (FILE * file);
      int read_be_word(FILE * file);

      int readihexN (int bytes_per_word, Register **fr, int32_t size, FILE * file, int32_t offset);

      inline int ntoh16(int w) { return isBigEndian ? w : ((w >> 8) & 0xff) | ((w & 0xff) << 8);}
};
#endif // __HEXUTILS_H__
