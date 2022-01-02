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

#ifndef PICMRCORE_H
#define PICMRCORE_H

#include "mcucore.h"

enum {
    C=0,DC,Z,PD,TO,RP0,RP1,IRP
};

class MAINMODULE_EXPORT PicMrCore : public McuCore
{
    public:
        PicMrCore( eMcu* mcu );
        ~PicMrCore();

        virtual void reset();
        virtual void runDecoder() override;

    protected:
        virtual void runDecoder( uint16_t instr );
        uint8_t  m_Wreg;
        uint8_t* m_FSR;
        uint8_t* m_OPTION;

        regBits_t m_bankBits;
        uint16_t  m_bank;

        uint16_t m_PCLaddr;
        uint16_t m_PCHaddr;

        uint32_t m_stack[8];
        uint8_t  m_sp;
        uint8_t  m_stackSize;

        virtual void setBank( uint8_t bank );

        virtual uint8_t GET_RAM( uint16_t addr ) override //
        {
            addr = m_mcu->getMapperAddr( addr+m_bank );
            if( addr == 0 ) addr = *m_FSR;// INDF
            return McuCore::GET_RAM( addr );
        }
        virtual void SET_RAM( uint16_t addr, uint8_t v ) override //
        {
            if( addr == m_PCLaddr )                  // Writting to PCL
                setPC( v + (m_dataMem[m_PCHaddr]<<8) );

            else if( addr == 0 ) addr = *m_FSR;      // INDF

            addr = m_mcu->getMapperAddr( addr+m_bank );

            McuCore::SET_RAM( addr, v );
        }

        virtual void PUSH_STACK( uint32_t addr ) override // Harware circular Stack
        {
            m_stack[m_sp] = addr;
            m_sp++;
            if( m_sp == m_stackSize ) m_sp = 0;
        }
        virtual uint32_t POP_STACK() override // Harware circular Stack
        {
            if( m_sp == 0 ) m_sp = m_stackSize-1;
            else            m_sp--;
            return m_stack[m_sp];
        }

        virtual void setPC( uint32_t pc ) override
        {
            PC = pc;
            m_dataMem[ m_PCLaddr] = PC & 0xFF;
        }

        inline void setValue( uint8_t newV, uint8_t f, uint8_t d );
        inline void setValueZ( uint8_t newV, uint8_t f, uint8_t d );

        inline uint8_t add( uint8_t val1, uint8_t val2 );
        inline uint8_t sub( uint8_t val1, uint8_t val2 );

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
        inline void CALL( uint16_t k );
        inline void GOTO( uint16_t k );

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
