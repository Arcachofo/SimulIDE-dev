/***************************************************************************
 *   Copyright (C) 2020 by Santiago González                               *
 *                                                                         *
 ***( see copyright.txt file at root folder )*******************************/

#ifndef PICMRCORE_H
#define PICMRCORE_H

#include "mcucpu.h"

enum {
    C=0,DC,Z,PD,TO,RP0,RP1,IRP
};

class MAINMODULE_EXPORT PicMrCore : public McuCpu
{
    public:
        PicMrCore( eMcu* mcu );
        ~PicMrCore();

        virtual void reset();
        virtual void runStep() override;

        virtual uint RET_ADDR() override { return m_stack[m_sp]; }

    protected:
        virtual void runStep( uint16_t instr );
        uint8_t* m_Wreg;
        uint8_t* m_OPTION;

        regBits_t m_bankBits;
        uint16_t  m_bank;

        uint16_t m_PCLaddr;
        uint16_t m_PCHaddr;

        uint32_t m_stack[8];
        uint8_t  m_sp;
        uint8_t  m_stackSize;

        virtual void setBank( uint8_t bank );

        void incDefault()
        {
            setPC( m_PC+1 );
            m_mcu->cyclesDone += 1;
        }

        virtual void PUSH_STACK( uint32_t addr ) override // Harware circular Stack
        {
            m_stack[m_sp] = addr;
            m_sp++;
            if( m_sp == m_stackSize ) m_sp = 0;
        }
        virtual uint32_t POP_STACK() override // Hardware circular Stack
        {
            if( m_sp == 0 ) m_sp = m_stackSize-1;
            else            m_sp--;
            return m_stack[m_sp];
        }

        virtual void setPC( uint32_t pc ) override
        {
            m_PC = pc;
            m_dataMem[ m_PCLaddr] = m_PC & 0xFF;
        }

        void setValue( uint8_t newV, uint8_t f, uint8_t d )
        {
            if( d ) SET_RAM( f, newV );
            else    *m_Wreg = newV;
        }
        void setValueZ( uint8_t newV, uint8_t f, uint8_t d )
        {
            setValue( newV, f, d );
            write_S_Bit( Z, newV==0 );
        }

        uint8_t add( uint8_t val1, uint8_t val2 );
        uint8_t sub( uint8_t val1, uint8_t val2 );

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
