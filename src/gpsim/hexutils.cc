/*
   Copyright (C) 1998 T. Scott Dattalo
   Copyright (C) 2007 Roy R Rankin

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

// T. Scott Dattalo
// Portions of this file were obtained from:
/* intel16.c - read an intel hex file */
/* Copyright (c) 1994 Ian King */

#include <stdio.h>
#include <ctype.h>

#include "pic-processor.h"
#include "hexutils.h"


HexLoader::HexLoader()
{
   // Determine endianess of current processor
   short word = 0x4321;
   isBigEndian = (*(char *)& word) != 0x21;
}

int HexLoader::getachar( FILE* file )
{
  int c;
  do c = fgetc (file);
  while( c == '\r' );     // strip LF out of MSDOS files

  return c;
}

unsigned char HexLoader::getbyte (FILE * file)
{
  unsigned char byte;
  uint data;

  if (fscanf (file, "%02x", &data) != 1) return 0;

  byte = data & 0xff;
  checksum += byte;      /* all the bytes are used in the checksum */
                         /* so here is the best place to update it */
  return byte;
}

int HexLoader::read_be_word(FILE * file) // read big endian word
{
        int w;
        w = getbyte(file);
        w |= getbyte(file) << 8;
        return(ntoh16(w));
}

int HexLoader::readihex16( Processor *pProcessor, FILE * file )
{
  int extended_address = 0;
  int address;
  int linetype = 0;
  int bytesthisline;
  int i;
  int lineCount = 1;
  int csby;
  Processor *& cpu = pProcessor;

  while (1)
  {
      if (getachar (file) != ':')
      {
        printf ("Need a colon as first character in each line\n");
        printf ("Colon missing in line %d\n", lineCount);
        return ERR_BAD_FILE;
      }

      checksum = 0;
      bytesthisline = getbyte (file);
      address = read_be_word(file) / 2;

      linetype = getbyte (file);        /* 0 for data, 1 for end  */

      switch (linetype ) 
      {
          case 0:      // Data record
            {
              unsigned char buff[256];
              bytesthisline &= 0xff;
              for (i = 0; i < bytesthisline; i++)
                buff[i] = getbyte(file);

              cpu->init_program_memory_at_index(address|extended_address, buff, bytesthisline);
            }
            break;

          case 1:      // End of hex file
            return SUCCESS;

          case 4:      // Extended address
            {
              extended_address = read_be_word(file) << 15;
              printf ("Extended linear address %x %x\n", address, extended_address);
            }
            break;

          default:
            printf ("Error! Unknown record type! %d\n", linetype);
            return ERR_BAD_FILE;
      }
      csby = getbyte (file);        // get the checksum byte
      // this should make the checksum zero * due to side effect of getbyte */

      if (checksum)
      {
        printf ("Checksum error in input file.\n");
        printf ("Got 0x%02x want 0x%02x at line %d\n", csby, (0 - checksum) & 0xff, lineCount);
        return ERR_BAD_FILE;
      }

      (void) getachar( file );        /* lose <return> */

      lineCount++;
    }
  return SUCCESS;
}

/* ... The End ... */
