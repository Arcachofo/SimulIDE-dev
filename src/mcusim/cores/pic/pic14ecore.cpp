/***************************************************************************
 *   Copyright (C) 2021 by santiago González                               *
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

#include "pic14ecore.h"
#include "datautils.h"
#include "regwatcher.h"

Pic14eCore::Pic14eCore( eMcu* mcu )
          : PicMrCore( mcu )
{
    m_stackSize = 16;

    m_FSR = NULL; //m_mcu->getReg( "FSR" );
    m_OPTION = NULL; //m_mcu->getReg( "OPTION" );

    m_bankBits = getRegBits( "BSR0,BSR1,BSR2,BSR3,BSR4", mcu );
    watchBitNames( "BSR0,BSR1,BSR2,BSR3,BSR4", R_WRITE, this, &Pic14eCore::setBank, mcu );
}
Pic14eCore::~Pic14eCore() {}

// Miscellaneous instructions

/*inline void Pic14eCore::RESET()
{

}*/

inline void Pic14eCore::CALLW()
{
}

inline void Pic14eCore::BRW()
{
}

inline void Pic14eCore::MOVIW_pF( uint8_t n )
{
}

inline void Pic14eCore::MOVIW_nF( uint8_t n )
{
}

inline void Pic14eCore::MOVIW_Fp( uint8_t n )
{
}

inline void Pic14eCore::MOVIW_Fn( uint8_t n )
{
}

inline void Pic14eCore::MOVWI_pF( uint8_t n )
{
}

inline void Pic14eCore::MOVWI_nF( uint8_t n )
{
}

inline void Pic14eCore::MOVWI_Fp( uint8_t n )
{
}

inline void Pic14eCore::MOVWI_Fn( uint8_t n )
{
}

inline void Pic14eCore::MOVLB( uint8_t k )
{
}

// ALU operations: dest ← OP(f,W)
inline void Pic14eCore::LSLF( uint8_t f, uint8_t d )
{
}

inline void Pic14eCore::LSRF( uint8_t f, uint8_t d )
{
}

inline void Pic14eCore::ASRF( uint8_t f, uint8_t d )
{
}

inline void Pic14eCore::SUBWFB( uint8_t f, uint8_t d )
{
}

inline void Pic14eCore::ADDWFC( uint8_t f, uint8_t d )
{
    //if( *m_STATUS & 1<<C ) newV |= 1<<7; // Carry In
    //uint8_t newV = add( GET_RAM( f ), m_Wreg+carry );
    //setValue( newV, f, d );
}

// Operations with literal k

inline void Pic14eCore::ADDFSR( uint8_t n, uint8_t k )
{
}

inline void Pic14eCore::MOVLP( uint8_t k )
{
}

inline void Pic14eCore::BRA( uint8_t k )
{
}

inline void Pic14eCore::MOVIW( uint8_t n, uint8_t k )
{
}

inline void Pic14eCore::MOVWI( uint8_t n, uint8_t k )
{
}

void Pic14eCore::runDecoder( uint16_t instr )
{
    if( (instr & 0x3FC0) == 0 )  // Miscellaneous instrs
    {
        if( (instr & 0x0030) == 0 ){
            if     ( instr == 0x0001 ) reset(); // RESET 00 0000 0000 0001
            else if( instr == 0x000A ) CALLW(); // CALLW 00 0000 0000 1010
            else if( instr == 0x000B ) BRW();   // BRW   00 0000 0000 1011
        }
        else if( (instr & 0x0030) == 1 )
        {
            uint8_t n = instr & 1<<2;
            if( (instr & 0x0008) == 0 ){
                switch( instr & 0x0003) {
                    case 0: MOVIW_pF( n ); return; // MOVIW ++FSRn 00 0000 0001 0n00
                    case 1: MOVIW_nF( n ); return; // MOVIW −−FSRn 00 0000 0001 0n01
                    case 2: MOVIW_Fp( n ); return; // MOVIW FSRn++ 00 0000 0001 0n10
                    case 3: MOVIW_Fn( n ); return; // MOVIW FSRn−− 00 0000 0001 0n11
                }
            }
            else if( (instr & 0x0008) == 1 ){
                switch( instr & 0x0003) {
                    case 0: MOVWI_pF( n ); return; // MOVWI ++FSRn 00 0000 0001 1n00
                    case 1: MOVWI_nF( n ); return; // MOVWI −−FSRn 00 0000 0001 1n01
                    case 2: MOVWI_Fp( n ); return; // MOVWI FSRn++ 00 0000 0001 1n10
                    case 3: MOVWI_Fn( n ); return; // MOVWI FSRn−− 00 0000 0001 1n11
                }
            }
        }
        else if( (instr & 0x0020) == 1 ) MOVLB( instr & 0x1F ); // MOVLB k 00 0000 001k kkkk
    }
    else if( (instr & 0x3000) == 0x3000 ){
        uint8_t d = instr & 1<<7;
        uint8_t f = instr & 0x007F;
        // ALU operations: dest ← OP(f,W)
        switch( instr & 0x3F00 ) {
            case 0x3500: LSLF( f, d );   return; // LSLF   f,d 11 0101 dfff ffff
            case 0x3600: LSRF( f, d );   return; // LSRF   f,d 11 0110 dfff ffff
            case 0x3700: ASRF( f, d );   return; // ASRF   f,d 11 0111 dfff ffff
            case 0x3B00: SUBWFB( f, d ); return; // SUBWFB f,d 11 1011 dfff ffff
            case 0x3D00: ADDWFC( f, d ); return; // ADDWFC f,d 11 1101 dfff ffff
        }
        uint8_t n = instr & 1<<6;
        // Operations with literal k
        switch( instr & 0x3F80 ) {
            case 0x3100: ADDFSR( n, instr & 0x7F ); return; // ADDFSR FSRn,k 11 0001 0nkk kkkk
            case 0x3180: MOVLP( instr & 0x1F );     return; // MOVLP       k 11 0001 1kkk kkkk
            case 0x3F00: MOVIW( n, instr & 0x7F );  return; // MOVIW k[FSRn] 11 1111 0nkk kkkk
            case 0x3F80: MOVWI( n, instr & 0x7F );  return; // MOVWI k[FSRn] 11 1111 1nkk kkkk
        }
        if( (instr & 0x3C00) == 0x3200 ) BRA( instr & 0x1FF); // BRA k 11 001k kkkk kkkk
    }
    PicMrCore::runDecoder( instr );
}

