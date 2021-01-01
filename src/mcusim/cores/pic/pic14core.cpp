/***************************************************************************
 *   Copyright (C) 2020 by santiago González                               *
 *   santigoro@gmail.com                                                   *
 *                                                                         *
 *   This program is free software; you can redistribute it and/or modify  *
 *   it under the terms of the GNU General Public License as published by  *
 *   the Free Software Foundation; either version 3 of the License, or     *
 *   (at your option) any later version.                                   *
 *                                                                         *
 *   This program is distributed in the hope that it will be useful,       *
 *   but WITHOUT ANY WARRANTY; without even the implied warranty of        *
 *   MERCHANTABILITY or FITNESS FOR A PARTICULAR PURPOSE.  See the         *
 *   GNU General Public License for more details.                          *
 *                                                                         *
 *   You should have received a copy of the GNU General Public License     *
 *   along with this program; if not, see <http://www.gnu.org/licenses/>.  *
 *                                                                         *
 ***************************************************************************/

#include "pic14core.h"


Pic14Core::Pic14Core( eMcu* mcu )
         : CoreCpu( mcu )
{

}
Pic14Core::~Pic14Core() {}

void Pic14Core::reset()
{
    CoreCpu::reset();
}

inline void Pic14Core::RETURN()
{

}

inline void Pic14Core::RETFIE()
{

}

inline void Pic14Core::OPTION()
{

}

inline void Pic14Core::SLEEP()
{

}

inline void Pic14Core::CLRWDT()
{

}

inline void Pic14Core::TRIS( uint8_t f )
{

}

inline void Pic14Core::MOVF( uint8_t f )
{

}

inline void Pic14Core::CLR( uint8_t f, uint8_t d )
{

}

void Pic14Core::runDecoder()
{
    uint16_t instruction = m_progMem[PC] & 0x3FFF;

    //uint32_t new_pc = PC + 1;    // future "default" pc
    int cycle = 1;

    if( (instruction & 0x3F80) == 0 )  // Miscellaneous instructions
    {
        switch( instruction & 0x000C)
        {
            case 0x0008: {
                if     ( instruction == 0x0008 ) RETURN(); // RETURN 00 0000 0000 1000
                else if( instruction == 0x0009 ) RETFIE();// RETFIE 00 0000 0000 1001
            } break;
            case 0x0000: {
                if     ( instruction == 0x0062 ) OPTION();// OPTION 00 0000 0110 0010
                else if( instruction == 0x0063 ) SLEEP();// SLEEP 00 0000 0110 0011
            } break;
            case 0x0004: {
                if     ( instruction == 0x0064 ) CLRWDT();// CLRWDT 00 0000 0110 0100
                else if( instruction > 0x0064 ) // TRIS f 00 0000 0110 01ff
                {
                    TRIS( instruction & 0x03 );
                }
            } break;
        }
    }
    else if( (instruction & 0x3000) == 0 ) // ALU operations: dest ← OP(f,W)
    {
        uint8_t f = instruction & 0x7F;
        uint8_t d = instruction>>7 & 1;

        if( (instruction & 0x3800) == 0 ) {
            switch( instruction & 0x0700) {
                case 0x0000: { MOVF( f ); } break;// MOVWF f   00 0000 1fff ffff
                case 0x0100: { CLR( f, d ); } break;// CLR f,d   00 0001 dfff ffff
                case 0x0200: {            // SUBWF f,d 00 0010 dfff ffff
                }
                case 0x0300: {            // DECF f,d  00 0011 dfff ffff
                }
                case 0x0400: {            // IORWF f,d 00 0100 dfff ffff
                }
                case 0x0500: {            // ANDWF f,d 00 0101 dfff ffff
                }
                case 0x0600: {            // XORWF f,d 00 0110 dfff ffff
                }
                case 0x0700: {            // ADDWF f,d 00 0111 dfff ffff
                }
            }
        } else {
            switch( instruction & 0x0700) {
                case 0x0000: {            // MOVF f,d   00 1000 dfff ffff
                }
                case 0x0100: {            // COMF f,d   00 0001 dfff ffff
                }
                case 0x0200: {            // INCF f,d   00 0010 dfff ffff
                }
                case 0x0300: {            // DECFSZ f,d 00 0011 dfff ffff
                }
                case 0x0400: {            // RRF f,d    00 0100 dfff ffff
                }
                case 0x0500: {            // RLF f,d    00 0101 dfff ffff
                }
                case 0x0600: {            // SWAPF f,d  00 0110 dfff ffff
                }
                case 0x0700: {            // INCFSZ f,d 00 0111 dfff ffff
                }
            }
        }
    } else {
        if( (instruction & 0x3000) == 0x1000 ) // Bit operations
        {
            //uint8_t f = instruction & 0x7F;
            //uint8_t b = instruction>>7 & 7;

            switch( instruction & 0x3C00){
                case 0x1000: {} break; // BCF f,b 01 00bb bkkk kkkk
                case 0x1400: {} break; // BSF f,b 01 01bb bkkk kkkk
                case 0x1800: {} break; // BTFSC f,b 01 10bb bkkk kkkk
                case 0x1C00: {} break; // BTFSS f,b 01 11bb bkkk kkkk
            }
        }
        else if( (instruction & 0x3000) == 0x2000 ) // Control transfers
        {
            //uint16_t k = instruction & 0x07FF;

            if( (instruction & 0x0800) == 0 ) // CALL k 10 0kkk kkkk kkkk
            {
            }
            else {}                            // GOTO k 10 1kkk kkkk kkkk
        }
        else if( (instruction & 0x3000) == 0x3000 ) // Operations with W and 8-bit literal: W ← OP(k,W)
        {
            //uint8_t k = instruction & 0xFF;

            switch( instruction & 0x3C00){
                case 0x3000: {            // MOVLW k 11 00xx kkkk kkkk
                }
                case 0x3400: {            // RETLW k 11 01xx kkkk kkkk
                }
                case 0x3800: {
                    switch( instruction & 0x3F00) {
                        case 0x3800: {            // IORLW k 11 1000 kkkk kkkk
                        }
                        case 0x3900: {            // ANDLW k 11 1001 kkkk kkkk
                        }
                        case 0x3A00: {            // XORLW k 11 1010 kkkk kkkk
                        }
                    }
                }
                case 0x3C00: {
                    if((instruction & 0x0200)==0 )// SUBLW k 11 110x kkkk kkkk
                    {
                    }
                    else                          // ADDLW k 11 111x kkkk kkkk
                    {

                    }
                }
            }

        }
    }
    m_mcu->cyclesDone = cycle;
}

void Pic14Core::createInstructions()
{

}
