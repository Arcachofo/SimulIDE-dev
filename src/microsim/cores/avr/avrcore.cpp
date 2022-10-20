/***************************************************************************
 *   Copyright (C) 2020 by Santiago González                               *
 *                                                                         *
 ***( see copyright.txt file at root folder )*******************************/
/*
 *   Based on simavr decoder
 *
 *   Copyright 2008, 2010 Michel Pollet <buserror@gmail.com>
 *
 */

#include "avrcore.h"
#include "avr_defines.h"
#include "avrsleep.h"
#include "simulator.h"

AvrCore::AvrCore( eMcu* mcu )
       : McuCpu( mcu )
{
    if( mcu->regExist("EIND") ) EIND = m_mcu->getReg( "EIND" );
    else EIND = NULL;
    if( mcu->regExist("RAMPZ") )
    {
        RAMPZ = m_mcu->getReg( "RAMPZ" );
        m_rampzAddr = m_mcu->getRegAddress( "RAMPZ" );
    }
    else RAMPZ = NULL;
}
AvrCore::~AvrCore() {}

/*void AvrCore::reset()
{
    CpuBase::reset();
}*/

inline void AvrCore::flags_ns( uint8_t res )
{
    bool sn = res & (1<<7);
    write_S_Bit( S_N, sn );
    write_S_Bit( S_S, sn != (bool)STATUS(S_V) );
}
inline void AvrCore::flags_zns( uint8_t res )
{
    write_S_Bit( S_Z, (res == 0) );
    flags_ns( res );
}
inline void AvrCore::flags_Rzns( uint8_t res )
{
    if( res ) clear_S_Bit( S_Z );
    flags_ns( res );
}
inline void AvrCore::flags_sub( uint8_t res, uint8_t rd, uint8_t rr )
{
    /* carry & half carry */
    uint8_t sub_carry = (~rd & rr) | (rr & res) | (res & ~rd);

    write_S_Bit( S_H, sub_carry & (1<<3) );
    write_S_Bit( S_C, sub_carry & (1<<7) );

    /* overflow */
    /// SREG[S_V] = ( ((rd & ~rr & ~res) | (~rd & rr & res) ) >> 7) & 1;
    write_S_Bit( S_V, ((rd & ~rr & ~res) | (~rd & rr & res)) & (1<<7) );
}
inline void AvrCore::flags_sub_Rzns( uint8_t res, uint8_t rd, uint8_t rr )
{
    flags_sub( res, rd, rr );
    flags_Rzns( res );
}
inline void AvrCore::flags_add_zns( uint8_t res, uint8_t rd, uint8_t rr )
{
    /* carry & half carry */
    uint8_t add_carry = (rd & rr) | (rr & ~res) | (~res & rd);

    write_S_Bit( S_H, add_carry & (1<<3) );
    write_S_Bit( S_C, add_carry & (1<<7) );

    /// SREG[S_V] = (((rd & rr & ~res) | (~rd & ~rr & res)) >> 7) & 1; //overflow
    write_S_Bit( S_V, ((rd & rr & ~res) | (~rd & ~rr & res)) & (1<<7) );

    flags_zns( res ); //zns
}
inline void AvrCore::flags_sub_zns( uint8_t res, uint8_t rd, uint8_t rr )
{
    flags_sub( res, rd, rr );
    flags_zns( res ); //zns
}
inline void AvrCore::flags_znv0s( uint8_t res )
{
    clear_S_Bit( S_V );
    flags_zns( res );
}
inline void AvrCore::flags_zcnvs( uint8_t res, uint8_t vr )
{
    write_S_Bit( S_Z, (res == 0) );

    bool sc = (vr & 1)  != 0 ;
    write_S_Bit( S_C, sc );

    bool sn = (res & (1<<7)) != 0;
    write_S_Bit( S_N, sn );

    /// SREG[S_V] = SREG[S_N] ^ SREG[S_C];
    uint8_t sv = sn != sc;
    write_S_Bit( S_V, sv );

    /// SREG[S_S] = SREG[S_N] ^ SREG[S_V];
    write_S_Bit( S_S, sn != sv );
}
inline void AvrCore::flags_zcvs( uint8_t res, uint8_t vr )
{
    write_S_Bit( S_Z, (res == 0) );

    bool sc = (vr & 1)  != 0 ;
    write_S_Bit( S_C, sc );

    bool sn = STATUS( S_N ) != 0;

    /// SREG[S_V] = SREG[S_N] ^ SREG[S_C];
    bool sv = sn != sc;
    write_S_Bit( S_V, sv );

    /// SREG[S_S] = SREG[S_N] ^ SREG[S_V];
    write_S_Bit( S_S, sn != sv );
}
inline void AvrCore::flags_zns16( uint16_t res )
{
    write_S_Bit( S_Z, (res == 0) );

    bool sn = (res & (1<<15)) != 0 ;
    write_S_Bit( S_N, sn );

    bool sv = STATUS(S_V) != 0;
    /// SREG[S_S] = SREG[S_N] ^ SREG[S_V];
    write_S_Bit( S_S, sn != sv );
}
inline int AvrCore::is_instr_32b( uint32_t pc )
{
    uint16_t o = m_progMem[pc] & 0xfc0f;

    return  o == 0x9200 || // STS ! Store Direct to Data Space
            o == 0x9000 || // LDS Load Direct from Data Space
            o == 0x940c || // JMP Long Jump
            o == 0x940d || // JMP Long Jump
            o == 0x940e || // CALL Long Call to sub
            o == 0x940f;   // CALL Long Call to sub
}

void AvrCore::runStep()
{
    m_mcu->cyclesDone = 0;
    uint16_t instruction = m_progMem[m_PC];

    uint32_t new_pc = m_PC + 1;    // future "default" pc
    int cycle = 1;

    switch( instruction & 0xf000 )
    {
        case 0x0000:{
            switch( instruction){
                case 0x0000: {    // NOP
                }    break;
                default:
                {
                    switch( instruction & 0xfc00) {
                        case 0x0400: {    // CPC -- Compare with carry -- 0000 01rd dddd rrrr
                            get_vd5_vr5( instruction );
                            uint8_t res = vd - vr - STATUS( S_C );
                            flags_sub_Rzns( res, vd, vr );
                        }    break;
                        case 0x0c00: {    // ADD -- Add without carry -- 0000 11rd dddd rrrr
                            get_vd5_vr5( instruction );
                            uint8_t res = vd + vr;
                            m_dataMem[d] = res;
                            flags_add_zns( res, vd, vr);
                        }    break;
                        case 0x0800: {    // SBC -- Subtract with carry -- 0000 10rd dddd rrrr
                            get_vd5_vr5( instruction );
                            uint8_t res = vd - vr - STATUS( S_C );
                            m_dataMem[d] = res;
                            flags_sub_Rzns( res, vd, vr);
                        }    break;
                        default: {
                            switch( instruction & 0xff00) {
                                case 0x0100: {    // MOVW -- Copy Register Word -- 0000 0001 dddd rrrr
                                    uint8_t d =( (instruction >> 4) & 0xf) << 1;
                                    uint8_t r =( instruction & 0xf) << 1;
                                    uint16_t vr = m_dataMem[r]|( m_dataMem[r+1] << 8);
                                    SET_REG16_LH( d, vr );
                                }    break;
                                case 0x0200: {    // MULS -- Multiply Signed -- 0000 0010 dddd rrrr
                                    int8_t r = 16 +( instruction & 0xf);
                                    int8_t d = 16 +( (instruction >> 4) & 0xf);
                                    int16_t res =( (int8_t)m_dataMem[r]) *( (int8_t)m_dataMem[d]);
                                    SET_REG16_LH( 0, res);
                                    /// SREG[S_C] =( res >> 15) & 1;
                                    write_S_Bit( S_C, res & 1<<15 );
                                    write_S_Bit( S_Z, res == 0 );
                                    cycle++;
                                }    break;
                                case 0x0300: {    // MUL -- Multiply -- 0000 0011 fddd frrr
                                    int8_t r = 16 +( instruction & 0x7);
                                    int8_t d = 16 +( (instruction >> 4) & 0x7);
                                    int16_t res = 0;
                                    uint8_t c = 0;

                                    switch( instruction & 0x88) {
                                        case 0x00:     // MULSU -- Multiply Signed Unsigned -- 0000 0011 0ddd 0rrr
                                            res =( (uint8_t)m_dataMem[r]) *( (int8_t)m_dataMem[d]);
                                            c =( res >> 15) & 1;
                                            break;
                                        case 0x08:     // FMUL -- Fractional Multiply Unsigned -- 0000 0011 0ddd 1rrr
                                            res =( (uint8_t)m_dataMem[r]) *( (uint8_t)m_dataMem[d]);
                                            c =( res >> 15) & 1;
                                            res <<= 1;
                                            break;
                                        case 0x80:     // FMULS -- Multiply Signed -- 0000 0011 1ddd 0rrr
                                            res =( (int8_t)m_dataMem[r]) *( (int8_t)m_dataMem[d]);
                                            c =( res >> 15) & 1;
                                            res <<= 1;
                                            break;
                                        case 0x88:     // FMULSU -- Multiply Signed Unsigned -- 0000 0011 1ddd 1rrr
                                            res =( (uint8_t)m_dataMem[r]) *( (int8_t)m_dataMem[d]);
                                            c =( res >> 15) & 1;
                                            res <<= 1;
                                            break;
                                    }
                                    cycle++;
                                    SET_REG16_LH( 0, res);
                                    write_S_Bit( S_C, c );
                                    write_S_Bit( S_Z, res == 0 );
                                }    break;
                                default: ;//_avr_invalid_instruction(avr);
                            }
                        }
                    }
                }
            }
        }    break;

        case 0x1000: {
            switch( instruction & 0xfc00) {
                case 0x1800: {    // SUB -- Subtract without carry -- 0001 10rd dddd rrrr
                    get_vd5_vr5( instruction );
                    uint8_t res = vd - vr;
                    m_dataMem[d] = res;
                    flags_sub_zns( res, vd, vr);
                }    break;
                case 0x1000: {    // CPSE -- Compare, skip if equal -- 0001 00rd dddd rrrr
                    get_vd5_vr5( instruction );
                    bool res = vd == vr;
                    if( res )
                    {
                        if( is_instr_32b( new_pc ) ) { new_pc += 2; cycle += 2; }
                        else                         { new_pc += 1; cycle++; }
                    }
                }    break;
                case 0x1400: {    // CP -- Compare -- 0001 01rd dddd rrrr
                    get_vd5_vr5( instruction );
                    uint8_t res = vd - vr;
                    flags_sub_zns( res, vd, vr);
                }    break;
                case 0x1c00: {    // ADC -- Add with carry -- 0001 11rd dddd rrrr
                    get_vd5_vr5( instruction );
                    uint8_t res = vd + vr + STATUS( S_C );
                    m_dataMem[d] = res;
                    flags_add_zns( res, vd, vr );
                }    break;
                default: ;//_avr_invalid_instruction(avr);
            }
        }    break;

        case 0x2000:
        {
            get_vd5_vr5( instruction );
            uint8_t res = vr;
            bool znv = true;
            switch( instruction & 0xfc00)
            {
                case 0x2000: {    // AND -- Logical AND -- 0010 00rd dddd rrrr
                    res &= vd;
                }    break;
                case 0x2400: {    // EOR -- Logical Exclusive OR -- 0010 01rd dddd rrrr
                    res ^= vd;
                }    break;
                case 0x2800: {    // OR -- Logical OR -- 0010 10rd dddd rrrr
                    res |= vd;
                }    break;
                case 0x2c00: {    // MOV -- 0010 11rd dddd rrrr
                    //res = vr;
                    znv = false;
                }    break;
                default: ;//_avr_invalid_instruction(avr);
            }
            if( znv ) flags_znv0s( res);
            m_dataMem[d] = res;
        }    break;

        case 0x3000: {    // CPI -- Compare Immediate -- 0011 kkkk hhhh kkkk
            get_vh4_k8( instruction );
            uint8_t res = vh - k;
            flags_sub_zns( res, vh, k);
        }    break;

        case 0x4000: {    // SBCI -- Subtract Immediate With Carry -- 0100 kkkk hhhh kkkk
            get_vh4_k8( instruction );
            uint8_t res = vh - k - STATUS( S_C );
            m_dataMem[h] = res;
            flags_sub_Rzns( res, vh, k);
        }    break;

        case 0x5000: {    // SUBI -- Subtract Immediate -- 0101 kkkk hhhh kkkk
            get_vh4_k8( instruction );
            uint8_t res = vh - k;
            m_dataMem[h] = res;
            flags_sub_zns( res, vh, k);
        }    break;

        case 0x6000: {    // ORI aka SBR -- Logical OR with Immediate -- 0110 kkkk hhhh kkkk
            get_vh4_k8( instruction );
            uint8_t res = vh | k;
            m_dataMem[h] = res;
            flags_znv0s( res);
        }    break;

        case 0x7000: {    // ANDI    -- Logical AND with Immediate -- 0111 kkkk hhhh kkkk
            get_vh4_k8( instruction );
            uint8_t res = vh & k;
            m_dataMem[h] = res;
            flags_znv0s( res );
        }    break;

        case 0xa000:
        case 0x8000: {
            /*
             * Load( LDD/STD) store instructions
             *
             * 10q0 qqsd dddd yqqq
             * s = 0 = load, 1 = store
             * y = 16 bits register index, 1 = Y, 0 = X
             * q = 6 bit displacement
             */
            uint16_t v = 0;
            switch( instruction & 0xd008 )
            {
                case 0xa000:
                case 0x8000: {    // LD( LDD) -- Load Indirect using Z -- 10q0 qqsd dddd yqqq
                    v = m_dataMem[R_ZL] | ( m_dataMem[R_ZH] << 8);
                }    break;
                case 0xa008:
                case 0x8008: {    // LD( LDD) -- Load Indirect using Y -- 10q0 qqsd dddd yqqq
                    v = m_dataMem[R_YL] | ( m_dataMem[R_YH] << 8);
                }    break;
                //default: ;//_avr_invalid_instruction(avr);
            }
            get_d5_q6( instruction );
            if( instruction & 0x0200) SET_RAM( v+q, m_dataMem[d] );
            else                      SET_RAM( d, GET_RAM(v+q) );
            cycle += 1; // 2 cycles, 3 for tinyavr
        }    break;

        case 0x9000: {
            // SREG set/clear instructions
            // SEH,SEI,SEN,SES,SET,SEV,SEZ; CLH,CLI,CLN,CLS,CLT,CLV,CLZ
            if( (instruction & 0xff0f) == 0x9408 )
            {
                uint8_t bit = (instruction >> 4) & 7;
                bool set = (instruction & 0x0080) == 0;
                write_S_Bit( bit, set );
                if( bit == S_I ) m_mcu->enableInterrupts( set );
            }
            switch( instruction )
            {
                case 0x9588: { // SLEEP -- 1001 0101 1000 1000
                    /* Don't sleep if there are interrupts about to be serviced.
                     * Without this check, it was possible to incorrectly enter a state
                     * in which the cpu was sleeping and interrupts were disabled. For more
                     * details, see the commit message. */
                    qDebug() <<"Warning: AVR SLEEP instruction not Fully implemented";
////////             if( !int_pending.empty() || !SREG[S_I]) state = cpu_Sleeping;

                    m_mcu->sleep( true );
                }    break;
                case 0x9598: { // BREAK -- 1001 0101 1001 1000
                    qDebug() <<"ERROR: AVR BREAK instruction not implemented";
                }    break;
                case 0x95a8: { // WDR -- Watchdog Reset -- 1001 0101 1010 1000
                    m_mcu->wdr();
                }    break;
                case 0x95e8: { // SPM -- Store Program Memory -- 1001 0101 1110 1000
                    qDebug() <<"ERROR: AVR SPM instruction not implemented"; ////avr_ioctl(avr, AVR_IOCTL_FLASH_SPM, 0);
                }    break;
                case 0x9409:   // IJMP   -- Indirect jump -- 1001 0100 0000 1001
                case 0x9419:   // EIJMP  -- Indirect jump -- 1001 0100 0001 1001   bit 4 is "Extended"
                case 0x9509:   // ICALL  -- Indirect Call to Subroutine -- 1001 0101 0000 1001
                case 0x9519: { // EICALL -- Indirect Call to Subroutine -- 1001 0101 0001 1001   bit 8 is "Call: push pc"
                    int exte = instruction & 0x10;  // Extended
                    int call = instruction & 0x100; // Call: push pc
                    uint32_t z = m_dataMem[R_ZL] | (m_dataMem[R_ZH] << 8);
                    if( exte ){
                        if( !EIND ){
                            qDebug() << "ERROR: AVR Invalid instruction: EICALL with no EIND";
                            break;
                        }
                        z |= *EIND << 16;
                    }
                    if( call ){
                        PUSH_STACK( new_pc );
                        m_RET_ADDR = new_pc;
                        cycle += m_progAddrSize-1;
                    }
                    new_pc = z;
                    cycle++;
                }    break;
                case 0x9518:     // RETI -- Return from Interrupt -- 1001 0101 0001 1000
                    m_mcu->m_interrupts.retI();// SREG flag managed in AvrInterrupt
                case 0x9508: {   // RET -- Return -- 1001 0101 0000 1000
                    new_pc = POP_STACK();
                    cycle += 1 + m_progAddrSize;
                }    break;
                case 0x95c8: {    // LPM -- Load Program Memory R0 <-( Z) -- 1001 0101 1100 1000
                    uint16_t z = m_dataMem[R_ZL] |( m_dataMem[R_ZH] << 8);
                    cycle += 2; // 3 cycles
                    uint16_t prgData = m_progMem[z/2];
                    if( z&1 ) prgData >>= 8;
                    m_dataMem[0] = prgData & 0xFF;
                }    break;
                case 0x95d8: {    // ELPM -- Load Program Memory R0 <-( Z) -- 1001 0101 1101 1000
                    if( !RAMPZ){
                        qDebug() << "ERROR: AVR Invalid instruction: ELPM with no RAMPZ";
                        break;
                    }
                    uint32_t z = m_dataMem[R_ZL] |( m_dataMem[R_ZH] << 8) | (*RAMPZ << 16);
                    uint16_t prgData = m_progMem[z/2];
                    if( z&1 ) prgData >>= 8;
                    m_dataMem[0] = prgData & 0xFF;
                    cycle += 2; // 3 cycles
                }    break;
                default:  {
                    switch( instruction & 0xfe0f) {
                        case 0x9000: {    // LDS -- Load Direct from Data Space, 32 bits -- 1001 0000 0000 0000
                            get_d5( instruction );
                            uint16_t x = m_progMem[new_pc];
                            new_pc += 1;
                            m_dataMem[d] = GET_RAM(x);
                            cycle++; // 2 cycles
                        }    break;
                        case 0x9005:
                        case 0x9004: {    // LPM -- Load Program Memory -- 1001 000d dddd 01oo
                            get_d5( instruction );
                            uint16_t z = m_dataMem[R_ZL] | (m_dataMem[R_ZH] << 8);
                            int op = instruction & 1;
                            uint16_t prgData = m_progMem[z/2];
                            if( z&1 ) prgData >>= 8;
                            m_dataMem[d] = prgData & 0xFF;
                            if( op) SET_REG16_HL( R_ZL, ++z );
                            cycle += 2; // 3 cycles
                        }    break;
                        case 0x9006:
                        case 0x9007: {    // ELPM -- Extended Load Program Memory -- 1001 000d dddd 01oo
                            if( !RAMPZ){
                                qDebug() << "ERROR: AVR Invalid instruction: ELPM with no RAMPZ";
                                break;
                            }
                            uint16_t z = m_dataMem[R_ZL] |( m_dataMem[R_ZH] << 8) | (*RAMPZ << 16);
                            get_d5( instruction );
                            int op = instruction & 1;
                            uint16_t prgData = m_progMem[z/2];
                            if( z&1 ) prgData >>= 8;
                            m_dataMem[d] = prgData & 0xFF;
                            if( op) {
                                z++;
                                m_dataMem[m_rampzAddr] = z >> 16;
                                SET_REG16_HL( R_ZL, z );
                            }
                            cycle += 2; // 3 cycles
                        }    break;
                        /*
                         * Load store instructions
                         *
                         * 1001 00sr rrrr iioo
                         * s = 0 = load, 1 = store
                         * ii = 16 bits register index, 11 = X, 10 = Y, 00 = Z
                         * oo = 1) post increment, 2) pre-decrement
                         */
                        case 0x900c:
                        case 0x900d:
                        case 0x900e: {    // LD -- Load Indirect from Data using X -- 1001 000d dddd 11oo
                            int op = instruction & 3;
                            get_d5( instruction );
                            uint16_t x = (m_dataMem[R_XH] << 8) | m_dataMem[R_XL];
                            cycle++; // 2 cycles( 1 for tinyavr, except with inc/dec 2)
                            if( op == 2) x--;
                            uint8_t vd = GET_RAM(x);
                            if( op == 1) x++;
                            SET_REG16_HL( R_XL, x);
                            m_dataMem[d] = vd;
                        }    break;
                        case 0x920c:
                        case 0x920d:
                        case 0x920e: {    // ST -- Store Indirect Data Space X -- 1001 001d dddd 11oo
                            int op = instruction & 3;
                            get_vd5( instruction );
                            uint16_t x =( m_dataMem[R_XH] << 8) | m_dataMem[R_XL];
                            cycle++; // 2 cycles, except tinyavr
                            if( op == 2) x--;
                            SET_RAM( x, vd );
                            if( op == 1) x++;
                            SET_REG16_HL( R_XL, x);
                        }    break;
                        case 0x9009:
                        case 0x900a: {    // LD -- Load Indirect from Data using Y -- 1001 000d dddd 10oo
                            int op = instruction & 3;
                            get_d5( instruction );
                            uint16_t y =( m_dataMem[R_YH] << 8) | m_dataMem[R_YL];
                            cycle++; // 2 cycles, except tinyavr
                            if( op == 2) y--;
                            uint8_t vd = GET_RAM(y);
                            if( op == 1) y++;
                            SET_REG16_HL( R_YL, y);
                            m_dataMem[d] = vd;
                        }    break;
                        case 0x9209:
                        case 0x920a: {    // ST -- Store Indirect Data Space Y -- 1001 001d dddd 10oo
                            int op = instruction & 3;
                            get_vd5( instruction );
                            uint16_t y =( m_dataMem[R_YH] << 8) | m_dataMem[R_YL];
                             cycle++;
                            if( op == 2) y--;
                            SET_RAM( y, vd );
                            if( op == 1) y++;
                            SET_REG16_HL( R_YL, y);
                        }    break;
                        case 0x9200: {    // STS -- Store Direct to Data Space, 32 bits -- 1001 0010 0000 0000
                            get_vd5( instruction );
                            uint16_t x = m_progMem[new_pc];
                            new_pc += 1;
                            cycle++;
                            SET_RAM( x, vd );
                        }    break;
                        case 0x9001:
                        case 0x9002: {    // LD -- Load Indirect from Data using Z -- 1001 000d dddd 00oo
                            int op = instruction & 3;
                            get_d5( instruction );
                            uint16_t z =( m_dataMem[R_ZH] << 8) | m_dataMem[R_ZL];
                            cycle++;; // 2 cycles, except tinyavr
                            if( op == 2) z--;
                            uint8_t vd = GET_RAM(z);
                            if( op == 1) z++;
                            SET_REG16_HL( R_ZL, z);
                            m_dataMem[d] = vd;
                        }    break;
                        case 0x9201:
                        case 0x9202: {    // ST -- Store Indirect Data Space Z -- 1001 001d dddd 00oo
                            int op = instruction & 3;
                            get_vd5( instruction );
                            uint16_t z =( m_dataMem[R_ZH] << 8) | m_dataMem[R_ZL];
                             cycle++; // 2 cycles, except tinyavr
                            if( op == 2) z--;
                            SET_RAM( z, vd );
                            if( op == 1 ) z++;
                            SET_REG16_HL( R_ZL, z);
                        }    break;
                        case 0x900f: {    // POP -- 1001 000d dddd 1111
                            get_d5( instruction );
                            m_dataMem[d] = POP_STACK8();
                            cycle++;
                        }    break;
                        case 0x920f: {    // PUSH -- 1001 001d dddd 1111
                            get_vd5( instruction );
                            PUSH_STACK8(vd);
                            cycle++;
                        }    break;
                        case 0x9400: {    // COM -- One's Complement -- 1001 010d dddd 0000
                            get_vd5( instruction );
                            uint8_t res = 0xff - vd;
                            m_dataMem[d] = res;
                            flags_znv0s( res );
                            set_S_Bit( S_C );
                        }    break;
                        case 0x9401: {    // NEG -- Two's Complement -- 1001 010d dddd 0001
                            get_vd5( instruction );
                            uint8_t res = 0x00 - vd;
                            m_dataMem[d] = res;
                            write_S_Bit( S_H, ((res >> 3)|( vd >> 3)) & 1 );
                            write_S_Bit( S_V, res == 0x80 );
                            write_S_Bit( S_C, res != 0 );
                            flags_zns( res );
                        }    break;
                        case 0x9402: {    // SWAP -- Swap Nibbles -- 1001 010d dddd 0010
                            get_vd5( instruction );
                            uint8_t res =( vd >> 4) | ( vd << 4) ;
                            m_dataMem[d] = res;
                        }    break;
                        case 0x9403: {    // INC -- Increment -- 1001 010d dddd 0011
                            get_vd5( instruction );
                            uint8_t res = vd + 1;
                            m_dataMem[d] = res;
                            write_S_Bit( S_V, res == 0x80 );
                            flags_zns( res);
                        }    break;
                        case 0x9405: {    // ASR -- Arithmetic Shift Right -- 1001 010d dddd 0101
                            get_vd5( instruction );
                            uint8_t res = (vd >> 1) |(vd & 0x80);
                            m_dataMem[d] = res;
                            flags_zcnvs( res, vd );
                        }    break;
                        case 0x9406: {    // LSR -- Logical Shift Right -- 1001 010d dddd 0110
                            get_vd5( instruction );
                            uint8_t res = vd >> 1;
                            m_dataMem[d] = res;
                            clear_S_Bit( S_N );
                            flags_zcvs( res, vd);
                        }    break;
                        case 0x9407: {    // ROR -- Rotate Right -- 1001 010d dddd 0111
                            get_vd5( instruction );
                            uint8_t res =( STATUS(S_C) ? 0x80 : 0) | vd >> 1;
                            m_dataMem[d] = res;
                            flags_zcnvs( res, vd);
                        }    break;
                        case 0x940a: {    // DEC -- Decrement -- 1001 010d dddd 1010
                            get_vd5( instruction );
                            uint8_t res = vd - 1;
                            m_dataMem[d] = res;
                            write_S_Bit( S_V, res == 0x7f );
                            flags_zns( res );
                        }    break;
                        case 0x940c:
                        case 0x940d: {    // JMP -- Long Call to sub, 32 bits -- 1001 010a aaaa 110a
                            uint32_t a =( (instruction & 0x01f0) >> 3) |( instruction & 1);
                            uint16_t x = m_progMem[new_pc];
                            a =( a << 16) | x;
                            new_pc = a;
                            cycle += 2;
                        }    break;
                        case 0x940e:
                        case 0x940f: {    // CALL -- Long Call to sub, 32 bits -- 1001 010a aaaa 111a
                            uint32_t a =( (instruction & 0x01f0) >> 3) |( instruction & 1);
                            uint16_t x = m_progMem[new_pc];
                            a =( a << 16) | x;
                            new_pc += 1;
                            PUSH_STACK( new_pc );
                            m_RET_ADDR = new_pc;
                            cycle += 1+m_progAddrSize;
                            new_pc = a;
                        }    break;

                        default: {
                            switch( instruction & 0xff00) {
                                case 0x9600: {    // ADIW -- Add Immediate to Word -- 1001 0110 KKpp KKKK
                                    get_vp2_k6( instruction );
                                    uint16_t res = vp + k;
                                    SET_REG16_HL( p, res );
                                    /// SREG[S_V] =( (~vp & res) >> 15) & 1;
                                    write_S_Bit( S_V, (~vp & res) & (1<<15) );

                                    ///SREG[S_C] =( (~res & vp) >> 15) & 1;
                                    write_S_Bit( S_C, (~res & vp) & (1<<15) );

                                    flags_zns16( res );
                                    cycle++;
                                }    break;
                                case 0x9700: {    // SBIW -- Subtract Immediate from Word -- 1001 0111 KKpp KKKK
                                    get_vp2_k6( instruction );
                                    uint16_t res = vp - k;
                                    SET_REG16_HL( p, res );
                                    /// SREG[S_V] =( (vp & ~res) >> 15) & 1;
                                    write_S_Bit( S_V, (vp & ~res) & (1<<15) );

                                    ///SREG[S_C] =( (res & ~vp) >> 15) & 1;
                                    write_S_Bit( S_C, (res & ~vp) & (1<<15) );

                                    flags_zns16( res );
                                    cycle++;
                                }    break;
                                case 0x9800: {    // CBI -- Clear Bit in I/O Register -- 1001 1000 AAAA Abbb
                                    get_io5_b3mask( instruction );
                                    uint8_t res = GET_RAM( io ) & ~mask;
                                    SET_RAM( io, res );
                                    cycle++;
                                }    break;
                                case 0x9900: {    // SBIC -- Skip if Bit in I/O Register is Cleared -- 1001 1001 AAAA Abbb
                                    get_io5_b3mask( instruction );
                                    uint8_t res = GET_RAM( io ) & mask;
                                    if( !res)
                                    {
                                        if( is_instr_32b(new_pc) ) { new_pc += 2; cycle += 2; }
                                        else                       { new_pc += 1; cycle++; }
                                    }
                                }    break;
                                case 0x9a00: {    // SBI -- Set Bit in I/O Register -- 1001 1010 AAAA Abbb
                                    get_io5_b3mask( instruction );
                                    uint8_t res = GET_RAM( io ) | mask;
                                    SET_RAM( io, res );
                                    cycle++;
                                }    break;
                                case 0x9b00: {    // SBIS -- Skip if Bit in I/O Register is Set -- 1001 1011 AAAA Abbb
                                    get_io5_b3mask( instruction );
                                    uint8_t res = GET_RAM( io ) & mask;
                                    if( res )
                                    {
                                        if( is_instr_32b(new_pc) ) { new_pc += 2; cycle += 2; }
                                        else                       { new_pc += 1; cycle++; }
                                    }
                                }    break;
                                default:
                                    switch( instruction & 0xfc00) {
                                        case 0x9c00: {    // MUL -- Multiply Unsigned -- 1001 11rd dddd rrrr
                                            get_vd5_vr5( instruction );
                                            uint16_t res = vd * vr;
                                            cycle++;
                                            SET_REG16_LH( 0, res );
                                            write_S_Bit( S_Z, res == 0 );

                                            /// SREG[S_C] =( res >> 15) & 1;
                                            write_S_Bit( S_C, res & (1<<15) );
                                        }    break;
                                        default: ;//_avr_invalid_instruction(avr);
                                    }
                            }
                        }    break;
                    }
                }    break;
            }
        }    break;

        case 0xb000: {
            switch( instruction & 0xf800) {
                case 0xb800: {    // OUT A,Rr -- 1011 1AAd dddd AAAA
                    get_d5_a6( instruction );
                    SET_RAM( A, m_dataMem[d] );
                }    break;
                case 0xb000: {    // IN Rd,A -- 1011 0AAd dddd AAAA
                    get_d5_a6( instruction );
                    m_dataMem[d] = GET_RAM(A);
                }    break;
                default: ;//_avr_invalid_instruction(avr);
            }
        }    break;

        case 0xc000: {    // RJMP -- 1100 kkkk kkkk kkkk
            const int16_t k = ((int16_t)((instruction << 4) & 0xFFFF)) >> 4;
            new_pc = (new_pc + k) % m_progSize;
            cycle++;
        }    break;

        case 0xd000: {    // RCALL -- 1101 kkkk kkkk kkkk
            const int16_t k = ((int16_t)((instruction << 4) & 0xFFFF)) >> 4;
            cycle += m_progAddrSize;
            PUSH_STACK( new_pc );
            m_RET_ADDR = new_pc;
            new_pc = (new_pc + k) % m_progSize;
        }    break;

        case 0xe000: {    // LDI Rd, K aka SER( LDI r, 0xff) -- 1110 kkkk dddd kkkk
            get_h4_k8( instruction );
            m_dataMem[h] = k;
        }    break;

        case 0xf000: {
            switch( instruction & 0xfe00)
            {
                case 0xf000:
                case 0xf200:
                case 0xf400:
                case 0xf600: {    // BRXC/BRXS -- All the SREG branches -- 1111 0Boo oooo osss
                    int16_t o =( (int16_t)(instruction << 6)) >> 9; // offset
                    uint8_t s = instruction & 7;
                    bool set =( instruction & 0x0400) == 0;        // this bit means BRXC otherwise BRXS
                    int branch =( STATUS(s) && set) ||( !STATUS(s) && !set);
                    if( branch) {
                        cycle++; // 2 cycles if taken, 1 otherwise
                        new_pc = new_pc + o;
                    }
                }    break;
                case 0xf800:
                case 0xf900: {    // BLD -- Bit Store from T into a Bit in Register -- 1111 100d dddd 0bbb
                    get_vd5_s3_mask( instruction );
                    uint8_t v =( vd & ~mask) |( STATUS(S_T) ? mask : 0);
                    m_dataMem[d] = v;
                }    break;
                case 0xfa00:
                case 0xfb00:{    // BST -- Bit Store into T from bit in Register -- 1111 101d dddd 0bbb
                    get_vd5_s3( instruction )
                    write_S_Bit( S_T, ( vd >> s) & 1 );
                }    break;
                case 0xfc00:
                case 0xfe00: {    // SBRS/SBRC -- Skip if Bit in Register is Set/Clear -- 1111 11sd dddd 0bbb
                    get_vd5_s3_mask( instruction )
                    int set =( instruction & 0x0200) != 0;
                    int branch =( (vd & mask) && set) ||( !(vd & mask) && !set);
                    if( branch)
                    {
                        if( is_instr_32b(new_pc) ) { new_pc += 2; cycle += 2;}
                        else                       { new_pc += 1; cycle++; }
                    }   break;
                }
                default: ;//_avr_invalid_instruction(avr);
            }
        }    break;
        default: ;//_avr_invalid_instruction(avr);
    }
    if( new_pc >= m_progSize ) new_pc = 0;

    m_PC = new_pc;
    m_mcu->cyclesDone = cycle;
}
