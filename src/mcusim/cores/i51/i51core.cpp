/* 8051 emulator core
 * Copyright 2006 Jari Komppa
 *
 * Permission is hereby granted, free of charge, to any person obtaining
 * a copy of this software and associated documentation files( the
 * "Software" ), to deal in the Software without restriction, including
 * without limitation the rights to use, copy, modify, merge, publish,
 * distribute, sublicense, and/or sell copies of the Software, and to
 * permit persons to whom the Software is furnished to do so, subject
 * to the following conditions:
 *
 * The above copyright notice and this permission notice shall be included
 * in all copies or substantial portions of the Software.
 *
 * THE SOFTWARE IS PROVIDED "AS IS", WITHOUT WARRANTY OF ANY KIND, EXPRESS
 * OR IMPLIED, INCLUDING BUT NOT LIMITED TO THE WARRANTIES OF MERCHANTABILITY,
 * FITNESS FOR A PARTICULAR PURPOSE AND NONINFRINGEMENT. IN NO EVENT SHALL THE
 * AUTHORS OR COPYRIGHT HOLDERS BE LIABLE FOR ANY CLAIM, DAMAGES OR OTHER
 * LIABILITY, WHETHER IN AN ACTION OF CONTRACT, TORT OR OTHERWISE, ARISING
 * FROM, OUT OF OR IN CONNECTION WITH THE SOFTWARE OR THE USE OR OTHER DEALINGS
 * IN THE SOFTWARE.
 *
 *( i.e. the MIT License )
 *
 * m_opcodes.c
 * 8051 m_opcode simulation functions
 */
/***************************************************************************
 *   Modified 2020 by santiago González                                    *
 *   santigoro@gmail.com                                                   *
 *                                                                         *
 *   This program is free software; you can redistribute it and/or modify  *
 *   it under the terms of the GNU General Public License as published by  *
 *   the Free Software Foundation; either version 3 of the License, or     *
 *  ( at your option ) any later version.                                  *
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

#include "i51core.h"
#include "mcuport.h"

/// #define PSW m_sreg

#define BAD_VALUE 0x77
//#define PSW       m_sreg[0]
#define ACC       m_acc[0]

#define OPERAND1  m_progMem[PC+1]
#define OPERAND2  m_progMem[PC+2]

#define BANK_SELECT ( (STATUS(RS0)>>RS0) | (STATUS(RS1)>>RS1) ) //( (PSW & (PSWMASK_RS0|PSWMASK_RS1))>>PSW_RS0 )
#define INDIR_RX_ADDRESS ( m_dataMem[(m_opcode & 1) + 8*BANK_SELECT] )
#define RX_ADDRESS ( (m_opcode & 7) + 8*BANK_SELECT )

I51Core::I51Core( eMcu* mcu  )
       : McuCore( mcu  )
{
    m_acc = m_mcu->getReg( "ACC" );

    m_upperData = (m_dataMemEnd > m_regEnd);

    QHash<QString, McuPort*>  ports = m_mcu->getPorts();
    for( QString portName : ports.keys() )
    {
        McuPort* port = ports.value( portName );
        m_outPortAddr.emplace_back( port->getOutAddr() );
        m_inPortAddr.emplace_back( port->getInAddr() );
    }
}
I51Core::~I51Core() {}

void I51Core::reset()
{
    McuCore::reset();
}

void I51Core::pushStack8( uint8_t aValue )
{
    REG_SPL++;
    uint16_t address = REG_SPL;

    if( REG_SPL > m_lowDataMemEnd )
    {
        //if( m_upperData )  m_upperData[REG_SP] = aValue;
        if( m_upperData ) address += m_regEnd;
        else // No Upper Data
        {
            ////if( aCPU->except ) aCPU->except(EXCEPTION_STACK );
        }
    }
    m_dataMem[address] = aValue;

    ////if(( REG_SP == 0 ) && ( aCPU->except )) aCPU->except(EXCEPTION_STACK );
}

uint8_t I51Core::popStack8()
{
    uint8_t  value = BAD_VALUE;
    uint16_t address = REG_SPL;

    if( address > m_lowDataMemEnd )
    {
        if( m_upperData ) address += m_regEnd;
        ////else if( aCPU->except  ) aCPU->except(EXCEPTION_STACK );
    }
    value = m_dataMem[address];

    REG_SPL--;

    ////if(( REG_SP == 0 ) && ( aCPU->except )) aCPU->except(EXCEPTION_STACK );
    return value;
}

void I51Core::add_solve_flags( uint8_t value1, uint8_t value2, uint8_t acc )
{
    /* Carry: overflow from 7th bit to 8th bit */
    /// PSW[Cy] = ( value1+value2+acc ) >> 8;
    uint8_t cy = ((value1+value2+acc)>>1) & 1<<7;
    write_S_Bit( Cy, cy );

    /* Auxiliary carry: overflow from 3th bit to 4th bit */
    ///PSW[AC] =( (value1 & 7 )+( value2 & 7 ) + acc ) >> 3;
    write_S_Bit( AC, ((value1 & 7 )+( value2 & 7 ) + acc) & 1<<3 );

    /* Overflow: overflow from 6th or 7th bit, but not both */
    /// PSW[OV] =( ((value1 & 127 )+( value2 & 127 ) + acc ) >> 7 )^PSW[Cy];
    write_S_Bit( OV, ((value1 & 127 )+( value2 & 127 ) + acc ) ^ cy );
}

void I51Core::sub_solve_flags( uint8_t value1, uint8_t value2 )
{
    ///PSW[Cy] =( (value1-value2) >> 8 ) & 1;
    uint8_t cy = ((value1-value2)>>1) & 1<<7; //Carry: overflow from 7th bit to 8th bit
    write_S_Bit( Cy, cy );

    /// PSW[AC] =( ((value1 & 7 )-( value2 & 7 )) >> 3 ) & 1;
    write_S_Bit( AC, ((value1 & 7 )-( value2 & 7 )) & 1<<3 );

    /// PSW[OV] =( (((value1 & 127 )-( value2 & 127 )) >> 7 ) & 1 )^PSW[Cy];
    write_S_Bit( OV, ((value1 & 127 )-( value2 & 127 )) ^ cy );
}

// INSTRUCTIONS -----------------------------

void I51Core::acall_offset()
{
    int address = (( PC+2 ) & 0xf800) | OPERAND1 |( (m_opcode & 0xe0 ) << 3 );

    ///push_addr( PC+2 );
    pushStack8(( PC+2 ) & 0xff );
    pushStack8(( PC+2 ) >> 8 );

    PC = address;
    m_mcu->cyclesDone = 2;
}

void I51Core::add_a_imm()
{
    add_solve_flags( ACC, OPERAND1, 0 );
    ACC += OPERAND1;
    PC += 2;
    m_mcu->cyclesDone = 1;
}

void I51Core::add_a_mem()
{
    int value = GET_RAM( OPERAND1 );
    add_solve_flags( ACC, value, 0 );
    ACC += value;
    PC += 2;
    m_mcu->cyclesDone = 1;
}

void I51Core::add_a_indir_rx()
{
    int address = INDIR_RX_ADDRESS;

    if( address > m_lowDataMemEnd )
    {
        if( m_upperData ) address += m_regEnd ;
    }
    add_solve_flags( ACC, m_dataMem[address], 0 );
    ACC += m_dataMem[address];

    incDefault();
}

void I51Core::rlc_a()
{
    uint8_t newc = ACC & (1<<7);

    /// ACC = (ACC << 1) | PSW[Cy];
    ACC = ACC << 1;
    if( STATUS(Cy) ) ACC += 1;

    //PSW[Cy] = newc;
    write_S_Bit( Cy, newc );

    incDefault();
}

void I51Core::addc_a_imm()
{
    uint8_t carry = STATUS(Cy) >> Cy;///  PSW[Cy];

    add_solve_flags( ACC, OPERAND1, carry );

    ACC += OPERAND1 + carry;
    PC += 2;
    m_mcu->cyclesDone = 1;
}

void I51Core::addc_a_mem()
{
    uint8_t carry = STATUS(Cy) >> Cy;///  PSW[Cy];
    int value = GET_RAM( OPERAND1 );
    add_solve_flags( ACC, value, carry );
    ACC += value + carry;
    PC += 2;
    m_mcu->cyclesDone = 1;
}

void I51Core::anl_mem_a()
{
    int address = OPERAND1;
    SET_RAM( address, m_dataMem[address] &= ACC ); //if( address > m_lowDataMemEnd ) aCPU->mSFR[address - 0x80] &= ACC; //else m_dataMem[address] &= ACC;
    PC += 2;
    m_mcu->cyclesDone = 1;
}

void I51Core::anl_mem_imm()
{
    int address = OPERAND1;
    SET_RAM( address, m_dataMem[address] &= OPERAND2 ); // if( address > m_lowDataMemEnd ) aCPU->mSFR[address - 0x80] &= OPERAND2; //else m_dataMem[address] &= OPERAND2;
    PC += 3;
    m_mcu->cyclesDone = 1;
}

void I51Core::anl_a_imm()
{
    ACC &= OPERAND1;
    PC += 2;
    m_mcu->cyclesDone = 1;
}

void I51Core::anl_a_mem()
{
    int value = GET_RAM( OPERAND1 );
    ACC &= value;
    PC += 2;
    m_mcu->cyclesDone = 1;
}

void I51Core::anl_a_indir_rx()
{
    ACC &= GET_RAM( INDIR_RX_ADDRESS );
    incDefault();
}

void I51Core::anl_a_rx()
{
    int rx = RX_ADDRESS;
    ACC &= m_dataMem[rx];
    incDefault();
}

void I51Core::ajmp_offset()
{
    PC = ((PC+2) & 0xf800) | OPERAND1 | (( m_opcode & 0xe0 ) << 3 );
    m_mcu->cyclesDone = 2;
}

void I51Core::ljmp_address()
{
    PC = ( OPERAND1 << 8 ) | OPERAND2;
    m_mcu->cyclesDone = 1;
}

void I51Core::rr_a()
{
    ACC = (ACC >> 1) | (ACC << 7);
    incDefault();
}

void I51Core::inc_a()
{
    ACC++;
    incDefault();
}

void I51Core::inc_mem()
{
    int address = OPERAND1;
    SET_RAM( address, m_dataMem[address]++ );
    PC += 2;
    m_mcu->cyclesDone = 1;
}

void I51Core::inc_indir_rx()
{
    int address = INDIR_RX_ADDRESS;
    if( address > m_lowDataMemEnd )
    {
        if( m_upperData ) address += m_regEnd;
    }
    m_dataMem[address]++;
    incDefault();
}

void I51Core::lcall_address()
{
    ///push_addr( PC+3 );
    pushStack8(( PC+3 ) & 0xff );
    pushStack8(( PC+3 ) >> 8 );

    PC = (OPERAND1 << 8) | OPERAND2;

    m_mcu->cyclesDone = 2;
}

void I51Core::rrc_a()
{
    uint8_t c = STATUS(Cy); /// PSW[Cy];

    ///PSW[Cy] = ACC & 1;
    write_S_Bit( Cy, ACC & 1 );

    ACC = (ACC >> 1);
    if( c ) ACC |= c; // STATUS carry is already in bit 7

    incDefault();
}

void I51Core::dec_a()
{
    ACC--;
    incDefault();
}

void I51Core::dec_mem()
{
    int address = OPERAND1;

    SET_RAM( address, m_dataMem[address]-- );

    PC += 2;
    m_mcu->cyclesDone = 1;
}

void I51Core::dec_indir_rx()
{
    int address = INDIR_RX_ADDRESS;
    if( address > m_lowDataMemEnd )
    {
        if( m_upperData ) address += m_regEnd;
    }
    m_dataMem[address]--;

    incDefault();
}

void I51Core::ret()
{
    PC = popStack8() << 8;
    PC |= popStack8();
    m_mcu->cyclesDone = 2;
}

void I51Core::rl_a()
{
    ACC =( ACC << 1 ) | ( ACC >> 7 );
    incDefault();
}

void I51Core::jbc_bitaddr_offset()
{
    uint8_t address = OPERAND1;
    uint8_t bit = address & 7;
    uint8_t bitmask =( 1 << bit );

    address &= 0xf8;

    if( (m_dataMem[address] & bitmask ) )
    {
        m_dataMem[address] &= ~bitmask;
        PC += (int8_t)OPERAND2;
    }
    PC += 3;
    m_mcu->cyclesDone = 2;
}

void I51Core::jb_bitaddr_offset()
{
    uint8_t address = OPERAND1;
    uint8_t bit = address & 7;
    uint8_t bitmask =( 1 << bit );

    address &= 0xf8;

    if( getValue( address ) & bitmask )
        PC += (int8_t)OPERAND2;

    PC += 3;
    m_mcu->cyclesDone = 2;
}

void I51Core::jnb_bitaddr_offset()
{
    uint8_t address = OPERAND1;
    uint8_t bit = address & 7;
    uint8_t bitmask =( 1 << bit );

    address &= 0xf8;

    if( !(getValue( address ) & bitmask ) )
        PC += (int8_t)OPERAND2;

    PC += 3;
    m_mcu->cyclesDone = 2;
}

void I51Core::jc_offset()
{
    if( STATUS(Cy) ) PC += (signed char)OPERAND1;
    PC += 2;
    m_mcu->cyclesDone = 2;
}

void I51Core::reti()
{
    RETI();
}

void I51Core::addc_a_indir_rx()
{
    uint8_t carry = STATUS(Cy) >> Cy; /// PSW[Cy];

    int value = GET_RAM( INDIR_RX_ADDRESS );

    add_solve_flags( ACC, value, carry );
    ACC += value + carry;

    incDefault();
}

void I51Core::orl_mem_a()
{
    int address = OPERAND1;

    SET_RAM( address, m_dataMem[address] | ACC );

    PC += 2;
    m_mcu->cyclesDone = 1;
}

void I51Core::orl_mem_imm()
{
    int address = OPERAND1;

    SET_RAM( address, m_dataMem[address] | OPERAND2 );

    PC += 3;
    m_mcu->cyclesDone = 1;
}

void I51Core::orl_a_imm()
{
    ACC |= OPERAND1;
    PC += 2;
    m_mcu->cyclesDone = 1;
}

void I51Core::orl_a_mem()
{
    ACC |= GET_RAM( OPERAND1 );
    PC += 2;
    m_mcu->cyclesDone = 1;
}

void I51Core::orl_a_indir_rx()
{
    ACC |= GET_RAM( INDIR_RX_ADDRESS );
    incDefault();
}

void I51Core::jnc_offset()
{
    if( STATUS(Cy) ) PC += 2;
    else             PC +=(signed char)OPERAND1 + 2;
    m_mcu->cyclesDone = 1;
}

void I51Core::jz_offset()
{
    if( !ACC ) PC +=(signed char)OPERAND1 + 2;
    else       PC += 2;
    m_mcu->cyclesDone = 1;
}

void I51Core::xrl_mem_a()
{
    int address = OPERAND1;
    SET_RAM( address, m_dataMem[address] ^ ACC ); //if( address > m_lowDataMemEnd ) aCPU->mSFR[address - 0x80] ^= ACC; //else m_dataMem[address] ^= ACC;
    PC += 2;
    m_mcu->cyclesDone = 1;
}

void I51Core::xrl_mem_imm()
{
    int address = OPERAND1;
    SET_RAM( address, m_dataMem[address] ^ OPERAND2 ); //if( address > m_lowDataMemEnd ) aCPU->mSFR[address - 0x80] ^= OPERAND2; //else m_dataMem[address] ^= OPERAND2;
    PC += 3;
    m_mcu->cyclesDone = 1;
}

void I51Core::xrl_a_imm()
{
    ACC ^= OPERAND1;
    PC += 2;
    m_mcu->cyclesDone = 1;
}

void I51Core::xrl_a_mem()
{
    int value = GET_RAM( OPERAND1 );
    ACC ^= value;
    PC += 2;
    m_mcu->cyclesDone = 1;
}

void I51Core::xrl_a_indir_rx()
{
    ACC ^= GET_RAM( INDIR_RX_ADDRESS );
    incDefault();
}


void I51Core::jnz_offset()
{
    if( ACC ) PC +=(signed char)OPERAND1 + 2;
    else      PC += 2;
    m_mcu->cyclesDone = 1;
}

void I51Core::orl_c_bitaddr()
{
    int address = OPERAND1;
    uint8_t carry = STATUS(Cy) >> Cy; /// PSW[Cy];

    int bit = address & 7;
    int bitmask = 1 << bit;

    address &= 0xf8;
    int value = getValue( address );
    value = (value & bitmask) ? 1 : carry;
    /// PSW[Cy] = value;
    write_S_Bit( Cy, value );

    PC += 2;
    m_mcu->cyclesDone = 1;
}

void I51Core::jmp_indir_a_dptr()
{
    PC =( (m_dataMem[REG_DPH] << 8 ) | m_dataMem[REG_DPL] ) + ACC;
    m_mcu->cyclesDone = 2;
}

void I51Core::mov_a_imm()
{
    ACC = OPERAND1;
    PC += 2;
    m_mcu->cyclesDone = 1;
}

void I51Core::mov_mem_imm()
{
    int address = OPERAND1;
    SET_RAM( address, OPERAND2 ); //if( address > m_lowDataMemEnd ) aCPU->mSFR[address - 0x80] = OPERAND2; //else m_dataMem[address] = OPERAND2;

    PC += 3;
    m_mcu->cyclesDone = 1;
}

void I51Core::mov_indir_rx_imm()
{
    uint16_t address = INDIR_RX_ADDRESS;
    int value = OPERAND1;

    if( address > m_lowDataMemEnd )
    {
        if( m_upperData )
        {
            address += m_regEnd;//m_upperData[address - 0x80] = value;
            if( address <= m_dataMemEnd ) m_dataMem[address] = value;
        }
    }
    else m_dataMem[address] = value;

    PC += 2;
    m_mcu->cyclesDone = 1;
}

void I51Core::sjmp_offset()
{
    PC +=(signed char)(OPERAND1 ) + 2;
    m_mcu->cyclesDone = 2;
}

void I51Core::anl_c_bitaddr()
{
    int address = OPERAND1;
    uint8_t carry = STATUS(Cy) >> Cy; /// PSW[Cy];

    int bit = address & 7;
    int bitmask =( 1 << bit );
    address &= 0xf8;
    int value =( getValue( address ) & bitmask ) ? carry : 0;

    /// PSW[Cy] = value ;
    write_S_Bit( Cy, value );

    PC += 2;
    m_mcu->cyclesDone = 1;
}

void I51Core::movc_a_indir_a_pc()
{
    int address = PC+1 + ACC;
    ACC = m_progMem[address & (m_progSize - 1)];
    incDefault();
}

void I51Core::div_ab()
{
    int a = ACC;
    int b = m_dataMem[REG_B];
    int res;
    //PSW &= ~(PSWMASK_C|PSWMASK_OV );
    /// PSW[Cy] = 0;
    clear_S_Bit( Cy );

    //PSW[OV] = 0;
    clear_S_Bit( OV );

    if( b )
    {
        res = a/b;
        b = a % b;
        a = res;
    }
    else set_S_Bit( OV ); /// PSW[OV] = 1;

    ACC = a;
    SET_RAM( REG_B, b );
    PC++;
    m_mcu->cyclesDone = 3;
}

void I51Core::mov_mem_mem()
{
    int address = OPERAND2;
    int value = getValue( OPERAND1 );

    SET_RAM( address, value );// if( address > m_lowDataMemEnd ) aCPU->mSFR[address - 0x80] = value; else m_dataMem[address] = value;
    PC += 3;
    m_mcu->cyclesDone = 1;
}

void I51Core::mov_mem_indir_rx()
{
    int address1 = OPERAND1;
    int address2 = INDIR_RX_ADDRESS;

    if( address1 > m_lowDataMemEnd )
    {
        if( address2 > m_lowDataMemEnd )
        {
            int value = BAD_VALUE;
            if( m_upperData ) value = m_dataMem[address2+m_regEnd];
            SET_RAM( address1, value ); //aCPU->mSFR[address1 - 0x80] = value;
        }
        else m_dataMem[address1+m_regEnd] = m_dataMem[address2];
    }
    else{
        if( address2 > m_lowDataMemEnd )
        {
            int value = BAD_VALUE;
            if( m_upperData ) value = m_dataMem[address2+m_regEnd];
            m_dataMem[address1] = value;
        }
        else m_dataMem[address1] = m_dataMem[address2];
    }
    PC += 2;
    m_mcu->cyclesDone = 1;
}

void I51Core::mov_dptr_imm()
{
    SET_RAM( REG_DPH, OPERAND1 );
    SET_RAM( REG_DPL, OPERAND2 );
    PC += 3;
    m_mcu->cyclesDone = 1;
}

void I51Core::mov_bitaddr_c()
{
    int address = OPERAND1;
    uint8_t carry = STATUS(Cy) >> Cy; /// PSW[Cy];
    if( address > m_lowDataMemEnd )
    {
        int bit = address & 7;
        int bitmask =( 1 << bit );
        address &= 0xf8;
        SET_RAM( address, (m_dataMem[address] & ~bitmask ) | ( carry << bit ) );
    }
    else{
        int bit = address & 7;
        int bitmask =( 1 << bit );
        address >>= 3;
        address += 0x20;
        m_dataMem[address] =( m_dataMem[address] & ~bitmask ) | ( carry << bit );
    }
    PC += 2;
    m_mcu->cyclesDone = 1;
}

void I51Core::movc_a_indir_a_dptr()
{
    int address = ( m_dataMem[REG_DPH] << 8 ) | (( m_dataMem[REG_DPL] << 0 )+ACC);
    ACC = m_progMem[address & (m_progSize - 1)];
    PC++;
    m_mcu->cyclesDone = 1;
}

void I51Core::subb_a_imm()
{
    uint8_t carry = STATUS(Cy) >> Cy; /// PSW[Cy];
    sub_solve_flags(ACC, OPERAND1 + carry );
    ACC -= OPERAND1 + carry;
    PC += 2;
    m_mcu->cyclesDone = 1;
}

void I51Core::subb_a_mem()
{
    int value = GET_RAM( OPERAND1 );
    if( STATUS(Cy) ) value++;
    sub_solve_flags( ACC, value );
    ACC -= value;

    PC += 2;
    m_mcu->cyclesDone = 1;
}
void I51Core::subb_a_indir_rx()
{
    int value = GET_RAM( INDIR_RX_ADDRESS );
    int address = INDIR_RX_ADDRESS;

    if( address <= m_lowDataMemEnd )
    { if( STATUS(Cy) ) value += 1;}

    sub_solve_flags( ACC, value );
    ACC -= value;

    incDefault();
}

void I51Core::orl_c_compl_bitaddr()
{
    int address = OPERAND1;
    uint8_t carry = STATUS(Cy) >> Cy; /// PSW[Cy];

    int bit = address & 7;
    int bitmask =( 1 << bit );
    address &= 0xf8;
    int value =( getValue( address ) & bitmask ) ? carry : 1;
    /// PSW[Cy] = value ;
    write_S_Bit( Cy, value );

    PC += 2;
    m_mcu->cyclesDone = 1;
}

void I51Core::mov_c_bitaddr()
{
    int address = OPERAND1;

    int bit = address & 7;
    int bitmask =( 1 << bit );
    address &= 0xf8;
    int value = getValue( address );
    value =( value & bitmask ) ? 1 : 0;

    /// PSW[Cy] = value ;
    write_S_Bit( Cy, value );

    PC += 2;
    m_mcu->cyclesDone = 1;
}

void I51Core::inc_dptr()
{
    SET_RAM( REG_DPL, m_dataMem[REG_DPL]+1); //aCPU->mSFR[REG_DPL]++;

    if( !m_dataMem[REG_DPL] ) SET_RAM( REG_DPH, m_dataMem[REG_DPH]+1 );//aCPU->mSFR[REG_DPH]++;
    incDefault();
}

void I51Core::mul_ab()
{
    int a = ACC;
    int b = m_dataMem[REG_B];
    int res = a*b;
    ACC = res & 0xff;
    SET_RAM( REG_B, res >> 8 );

    //PSW &= ~(PSWMASK_C|PSWMASK_OV );

    /// PSW[Cy] = 0;
    clear_S_Bit( Cy );

    /// PSW[OV] = 0;
    clear_S_Bit( OV );

    ////if( aCPU->mSFR[REG_B] ) PSW |= PSWMASK_OV;
    PC++;
    m_mcu->cyclesDone = 3;
}

void I51Core::mov_indir_rx_mem()
{
    int address = INDIR_RX_ADDRESS;
    int value = GET_RAM( OPERAND1 );

    if( address > m_lowDataMemEnd )
    {
        if( m_upperData ) m_dataMem[address+m_regEnd] = value;
    }
    else m_dataMem[address] = value;

    PC += 2;
    m_mcu->cyclesDone = 1;
}

void I51Core::anl_c_compl_bitaddr()
{
    int address = OPERAND1;
    uint8_t carry = STATUS(Cy) >> Cy; /// PSW[Cy];

    int bit = address & 7;
    int bitmask =( 1 << bit );
    address &= 0xf8;
    int value =(  getValue( address ) & bitmask ) ? 0 : carry;
    /// PSW[Cy] = value ;
    write_S_Bit( Cy, value );

    PC += 2;
    m_mcu->cyclesDone = 1;
}

void I51Core::cpl_bitaddr()
{
    int address = m_progMem[(PC+1 ) &( m_progSize - 1 )];

    int bit = address & 7;
    int bitmask =( 1 << bit );
    address &= 0xf8;
    SET_RAM( address, m_dataMem[address] ^ bitmask);

    PC += 2;
    m_mcu->cyclesDone = 1;
}

void I51Core::cpl_c()
{
    /// PSW[Cy] ^= 1;
    *m_STATUS ^= 1 << Cy;
    incDefault();
}

void I51Core::cjne_a_imm_offset()
{
    int value = OPERAND1;

    /// if( ACC < value ) PSW[Cy] = 1;
    /// else              PSW[Cy] = 0;
    write_S_Bit( Cy, ACC < value );

    if( ACC != value ) PC +=(signed char)OPERAND2 + 3;
    else               PC += 3;

    m_mcu->cyclesDone = 2;
}

void I51Core::cjne_a_mem_offset()
{
    int address = OPERAND1;
    int value = getValue( address );

    /// if( ACC < value ) PSW[Cy] = 1;
    /// else              PSW[Cy] = 0;
    write_S_Bit( Cy, ACC < value );

    if( ACC != value ) PC +=(signed char)OPERAND2 + 3;
    else               PC += 3;

    m_mcu->cyclesDone = 2;
}

void I51Core::cjne_indir_rx_imm_offset()
{
    int address = INDIR_RX_ADDRESS;
    int value1 = BAD_VALUE;
    int value2 = OPERAND1;

    if( address > m_lowDataMemEnd )
    {
        if( m_upperData ) value1 = m_dataMem[address+m_regEnd];
    }
    else value1 = m_dataMem[address];

    /// if( value1 < value2 ) PSW[Cy] = 1;
    /// else                  PSW[Cy] = 0;
    write_S_Bit( Cy, value1 < value2 );

    if( value1 != value2 ) PC +=(signed char)OPERAND2 + 3;
    else                   PC += 3;

    m_mcu->cyclesDone = 2;
}

void I51Core::push_mem()
{
    int value = GET_RAM( OPERAND1 );
    pushStack8(value );
    PC += 2;
    m_mcu->cyclesDone = 1;
}

void I51Core::clr_bitaddr()
{
    int address = m_progMem[(PC+1 ) &( m_progSize - 1 )];
    if( address > m_lowDataMemEnd )
    {
        int bit = address & 7;
        int bitmask =( 1 << bit );
        address &= 0xf8;
        SET_RAM( address, m_dataMem[address] & ~bitmask);
    }
    else{
        int bit = address & 7;
        int bitmask =( 1 << bit );
        address >>= 3;
        address += 0x20;
        m_dataMem[address] &= ~bitmask;
    }
    PC += 2;
    m_mcu->cyclesDone = 1;
}

void I51Core::clr_c()
{
    /// PSW[Cy] = 0;
    clear_S_Bit( Cy );
    incDefault();
}

void I51Core::swap_a()
{
    ACC =( ACC << 4 ) |( ACC >> 4 );
    incDefault();
}

/*void I51Core::xch_a_mem()
{
    uint16_t address = OPERAND1;
    int value = GET_RAM( address );

    SET_RAM( address, ACC );

    ACC = value;
    PC += 2;
    m_mcu->cyclesDone = 1;
}

void I51Core::xch_a_indir_rx()
{
    uint16_t address = INDIR_RX_ADDRESS;
    uint8_t val = GET_RAM( address );

    SET_RAM( address, ACC );
    ACC = val;

    PC++;
    m_mcu->cyclesDone = 1;
}*/

void I51Core::xch_a_rx()
{
    int rx = RX_ADDRESS;
    int a = ACC ;

    ACC = m_dataMem[rx];
    m_dataMem[rx] = a;

    incDefault();
}

void I51Core::xch( uint16_t addr )
{
    int value = GET_RAM( addr );

    SET_RAM( addr, ACC );
    ACC = value;

    incDefault();
}

void I51Core::pop_mem()
{
    SET_RAM( OPERAND1, popStack8() );

    PC += 2;
    m_mcu->cyclesDone = 1;
}

void I51Core::setb_bitaddr()
{
    int address = OPERAND1; //m_progMem[(PC+1) /*& ( m_progSize - 1 )*/];

    int bit = address & 7;
    int bitmask =( 1 << bit );
    address &= 0xf8;

    SET_RAM( address, m_dataMem[address] | bitmask);

    PC += 2;
    m_mcu->cyclesDone = 1;
}

void I51Core::setb_c()
{
    /// PSW[Cy] = 1;
    set_S_Bit( Cy );
    incDefault();
}

void I51Core::da_a()
{
    // data sheets for this operation are a bit unclear..
    // - should AC( or C ) ever be cleared?
    // - should this be done in two steps?

    int result = ACC;
    if( (result & 0xf ) > 9 || STATUS(AC) )
        result += 0x6;

    if( (result & 0xff0 ) > 0x90 || STATUS(Cy) )
        result += 0x60;

    if( result > 0x99 ) set_S_Bit(Cy); /// PSW[Cy] = 1;

    ACC = result;

    incDefault();
}

void I51Core::djnz_mem_offset()
{
    int address = OPERAND1;
    int value;

    SET_RAM( address, m_dataMem[address]-1 );
    value = m_dataMem[address];

    if( value ) PC +=(signed char)OPERAND2 + 3;
    else        PC += 3;

    m_mcu->cyclesDone = 2;
}

void I51Core::xchd_a_indir_rx()
{
    int address = INDIR_RX_ADDRESS;

    if( address > m_lowDataMemEnd )
    {
        if( m_upperData ) address += m_regEnd;
    }
    int value = m_dataMem[address];

    m_dataMem[address] =( m_dataMem[address] & 0x0f ) |( ACC & 0x0f );

    ACC =( ACC & 0xf0 ) |( value & 0x0f );
    incDefault();
}


void I51Core::movx_a_indir_dptr()
{
    /////
    /*int dptr =( m_dataMem[REG_DPH] << 8 ) | m_dataMem[REG_DPL];
    if( aCPU->xread )
    {
        ACC = aCPU->xread(dptr );
    }
    else
    {
        if( aCPU->mExtData )
            ACC = aCPU->mExtData[dptr &( aCPU->mExtDataSize - 1 )];
    }*/
    incDefault();
}

void I51Core::movx_a_indir_rx()
{
    /////
    /*int address = INDIR_RX_ADDRESS;
    if( aCPU->xread )
    {
        ACC = aCPU->xread(address );
    }
    else
    {
        if( aCPU->mExtData )
            ACC = aCPU->mExtData[address &( aCPU->mExtDataSize - 1 )];
    }*/

    incDefault();
}

void I51Core::clr_a()
{
    ACC = 0;
    incDefault();
}

void I51Core::mov_a_mem()
{
    // mov a,acc is not a valid instruction
    int address = OPERAND1;
    int value = GET_RAM( address );
    ////if( ( REG_ACC == address ) && ( aCPU->except ) ) aCPU->except(EXCEPTION_ACC_TO_A );
    ACC = value;

    PC += 2;
    m_mcu->cyclesDone = 1;
}

void I51Core::mov_a_indir_rx()
{
    ACC = GET_RAM( INDIR_RX_ADDRESS );
    incDefault();
}

void I51Core::movx_indir_dptr_a()
{
    /////
    /*int dptr =( m_dataMem[REG_DPH] << 8 ) | m_dataMem[REG_DPL];

    if( aCPU->xwrite )
    {
        aCPU->xwrite(dptr, ACC );
    }
    else
    {
        if( aCPU->mExtData )
            aCPU->mExtData[dptr &( aCPU->mExtDataSize - 1 )] = ACC;
    }*/

    incDefault();
}

void I51Core::movx_indir_rx_a()
{
    /////
    /*int address = INDIR_RX_ADDRESS;

    if( aCPU->xwrite )
    {
        aCPU->xwrite(address, ACC );
    }
    else
    {
        if( aCPU->mExtData )
            aCPU->mExtData[address &( aCPU->mExtDataSize - 1 )] = ACC;
    }*/

    incDefault();
}

void I51Core::cpl_a()
{
    ACC = ~ACC;
    incDefault();
}

void I51Core::mov_mem_a()
{
    SET_RAM( OPERAND1, ACC );
    PC += 2;
    m_mcu->cyclesDone = 1;
}

void I51Core::mov_indir_rx_a()
{
    int address = INDIR_RX_ADDRESS;
    if( address > m_lowDataMemEnd )
    {
        if( m_upperData ) m_dataMem[address+m_regEnd] = ACC;
    }
    else m_dataMem[address] = ACC;

    incDefault();
}

void I51Core::nop()
{
    ////if(( m_progMem[PC] != 0 ) && ( aCPU->except )) aCPU->except(EXCEPTION_ILLEGAL_m_opcode );
    incDefault();
}

void I51Core::inc_rx()
{
    int rx = RX_ADDRESS;
    m_dataMem[rx]++;
    incDefault();
}

void I51Core::dec_rx()
{
    int rx = RX_ADDRESS;
    m_dataMem[rx]--;
    incDefault();
}

void I51Core::add_a_rx()
{
    int rx = RX_ADDRESS;
    add_solve_flags( m_dataMem[rx], ACC, 0 );
    ACC += m_dataMem[rx];
    incDefault();
}

void I51Core::addc_a_rx()
{
    int rx = RX_ADDRESS;
    uint8_t carry = STATUS(Cy) >> Cy; /// PSW[Cy];
    add_solve_flags(m_dataMem[rx], ACC, carry );
    ACC += m_dataMem[rx] + carry;
    incDefault();
}

void I51Core::orl_a_rx()
{
    int rx = RX_ADDRESS;
    ACC |= m_dataMem[rx];
    incDefault();
}

void I51Core::xrl_a_rx()
{
    int rx = RX_ADDRESS;
    ACC ^= m_dataMem[rx];
    incDefault();
}


void I51Core::mov_rx_imm()
{
    m_dataMem[RX_ADDRESS] = OPERAND1;
    PC += 2;
    m_mcu->cyclesDone = 1;
}

void I51Core::mov_mem_rx()
{
    SET_RAM( OPERAND1, m_dataMem[RX_ADDRESS] );
    PC += 2;
    m_mcu->cyclesDone = 1;
}

void I51Core::subb_a_rx()
{
    int rx = RX_ADDRESS;
    uint8_t carry = STATUS(Cy) >> Cy; /// PSW[Cy];
    sub_solve_flags( ACC, m_dataMem[rx] + carry );
    ACC -= m_dataMem[rx] + carry;
    incDefault();
}

void I51Core::mov_rx_mem()
{
    int value = GET_RAM( OPERAND1 );
    m_dataMem[RX_ADDRESS] = value;

    PC += 2;
    m_mcu->cyclesDone = 1;
}

void I51Core::cjne_rx_imm_offset()
{
    int rx = RX_ADDRESS;
    int value = OPERAND1;

    /// if( m_dataMem[rx] < value ) PSW[Cy] = 1;
    /// else                        PSW[Cy] = 1;
    write_S_Bit( Cy, m_dataMem[rx] < value );

    PC += 3;
    if( m_dataMem[rx] != value ) PC += (int8_t)OPERAND2;

    m_mcu->cyclesDone = 2;
}

void I51Core::djnz_rx_offset()
{
    int rx = RX_ADDRESS;
    m_dataMem[rx]--;

    if( m_dataMem[rx] ) PC += (int8_t)OPERAND1;
    PC += 2;

    m_mcu->cyclesDone = 2;
}

void I51Core::mov_a_rx()
{
    ACC = m_dataMem[RX_ADDRESS];
    incDefault();
}

void I51Core::mov_rx_a()
{
    m_dataMem[RX_ADDRESS] = ACC;
    incDefault();
}

void I51Core::runDecoder()
{
    m_opcode = m_progMem[PC];

    switch( m_opcode )
    {
        case 0x00: nop(); break;
        case 0x01: ajmp_offset(); break;
        case 0x02: ljmp_address(); break;
        case 0x03: rr_a(); break;
        case 0x04: inc_a(); break;
        case 0x05: inc_mem(); break;
        case 0x06: inc_indir_rx(); break;
        case 0x07: inc_indir_rx(); break;

        case 0x08:
        case 0x09:
        case 0x0a:
        case 0x0b:
        case 0x0c:
        case 0x0d:
        case 0x0e:
        case 0x0f: inc_rx(); break;

        case 0x10: jbc_bitaddr_offset(); break;
        case 0x11: acall_offset(); break;
        case 0x12: lcall_address(); break;
        case 0x13: rrc_a(); break;
        case 0x14: dec_a(); break;
        case 0x15: dec_mem(); break;
        case 0x16: dec_indir_rx(); break;
        case 0x17: dec_indir_rx(); break;

        case 0x18:
        case 0x19:
        case 0x1a:
        case 0x1b:
        case 0x1c:
        case 0x1d:
        case 0x1e:
        case 0x1f: dec_rx(); break;

        case 0x20: jb_bitaddr_offset(); break;
        case 0x21: ajmp_offset(); break;
        case 0x22: ret(); break;
        case 0x23: rl_a(); break;
        case 0x24: add_a_imm(); break;
        case 0x25: add_a_mem(); break;
        case 0x26: add_a_indir_rx(); break;
        case 0x27: add_a_indir_rx(); break;

        case 0x28:
        case 0x29:
        case 0x2a:
        case 0x2b:
        case 0x2c:
        case 0x2d:
        case 0x2e:
        case 0x2f: add_a_rx(); break;

        case 0x30: jnb_bitaddr_offset(); break;
        case 0x31: acall_offset(); break;
        case 0x32: reti(); break;
        case 0x33: rlc_a(); break;
        case 0x34: addc_a_imm(); break;
        case 0x35: addc_a_mem(); break;
        case 0x36: addc_a_indir_rx(); break;
        case 0x37: addc_a_indir_rx(); break;

        case 0x38:
        case 0x39:
        case 0x3a:
        case 0x3b:
        case 0x3c:
        case 0x3d:
        case 0x3e:
        case 0x3f: addc_a_rx(); break;

        case 0x40: jc_offset(); break;
        case 0x41: ajmp_offset(); break;
        case 0x42: orl_mem_a(); break;
        case 0x43: orl_mem_imm(); break;
        case 0x44: orl_a_imm(); break;
        case 0x45: orl_a_mem(); break;
        case 0x46: orl_a_indir_rx(); break;
        case 0x47: orl_a_indir_rx(); break;

        case 0x48:
        case 0x49:
        case 0x4a:
        case 0x4b:
        case 0x4c:
        case 0x4d:
        case 0x4e:
        case 0x4f: orl_a_rx(); break;

        case 0x50: jnc_offset(); break;
        case 0x51: acall_offset(); break;
        case 0x52: anl_mem_a(); break;
        case 0x53: anl_mem_imm(); break;
        case 0x54: anl_a_imm(); break;
        case 0x55: anl_a_mem(); break;
        case 0x56: anl_a_indir_rx(); break;
        case 0x57: anl_a_indir_rx(); break;

        case 0x58:
        case 0x59:
        case 0x5a:
        case 0x5b:
        case 0x5c:
        case 0x5d:
        case 0x5e:
        case 0x5f: anl_a_rx(); break;

        case 0x60: jz_offset(); break;
        case 0x61: ajmp_offset(); break;
        case 0x62: xrl_mem_a(); break;
        case 0x63: xrl_mem_imm(); break;
        case 0x64: xrl_a_imm(); break;
        case 0x65: xrl_a_mem(); break;
        case 0x66: xrl_a_indir_rx(); break;
        case 0x67: xrl_a_indir_rx(); break;

        case 0x68:
        case 0x69:
        case 0x6a:
        case 0x6b:
        case 0x6c:
        case 0x6d:
        case 0x6e:
        case 0x6f: xrl_a_rx(); break;

        case 0x70: jnz_offset(); break;
        case 0x71: acall_offset(); break;
        case 0x72: orl_c_bitaddr(); break;
        case 0x73: jmp_indir_a_dptr(); break;
        case 0x74: mov_a_imm(); break;
        case 0x75: mov_mem_imm(); break;
        case 0x76: mov_indir_rx_imm(); break;
        case 0x77: mov_indir_rx_imm(); break;

        case 0x78:
        case 0x79:
        case 0x7a:
        case 0x7b:
        case 0x7c:
        case 0x7d:
        case 0x7e:
        case 0x7f: mov_rx_imm(); break;

        case 0x80: sjmp_offset(); break;
        case 0x81: ajmp_offset(); break;
        case 0x82: anl_c_bitaddr(); break;
        case 0x83: movc_a_indir_a_pc(); break;
        case 0x84: div_ab(); break;
        case 0x85: mov_mem_mem(); break;
        case 0x86: mov_mem_indir_rx(); break;
        case 0x87: mov_mem_indir_rx(); break;

        case 0x88:
        case 0x89:
        case 0x8a:
        case 0x8b:
        case 0x8c:
        case 0x8d:
        case 0x8e:
        case 0x8f: mov_mem_rx(); break;

        case 0x90: mov_dptr_imm(); break;
        case 0x91: acall_offset(); break;
        case 0x92: mov_bitaddr_c(); break;
        case 0x93: movc_a_indir_a_dptr(); break;
        case 0x94: subb_a_imm(); break;
        case 0x95: subb_a_mem(); break;
        case 0x96: subb_a_indir_rx(); break;
        case 0x97: subb_a_indir_rx(); break;

        case 0x98:
        case 0x99:
        case 0x9a:
        case 0x9b:
        case 0x9c:
        case 0x9d:
        case 0x9e:
        case 0x9f: subb_a_rx(); break;

        case 0xa0: orl_c_compl_bitaddr(); break;
        case 0xa1: ajmp_offset(); break;
        case 0xa2: mov_c_bitaddr(); break;
        case 0xa3: inc_dptr(); break;
        case 0xa4: mul_ab(); break;
        case 0xa5: nop(); break; // unused
        case 0xa6: mov_indir_rx_mem(); break;
        case 0xa7: mov_indir_rx_mem(); break;

        case 0xa8:
        case 0xa9:
        case 0xaa:
        case 0xab:
        case 0xac:
        case 0xad:
        case 0xae:
        case 0xaf: mov_rx_mem(); break;

        case 0xb0: anl_c_compl_bitaddr(); break;
        case 0xb1: acall_offset(); break;
        case 0xb2: cpl_bitaddr(); break;
        case 0xb3: cpl_c(); break;
        case 0xb4: cjne_a_imm_offset(); break;
        case 0xb5: cjne_a_mem_offset(); break;
        case 0xb6: cjne_indir_rx_imm_offset(); break;
        case 0xb7: cjne_indir_rx_imm_offset(); break;

        case 0xb8:
        case 0xb9:
        case 0xba:
        case 0xbb:
        case 0xbc:
        case 0xbd:
        case 0xbe:
        case 0xbf: cjne_rx_imm_offset(); break;

        case 0xc0: push_mem(); break;
        case 0xc1: ajmp_offset(); break;
        case 0xc2: clr_bitaddr(); break;
        case 0xc3: clr_c(); break;
        case 0xc4: swap_a(); break;
        case 0xc5: { xch( OPERAND1 ); PC ++; } break;
        case 0xc6:
        case 0xc7: xch( INDIR_RX_ADDRESS ); break;

        case 0xc8:
        case 0xc9:
        case 0xca:
        case 0xcb:
        case 0xcc:
        case 0xcd:
        case 0xce:
        case 0xcf: xch_a_rx(); break;

        case 0xd0: pop_mem(); break;
        case 0xd1: acall_offset(); break;
        case 0xd2: setb_bitaddr(); break;
        case 0xd3: setb_c(); break;
        case 0xd4: da_a(); break;
        case 0xd5: djnz_mem_offset(); break;
        case 0xd6: xchd_a_indir_rx(); break;
        case 0xd7: xchd_a_indir_rx(); break;

        case 0xd8:
        case 0xd9:
        case 0xda:
        case 0xdb:
        case 0xdc:
        case 0xdd:
        case 0xde:
        case 0xdf: djnz_rx_offset(); break;

        case 0xe0: movx_a_indir_dptr(); break;
        case 0xe1: ajmp_offset(); break;
        case 0xe2: movx_a_indir_rx(); break;
        case 0xe3: movx_a_indir_rx(); break;
        case 0xe4: clr_a(); break;
        case 0xe5: mov_a_mem(); break;
        case 0xe6: mov_a_indir_rx(); break;
        case 0xe7: mov_a_indir_rx(); break;

        case 0xe8:
        case 0xe9:
        case 0xea:
        case 0xeb:
        case 0xec:
        case 0xed:
        case 0xee:
        case 0xef: mov_a_rx(); break;

        case 0xf0: movx_indir_dptr_a(); break;
        case 0xf1: acall_offset(); break;
        case 0xf2: movx_indir_rx_a(); break;
        case 0xf3: movx_indir_rx_a(); break;
        case 0xf4: cpl_a(); break;
        case 0xf5: mov_mem_a(); break;
        case 0xf6: mov_indir_rx_a(); break;
        case 0xf7: mov_indir_rx_a(); break;

        case 0xf8:
        case 0xf9:
        case 0xfa:
        case 0xfb:
        case 0xfc:
        case 0xfd:
        case 0xfe:
        case 0xff: mov_rx_a(); break;
        default: incDefault();
       }
}

