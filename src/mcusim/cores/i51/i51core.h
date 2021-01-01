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

#ifndef I51CORE_H
#define I51CORE_H

#include "corecpu.h"

// SFR register locations
enum SFR_REGS
{
    REG_ACC  = 0xE0,
    REG_B    = 0xF0,
    REG_PSW  = 0xD0,
    _SP   = 0x81,
    REG_DPL  = 0x82,
    REG_DPH  = 0x83,
    REG_P0   = 0x80,
    REG_P1   = 0x90,
    REG_P2   = 0xA0,
    REG_P3   = 0xB0,
    REG_IP   = 0xB8,
    REG_IE   = 0xA8,
    REG_TMOD = 0x89,
    REG_TCON = 0x88,
    REG_TH0  = 0x8C,
    REG_TL0  = 0x8A,
    REG_TH1  = 0x8D,
    REG_TL1  = 0x8B,
    REG_SCON = 0x98,
    REG_PCON = 0x87
};

enum PSW_BITS
{
    PSW_P = 0,
    PSW_UNUSED = 1,
    PSW_OV = 2,
    PSW_RS0 = 3,
    PSW_RS1 = 4,
    PSW_F0 = 5,
    PSW_AC = 6,
    PSW_C = 7
};

enum PSW_MASKS
{
    PSWMASK_P = 0x01,
    PSWMASK_UNUSED = 0x02,
    PSWMASK_OV = 0x04,
    PSWMASK_RS0 = 0x08,
    PSWMASK_RS1 = 0x10,
    PSWMASK_F0 = 0x20,
    PSWMASK_AC = 0x40,
    PSWMASK_C = 0x80
};

enum IE_MASKS
{
    IEMASK_EX0 = 0x01,
    IEMASK_ET0 = 0x02,
    IEMASK_EX1 = 0x04,
    IEMASK_ET1 = 0x08,
    IEMASK_ES  = 0x10,
    IEMASK_ET2 = 0x20,
    IEMASK_UNUSED = 0x40,
    IEMASK_EA  = 0x80
};

enum PT_MASKS
{
    PTMASK_PX0 = 0x01,
    PTMASK_PT0 = 0x02,
    PTMASK_PX1 = 0x04,
    PTMASK_PT1 = 0x08,
    PTMASK_PS  = 0x10,
    PTMASK_PT2 = 0x20,
    PTMASK_UNUSED1 = 0x40,
    PTMASK_UNUSED2 = 0x80
};

enum TCON_MASKS
{
    TCONMASK_IT0 = 0x01,
    TCONMASK_IE0 = 0x02,
    TCONMASK_IT1 = 0x04,
    TCONMASK_IE1 = 0x08,
    TCONMASK_TR0 = 0x10,
    TCONMASK_TF0 = 0x20,
    TCONMASK_TR1 = 0x40,
    TCONMASK_TF1 = 0x80
};

enum TMOD_MASKS
{
    TMODMASK_M0_0 = 0x01,
    TMODMASK_M1_0 = 0x02,
    TMODMASK_CT_0 = 0x04,
    TMODMASK_GATE_0 = 0x08,
    TMODMASK_M0_1 = 0x10,
    TMODMASK_M1_1 = 0x20,
    TMODMASK_CT_1 = 0x40,
    TMODMASK_GATE_1 = 0x80
};

enum IP_MASKS
{
    IPMASK_PX0 = 0x01,
    IPMASK_PT0 = 0x02,
    IPMASK_PX1 = 0x04,
    IPMASK_PT1 = 0x08,
    IPMASK_PS  = 0x10,
    IPMASK_PT2 = 0x20
};

enum EM8051_EXCEPTION
{
    EXCEPTION_STACK,  // stack address > 127 with no upper memory, or roll over
    EXCEPTION_ACC_TO_A, // acc-to-a move operation; illegal (acc-to-acc is ok, a-to-acc is ok..)
    EXCEPTION_IRET_PSW_MISMATCH, // psw not preserved over interrupt call (doesn't care about P, F0 or UNUSED)
    EXCEPTION_IRET_SP_MISMATCH,  // sp not preserved over interrupt call
    EXCEPTION_IRET_ACC_MISMATCH, // acc not preserved over interrupt call
    EXCEPTION_ILLEGAL_OPCODE     // for the single 'reserved' opcode in the architecture
};

#define BAD_VALUE 0x77
#define PSW       m_psw[0]
#define ACC       m_acc[0]

#define OPERAND1  m_progMem[PC+1]
#define OPERAND2  m_progMem[PC+2]

#define BANK_SELECT ( (PSW & (PSWMASK_RS0|PSWMASK_RS1))>>PSW_RS0 )
#define INDIR_RX_ADDRESS ( m_dataMem[(m_opcode & 1) + 8*BANK_SELECT] )
#define RX_ADDRESS ( (m_opcode & 7) + 8*BANK_SELECT )

#define CARRY ((PSW & PSWMASK_C) >> PSW_C)

class MAINMODULE_EXPORT I51Core : public CoreCpu
{
    public:
        I51Core( eMcu* mcu );
        ~I51Core();

        virtual void reset();
        virtual void runDecoder();

    protected:
        uint16_t m_opcode;
        uint8_t* m_psw;
        uint8_t* m_acc;

        std::vector<uint16_t> m_outPortAddr;
        std::vector<uint16_t> m_inPortAddr;

        bool m_upperData;

        typedef void (I51Core::*funcPtr)();
        std::vector<funcPtr> m_instructions;

        /*template <typename T>
        void insertIntr( uint16_t opcode, T* core, funcPtr func)
        {
            m_instructions.at( opcode ) = func;
        }

        void createInstructions();*/

        inline uint8_t getValue( uint16_t addr ) // Read Port Input instead Latch
        {
            for( uint i=0; i<m_outPortAddr.size(); ++i )
            {
                if( addr == m_outPortAddr[i] )
                {
                    addr = m_inPortAddr[i];
                    break;
                }
            }
            return getRam( addr );
        }

        inline uint8_t getRam( uint16_t addr )
        {
            if( addr > m_lowDataMemEnd )
            {
                if( m_upperData ) addr += m_regEnd;
            }
            return GET_RAM( addr );
        }
        inline void setRam( uint16_t addr , uint8_t val )
        {
            if( addr > m_lowDataMemEnd )
            {
                if( m_upperData ) addr += m_regEnd;
            }
            SET_RAM( addr, val );
        }

        //void push_to_stack( int aValue );
        //int  pop_from_stack();
        inline void    pushStack8( uint8_t v );
        inline uint8_t popStack8();

        inline void add_solve_flags(int value1, int value2, int acc );
        inline void sub_solve_flags( int value1, int value2 );

        inline void ajmp_offset();
        inline void ljmp_address();
        inline void rr_a();
        inline void inc_a();
        inline void inc_mem();
        inline void inc_indir_rx();
        inline void jbc_bitaddr_offset();
        inline void acall_offset();
        inline void lcall_address();
        inline void rrc_a();
        inline void dec_a();
        inline void dec_mem();
        inline void dec_indir_rx();
        inline void jb_bitaddr_offset();
        inline void ret();
        inline void rl_a();
        inline void add_a_imm();
        inline void add_a_mem();
        inline void add_a_indir_rx();
        inline void jnb_bitaddr_offset();
        inline void reti();
        inline void rlc_a();
        inline void addc_a_imm();
        inline void addc_a_mem();
        inline void addc_a_indir_rx();
        inline void jc_offset();
        inline void orl_mem_a();
        inline void orl_mem_imm();
        inline void orl_a_imm();
        inline void orl_a_mem();
        inline void orl_a_indir_rx();
        inline void jnc_offset();
        inline void anl_mem_a();
        inline void anl_mem_imm();
        inline void anl_a_imm();
        inline void anl_a_mem();
        inline void anl_a_indir_rx();
        inline void jz_offset();
        inline void xrl_mem_a();
        inline void xrl_mem_imm();
        inline void xrl_a_imm();
        inline void xrl_a_mem();
        inline void xrl_a_indir_rx();
        inline void jnz_offset();
        inline void orl_c_bitaddr();
        inline void jmp_indir_a_dptr();
        inline void mov_a_imm();
        inline void mov_mem_imm();
        inline void mov_indir_rx_imm();
        inline void sjmp_offset();
        inline void anl_c_bitaddr();
        inline void movc_a_indir_a_pc();
        inline void div_ab();
        inline void mov_mem_mem();
        inline void mov_mem_indir_rx();
        inline void mov_dptr_imm();
        inline void mov_bitaddr_c() ;
        inline void movc_a_indir_a_dptr();
        inline void subb_a_imm();
        inline void subb_a_mem() ;
        inline void subb_a_indir_rx();
        inline void orl_c_compl_bitaddr();
        inline void mov_c_bitaddr() ;
        inline void inc_dptr();
        inline void mul_ab();
        inline void mov_indir_rx_mem();
        inline void anl_c_compl_bitaddr();
        inline void cpl_bitaddr();
        inline void cpl_c();
        inline void cjne_a_imm_offset();
        inline void cjne_a_mem_offset();
        inline void cjne_indir_rx_imm_offset();
        inline void push_mem();
        inline void clr_bitaddr();
        inline void clr_c();
        inline void swap_a();
        inline void xch_a_rx();
        //inline void xch_a_mem();
        //inline void xch_a_indir_rx();
        inline void xch( uint16_t addr );
        inline void pop_mem();
        inline void setb_bitaddr();
        inline void setb_c();
        inline void da_a();
        inline void djnz_mem_offset();
        inline void xchd_a_indir_rx();
        inline void movx_a_indir_dptr();
        inline void movx_a_indir_rx();
        inline void clr_a();
        inline void mov_a_mem();
        inline void mov_a_indir_rx();
        inline void movx_indir_dptr_a();
        inline void movx_indir_rx_a();
        inline void cpl_a();
        inline void mov_mem_a();
        inline void mov_indir_rx_a();
        inline void nop();
        inline void inc_rx();
        inline void dec_rx();
        inline void add_a_rx();
        inline void addc_a_rx();
        inline void orl_a_rx();
        inline void anl_a_rx();
        inline void xrl_a_rx();
        inline void mov_rx_imm();
        inline void mov_mem_rx();
        inline void subb_a_rx();
        inline void mov_rx_mem();
        inline void cjne_rx_imm_offset();
        inline void djnz_rx_offset();
        inline void mov_a_rx();
        inline void mov_rx_a();
};

#endif
