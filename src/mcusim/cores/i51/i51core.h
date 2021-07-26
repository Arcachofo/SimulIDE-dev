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
    REG_SBUF = 0x99,
    REG_B    = 0xF0,
    REG_DPL  = 0x82,
    REG_DPH  = 0x83,
};

enum {
    P=0,k,OV,RS0,RS1,F0,AC,CY
};

/*enum EM8051_EXCEPTION
{
    EXCEPTION_STACK,  // stack address > 127 with no upper memory, or roll over
    EXCEPTION_ACC_TO_A, // acc-to-a move operation; illegal (acc-to-acc is ok, a-to-acc is ok..)
    EXCEPTION_IRET_PSW_MISMATCH, // psw not preserved over interrupt call (doesn't care about P, F0 or UNUSED)
    EXCEPTION_IRET_SP_MISMATCH,  // sp not preserved over interrupt call
    EXCEPTION_IRET_ACC_MISMATCH, // acc not preserved over interrupt call
    EXCEPTION_ILLEGAL_OPCODE     // for the single 'reserved' opcode in the architecture
};*/



class MAINMODULE_EXPORT I51Core : public CoreCpu
{
    public:
        I51Core( eMcu* mcu );
        ~I51Core();

        virtual void reset();
        virtual void runDecoder();

    protected:
        uint16_t m_opcode;
        uint8_t* m_acc;

        /*uint8_t m_P;
        uint8_t m_OV;
        uint8_t m_RS0;
        uint8_t m_RS1;
        uint8_t m_F0;
        uint8_t m_AC;
        uint8_t m_CY;*/

        bool m_upperData;

        std::vector<uint16_t> m_outPortAddr;
        std::vector<uint16_t> m_inPortAddr;

        /*typedef void (I51Core::*funcPtr)();
        std::vector<funcPtr> m_instructions;

        template <typename T>
        void insertIntr( uint16_t opcode, T* core, funcPtr func)
        {
            m_instructions.at( opcode ) = func;
        }

        void createInstructions();*/

        inline uint8_t getValue( uint16_t addr ) // Read Fake Input instead
        {
            if( addr == REG_SBUF ) addr++;     // Fake Uart Input Register
            else
            {
                for( uint i=0; i<m_outPortAddr.size(); ++i )
                {
                    if( addr == m_outPortAddr[i] )
                    {
                        addr = m_inPortAddr[i]; // Fake Port Input Register
                        break;
                    }
                }
            }
            return GET_RAM( addr );
        }

        inline uint8_t GET_RAM( uint16_t addr ) override
        {
            if( m_upperData )
            {
                if( addr > m_lowDataMemEnd ) addr += m_regEnd;
            }
            return CoreCpu::GET_RAM( addr );
        }
        inline void SET_RAM( uint16_t addr , uint8_t val )
        {
            if( m_upperData )
            {
                if( addr > m_lowDataMemEnd ) addr += m_regEnd;
            }
            CoreCpu::SET_RAM( addr, val );
        }

        //void push_to_stack( int aValue );
        //int  pop_from_stack();
        inline void    pushStack8( uint8_t v );
        inline uint8_t popStack8();

        inline void add_solve_flags( uint8_t value1, uint8_t value2, uint8_t acc );
        inline void sub_solve_flags( uint8_t value1, uint8_t value2 );

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
