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
    m_sp = 0;

    QHash<QString, McuPort*>  ports = m_mcu->getPorts();
    for( QString portName : ports.keys() )
    {
        McuPort* port = ports.value( portName );
        m_outPortAddr.emplace_back( port->getOutAddr() );
        m_inPortAddr.emplace_back( port->getInAddr() );
    }
    m_bankBits = mcu->getRegBits( "RP0,RP1" );
    mcu->watchBitNames( "RP0,RP1", R_WRITE, this, &Pic14Core::setBank );
}
Pic14Core::~Pic14Core() {}

void Pic14Core::reset()
{
    CoreCpu::reset();

    m_Wreg = 0;
}

inline void Pic14Core::setValue( uint8_t newV, uint8_t f, uint8_t d )
{
    if( d ) SET_RAM( f, newV );
    else    m_Wreg = newV;

    incDefault();
}
inline void Pic14Core::setValueZ( uint8_t newV, uint8_t f, uint8_t d )
{
    setValue( newV, f, d );
    write_S_Bit( Z, newV==0 );
}

inline void Pic14Core::setAddFlags( uint8_t oldV, uint8_t newV, uint8_t src2 )
{
    write_S_Bit( Z, newV==0 );
    write_S_Bit( C, newV & 0x100 );
    write_S_Bit( DC, (newV^oldV^src2) & 0x10 );
}

inline void Pic14Core::setSubFlags( uint8_t oldV, uint8_t newV, uint8_t src2 )
{
    write_S_Bit( Z, newV==0 );
    write_S_Bit( C, newV & 0x100 );
    write_S_Bit( DC, (newV^oldV^src2) & 0x10 );
}


// Miscellaneous instructions

inline void Pic14Core::RETURN()
{
    RET();
}

inline void Pic14Core::RETFIE()
{
    RETI();
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


// ALU operations: dest ← OP(f,W)

inline void Pic14Core::MOVWF( uint8_t f )
{
    SET_RAM( f, m_Wreg);
    incDefault();
}

inline void Pic14Core::CLRF( uint8_t f )
{
    SET_RAM( f, 0 );
    incDefault();
}

inline void Pic14Core::SUBWF( uint8_t f, uint8_t d )
{
    uint8_t oldV = GET_RAM( f ) ;
    uint8_t newV = oldV - m_Wreg;

    setValue( newV, f, d );
    setSubFlags( oldV, newV, m_Wreg );
}

inline void Pic14Core::DECF( uint8_t f, uint8_t d )
{
    uint8_t newV = GET_RAM( f );
    setValueZ( newV--, f, d );
}

inline void Pic14Core::IORWF( uint8_t f, uint8_t d )
{
    uint8_t oldV = GET_RAM( f ) ;
    uint8_t newV = oldV | m_Wreg;

    setValueZ( newV, f, d );
}

inline void Pic14Core::ANDWF( uint8_t f, uint8_t d )
{
    uint8_t oldV = GET_RAM( f ) ;
    uint8_t newV = oldV & m_Wreg;

    setValueZ( newV, f, d );
}

inline void Pic14Core::XORWF( uint8_t f, uint8_t d )
{
    uint8_t oldV = GET_RAM( f ) ;
    uint8_t newV = oldV ^ m_Wreg;

    setValueZ( newV, f, d );
}

inline void Pic14Core::ADDWF( uint8_t f, uint8_t d )
{
    uint8_t oldV = GET_RAM( f ) ;
    uint8_t newV = oldV + m_Wreg;

    setValue( newV, f, d );
    setAddFlags( oldV, newV, m_Wreg );
}

inline void Pic14Core::MOVF( uint8_t f, uint8_t d )
{
    uint8_t newV = GET_RAM( f );
    setValueZ( newV, f, d );
}

inline void Pic14Core::COMF( uint8_t f, uint8_t d )
{
    uint8_t newV = GET_RAM( f ) ^ 0xFF;
    setValueZ( newV, f, d );
}

inline void Pic14Core::INCF( uint8_t f, uint8_t d )
{
    uint8_t newV = GET_RAM( f );
    setValueZ( ++newV, f, d );
}

inline void Pic14Core::DECFSZ( uint8_t f, uint8_t d )
{
    uint8_t newV = GET_RAM( f );
    setValue( --newV, f, d );
    if( newV == 0 ) incDefault();
}

inline void Pic14Core::RRF( uint8_t f, uint8_t d )
{
    uint8_t oldV = GET_RAM( f ) ;
    uint8_t newV = oldV >> 1;
    setValue( newV, f, d );
    write_S_Bit( C, oldV & 1 );
}

inline void Pic14Core::RLF( uint8_t f, uint8_t d )
{
    uint8_t oldV = GET_RAM( f ) ;
    uint8_t newV = oldV << 1;
    setValue( newV, f, d );
    write_S_Bit( C, oldV & 0x80 );
}

inline void Pic14Core::SWAPF( uint8_t f, uint8_t d )
{
    uint8_t oldV = GET_RAM( f );
    uint8_t newV = ((oldV >> 4) & 0x0F) | ((oldV << 4) & 0xF0);
    setValue( newV, f, d );
}

inline void Pic14Core::INCFSZ( uint8_t f, uint8_t d )
{
    uint8_t newV = GET_RAM( f );
    setValue( newV++, f, d );
    if( newV == 0 ) incDefault();
}


// Bit operations

inline void Pic14Core::BCF( uint8_t f, uint8_t b )
{
    uint8_t newV = GET_RAM( f );
    newV &= ~(1<<b);

    SET_RAM( f, newV );
    incDefault();
}

inline void Pic14Core::BSF( uint8_t f, uint8_t b )
{
    uint8_t newV = GET_RAM( f );
    newV |= 1<<b;

    SET_RAM( f, newV );
    incDefault();
}

inline void Pic14Core::BTFSC( uint8_t f, uint8_t b )
{
    uint8_t oldV = GET_RAM( f );
    uint8_t bitMask = 1<<b;

    if( (oldV & bitMask) == 0 ) incDefault();
    incDefault();
}

inline void Pic14Core::BTFSS( uint8_t f, uint8_t b )
{
    uint8_t oldV = GET_RAM( f );
    uint8_t bitMask = 1<<b;

    if( oldV & bitMask  ) incDefault();
    incDefault();
}


// Control transfers

inline void Pic14Core::CALL( uint8_t k )
{
    PC++;
    CALL_ADDR( k );
}

inline void Pic14Core::GOTO( uint8_t k )
{
    PC = k;
    m_mcu->cyclesDone = 2;
}

// Operations with W and 8-bit literal: W ← OP(k,W)

inline void Pic14Core::MOVLW( uint8_t k )
{
    m_Wreg = k;
    incDefault();
}

inline void Pic14Core::RETLW( uint8_t k )
{
    m_Wreg = k;
    RETURN();
}

inline void Pic14Core::IORLW( uint8_t k )
{
    m_Wreg |= k;
    write_S_Bit( Z, m_Wreg==0 );
    incDefault();
}

inline void Pic14Core::ANDLW( uint8_t k )
{
    m_Wreg &= k;
    write_S_Bit( Z, m_Wreg==0 );
    incDefault();
}

inline void Pic14Core::XORLW( uint8_t k )
{
    m_Wreg ^= k;
    write_S_Bit( Z, m_Wreg==0 );
    incDefault();
}

inline void Pic14Core::SUBLW( uint8_t k )
{
    m_Wreg -= k;
    write_S_Bit( Z, m_Wreg==0 );
    incDefault();
}

inline void Pic14Core::ADDLW( uint8_t k )
{
    m_Wreg += k;
    write_S_Bit( Z, m_Wreg==0 );
    incDefault();
}

void Pic14Core::runDecoder()
{
    uint16_t instr = m_progMem[PC] & 0x3FFF;

    if( (instr & 0x3F80) == 0 )  // Miscellaneous instrs
    {
        switch( instr & 0x000C)
        {
            case 0x0008: {
                if     ( instr == 0x0008 ) RETURN(); // RETURN 00 0000 0000 1000
                else if( instr == 0x0009 ) RETFIE();// RETFIE 00 0000 0000 1001
            } break;
            case 0x0000: {
                if     ( instr == 0x0062 ) OPTION();// OPTION 00 0000 0110 0010
                else if( instr == 0x0063 ) SLEEP();// SLEEP 00 0000 0110 0011
            } break;
            case 0x0004: {
                if     ( instr == 0x0064 ) CLRWDT();// CLRWDT 00 0000 0110 0100
            } break;
        }
    }
    else if( (instr & 0x3000) == 0 ) // ALU operations: dest ← OP(f,W)
    {
        uint8_t f = instr & 0x7F;
        uint8_t d = instr>>7 & 1;

        if( (instr & 0x3800) == 0 ) {
            switch( instr & 0x0700) {
                case 0x0000: MOVWF( f );    break; // MOVWF f   00 0000 1fff ffff
                case 0x0100: CLRF( f );     break; // CLR   f   00 0001 1fff ffff
                case 0x0200: SUBWF( f, d ); break; // SUBWF f,d 00 0010 dfff ffff
                case 0x0300: DECF( f, d );  break; // DECF  f,d 00 0011 dfff ffff
                case 0x0400: IORWF( f, d ); break; // IORWF f,d 00 0100 dfff ffff
                case 0x0500: ANDWF( f, d ); break; // ANDWF f,d 00 0101 dfff ffff
                case 0x0600: XORWF( f, d ); break; // XORWF f,d 00 0110 dfff ffff
                case 0x0700: ADDWF( f, d ); break; // ADDWF f,d 00 0111 dfff ffff
           }
        } else {
            switch( instr & 0x0700) {
                case 0x0000: MOVF( f, d );   break; // MOVF   f,d 00 1000 dfff ffff
                case 0x0100: COMF( f, d );   break; // COMF   f,d 00 0001 dfff ffff
                case 0x0200: INCF( f, d );   break; // INCF   f,d 00 0010 dfff ffff
                case 0x0300: DECFSZ( f, d ); break; // DECFSZ f,d 00 0011 dfff ffff
                case 0x0400: RRF( f, d );    break; // RRF    f,d 00 0100 dfff ffff
                case 0x0500: RLF( f, d );    break; // RLF    f,d 00 0101 dfff ffff
                case 0x0600: SWAPF( f, d );  break; // SWAPF  f,d 00 0110 dfff ffff
                case 0x0700: INCFSZ( f, d ); break; // INCFSZ f,d 00 0111 dfff ffff
            }
        }
    } else {
        if( (instr & 0x3000) == 0x1000 ) // Bit operations
        {
            uint8_t f = instr & 0x7F;
            uint8_t b = instr>>7 & 7;

            switch( instr & 0x3C00){
                case 0x1000: BCF( f, b );   break; // BCF   f,b 01 00bb bkkk kkkk
                case 0x1400: BSF( f, b );   break; // BSF   f,b 01 01bb bkkk kkkk
                case 0x1800: BTFSC( f, b ); break; // BTFSC f,b 01 10bb bkkk kkkk
                case 0x1C00: BTFSS( f, b ); break; // BTFSS f,b 01 11bb bkkk kkkk
            }
        }
        else if( (instr & 0x3000) == 0x2000 ) // Control transfers
        {
            uint16_t k = instr & 0x07FF;

            if( (instr & 0x0800) == 0 ) CALL( k ); // CALL k 10 0kkk kkkk kkkk
            else                        GOTO( k ); // GOTO k 10 1kkk kkkk kkkk
        }
        else if( (instr & 0x3000) == 0x3000 ) // Operations with W and 8-bit literal: W ← OP(k,W)
        {
            uint8_t k = instr & 0xFF;

            switch( instr & 0x3C00){
                case 0x3000: MOVLW( k ); break; // MOVLW k 11 00xx kkkk kkkk
                case 0x3400: RETLW( k ); break; // RETLW k 11 01xx kkkk kkkk
                case 0x3800: {
                    switch( instr & 0x3F00) {
                        case 0x3800: IORLW( k ); break; // IORLW k 11 1000 kkkk kkkk
                        case 0x3900: ANDLW( k ); break; // ANDLW k 11 1001 kkkk kkkk
                        case 0x3A00: XORLW( k ); break; // XORLW k 11 1010 kkkk kkkk
                    }
                }
                case 0x3C00: {
                    if((instr & 0x0200)==0 ) SUBLW( k ); // SUBLW k 11 110x kkkk kkkk
                    else                     ADDLW( k ); // ADDLW k 11 111x kkkk kkkk
                }
            }
        }
    }
}
