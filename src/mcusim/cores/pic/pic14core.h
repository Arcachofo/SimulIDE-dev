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

#ifndef PIC14CORE_H
#define PIC14CORE_H

#include "corecpu.h"

enum {
    C=0,DC,Z,PD,TO,RP0,RP1,IRP
};

class MAINMODULE_EXPORT Pic14Core : public CoreCpu
{
    public:
        Pic14Core( eMcu* mcu );
        ~Pic14Core();

        virtual void reset();
        virtual void runDecoder();

    protected:
        uint8_t  m_Wreg;

        regBits_t m_bankBits;
        uint16_t  m_bank;

        uint32_t m_stack[8];
        uint8_t  m_sp;

        /*uint8_t m_C;
        uint8_t m_DC;
        uint8_t m_Z;
        uint8_t m_PD;
        uint8_t m_TO;
        uint8_t m_RP0;
        uint8_t m_RP1;
        uint8_t m_IRP;*/

        std::vector<uint16_t> m_outPortAddr;
        std::vector<uint16_t> m_inPortAddr;

        void setBank( uint8_t bank )
        {
            m_bank = getRegBitsVal( bank, m_bankBits );
            m_bank <<= 7;
        }

        virtual uint8_t GET_RAM( uint16_t addr ) override //
        {
            addr = m_mcu->getMapperAddr( addr+m_bank );

            for( uint i=0; i<m_outPortAddr.size(); ++i )
            {
                if( addr == m_outPortAddr[i] )
                {
                    addr = m_inPortAddr[i]; // Fake Port Input Register
                    break;
                }
            }
            return CoreCpu::GET_RAM( addr );
        }
        virtual void SET_RAM( uint16_t addr, uint8_t v ) override //
        {
            addr = m_mcu->getMapperAddr( addr+m_bank );
            CoreCpu::SET_RAM( addr, v );
        }

        virtual void PUSH_STACK( uint32_t addr ) override // Harware circular Stack
        {
            m_stack[m_sp] = addr;
            m_sp++;
            if( m_sp == 8 ) m_sp = 0;
        }
        virtual uint32_t POP_STACK() override // Harware circular Stack
        {
            if( m_sp == 0 ) m_sp = 7;
            else            m_sp--;
            return m_stack[m_sp];
        }

        inline void setValue( uint8_t newV, uint8_t f, uint8_t d );
        inline void setValueZ( uint8_t newV, uint8_t f, uint8_t d );

        inline void setAddFlags( uint8_t oldV, uint8_t newV, uint8_t src2 );
        inline void setSubFlags( uint8_t oldV, uint8_t newV, uint8_t src2 );

        // Miscellaneous instructions
        inline void RETURN();
        inline void RETFIE();
        inline void OPTION();
        inline void SLEEP();
        inline void CLRWDT();
        //inline void TRIS( uint8_t f );

        // ALU operations: dest ← OP(f,W)
        inline void MOVWF( uint8_t f );
        inline void CLRF( uint8_t f );
        inline void SUBWF( uint8_t f, uint8_t d );
        inline void DECF( uint8_t f, uint8_t d );
        inline void IORWF( uint8_t f, uint8_t d );
        inline void ANDWF( uint8_t f, uint8_t d );
        inline void XORWF( uint8_t f, uint8_t d );
        inline void ADDWF( uint8_t f, uint8_t d );
        inline void MOVF( uint8_t f, uint8_t d );
        inline void COMF( uint8_t f, uint8_t d );
        inline void INCF( uint8_t f, uint8_t d );
        inline void DECFSZ( uint8_t f, uint8_t d );
        inline void RRF( uint8_t f, uint8_t d );
        inline void RLF( uint8_t f, uint8_t d );
        inline void SWAPF( uint8_t f, uint8_t d );
        inline void INCFSZ( uint8_t f, uint8_t d );

        // Bit operations
        inline void BCF( uint8_t f, uint8_t b );
        inline void BSF( uint8_t f, uint8_t b );
        inline void BTFSC( uint8_t f, uint8_t b );
        inline void BTFSS( uint8_t f, uint8_t b );

        // Control transfers
        inline void CALL( uint8_t k );
        inline void GOTO( uint8_t k );

        // Operations with W and 8-bit literal: W ← OP(k,W)
        inline void MOVLW( uint8_t k );
        inline void RETLW( uint8_t k );
        inline void IORLW( uint8_t k );
        inline void ANDLW( uint8_t k );
        inline void XORLW( uint8_t k );
        inline void SUBLW( uint8_t k );
        inline void ADDLW( uint8_t k );
};

#endif
