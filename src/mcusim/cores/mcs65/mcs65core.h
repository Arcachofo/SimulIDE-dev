/***************************************************************************
 *   Copyright (C) 2022 by santiago González                               *
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

#ifndef MCS65CORE_H
#define MCS65CORE_H

#include "mcucore.h"
#include "mcupin.h"

//#define NEGATIVE  0x80
//#define OVERFLOW  0x40
#define CONSTANT  0x20
#define BREAK     0x10
//#define DECIMAL   0x08
//#define INTERRUPT 0x04
//#define ZERO      0x02
//#define CARRY     0x01

#define ACC       m_acc

#define SET_NEGATIVE(x)  write_S_Bit( N, x & 0x80 ) //(x ? (m_STATUS |= NEGATIVE) : (m_STATUS &= (~NEGATIVE)) )
#define SET_OVERFLOW(x)  write_S_Bit( V, x ) //(x ? (m_STATUS |= OVERFLOW) : (m_STATUS &= (~OVERFLOW)) )
//#define SET_CONSTANT(x) (x ? (m_STATUS |= CONSTANT) : (m_STATUS &= (~CONSTANT)) )
//#define SET_BREAK(x) (x ? (m_STATUS |= BREAK) : (m_STATUS &= (~BREAK)) )
#define SET_DECIMAL(x)   write_S_Bit( D, x ) //(x ? (m_STATUS |= DECIMAL) : (m_STATUS &= (~DECIMAL)) )
#define SET_INTERRUPT(x) write_S_Bit( I, x ) //(x ? (m_STATUS |= INTERRUPT) : (m_STATUS &= (~INTERRUPT)) )
#define SET_ZERO(x)      write_S_Bit( Z, x ) //(x ? (m_STATUS |= ZERO) : (m_STATUS &= (~ZERO)) )
#define SET_CARRY(x)     write_S_Bit( C, x ) //(x ? (m_STATUS |= CARRY) : (m_STATUS &= (~CARRY)) )


class MAINMODULE_EXPORT Mcs65Core : public McuCore
{
    public:
        Mcs65Core( eMcu* mcu );
        ~Mcs65Core();

        enum { C=0,Z,I,D,B,O,V,N };

        enum cpuState_t{
            cpu_FETCH=0,
            cpu_DECODE,
            cpu_READ,
            cpu_EXEC,
            cpu_WRITE
        };

        /*enum readState_t{
            read_IDLE=0,
            read_READY,
            read_STEP,
        };*/

        enum addrMode_t{
            addr_NONE=0,
            addr_ABSO,
            addr_ACCU,
            addr_IMME,
            //addr_IMPL,
            addr_INDI,
            addr_RELA
        };

        virtual void reset() override;
        virtual void runDecoder() override;

    private:

        uint64_t m_psStep;

        // Control Pins
        IoPin* m_phi0Pin;
        IoPin* m_phi1Pin;
        IoPin* m_phi2Pin;
        IoPin* m_syncPin;
        // Interrupt Pins
        IoPin* m_irqPin;
        IoPin* m_nmiPin;
        // User Pins
        IoPin* m_rdyPin;
        IoPin* m_soPin;
        IoPin* m_dbePin;

        uint8_t m_SP;
        uint8_t m_acc;

        uint8_t* m_regX;
        uint8_t* m_regY;
        uint8_t* m_regI;

        cpuState_t m_cpuState;
        uint8_t m_instr;
        uint8_t m_Ocode;
        uint8_t m_group;
        uint8_t m_Amode;

        uint8_t m_IsrH;
        uint8_t m_IsrL;

        int m_step;

        addrMode_t m_addrMode;
        uint8_t m_op0;
        uint8_t m_op1;
        uint16_t m_readAddr;
        //readState_t m_readState;

        bool m_zeroPage;

        inline void setNZ( uint8_t val );

        void Read( uint16_t addr );
        void Write( uint16_t addr, uint8_t val );

        void pushStack8( uint8_t byte );
        void popStack8();

        inline void Op_ADC();
        inline void Op_AND();
        inline void Op_ASL();
        //inline void Op_ASL_ACC();
        inline void Op_BCC();
        inline void Op_BCS();
        inline void Op_BEQ();
        inline void Op_BIT();
        inline void Op_BMI();
        inline void Op_BNE();
        inline void Op_BPL();
        inline void Op_BRK();
        inline void Op_BVC();
        inline void Op_BVS();
        inline void Op_CLC();
        inline void Op_CLD();
        inline void Op_CLI();
        inline void Op_CLV();
        inline void Op_CMP();
        inline void Op_CPX();
        inline void Op_CPY();
        inline void Op_DEC();
        inline void Op_DEX();
        inline void Op_DEY();
        inline void Op_EOR();
        inline void Op_INC();
        inline void Op_INX();
        inline void Op_INY();
        inline void Op_JMP();
        inline void Op_JSR();
        inline void Op_LDA();
        inline void Op_LDX();
        inline void Op_LDY();
        inline void Op_LSR();
        //inline void Op_LSR_ACC();
        inline void Op_NOP();
        inline void Op_ORA();
        inline void Op_PHA();
        inline void Op_PHP();
        inline void Op_PLA();
        inline void Op_PLP();
        inline void Op_ROL();
        //inline void Op_ROL_ACC();
        inline void Op_ROR();
        //inline void Op_ROR_ACC();
        inline void Op_RTI();
        inline void Op_RTS();
        inline void Op_SBC();
        inline void Op_SEC();
        inline void Op_SED();
        inline void Op_SEI();
        inline void Op_STA();
        inline void Op_STX();
        inline void Op_STY();
        inline void Op_TAX();
        inline void Op_TAY();
        inline void Op_TSX();
        inline void Op_TXA();
        inline void Op_TXS();
        inline void Op_TYA();

        inline void BXX( uint8_t flag, uint8_t y );
        inline void BRK();
        inline void PHP();
        inline void BPL();
        inline void CLC();
        inline void JSR();
        inline void PLP();
        inline void SEC();
        inline void RTI();
        inline void PHA();
        inline void CLI();
        inline void RTS();
        inline void PLA();
        inline void SEI();
        inline void DEY();
        inline void TXA();
        inline void TYA();
        inline void TXS();
        inline void TAY();
        inline void TAX();
        inline void CLV();
        inline void TSX();
        inline void INY();
        inline void DEX();
        inline void CLD();
        inline void INX();
        inline void NOP();
        inline void SED();
};
#endif
