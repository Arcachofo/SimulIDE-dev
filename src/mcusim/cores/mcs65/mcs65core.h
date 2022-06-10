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

#define CONSTANT  0x20
#define BREAK     0x10

#define ACC       m_acc

#define SET_NEGATIVE(x)  write_S_Bit( N, x & 0x80 ) //(x ? (m_STATUS |= NEGATIVE) : (m_STATUS &= (~NEGATIVE)) )
#define SET_OVERFLOW(x)  write_S_Bit( V, x ) //(x ? (m_STATUS |= OVERFLOW) : (m_STATUS &= (~OVERFLOW)) )
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
            cpu_RESET=0,
            cpu_FETCH,
            cpu_DECODE,
            cpu_READ,
            cpu_EXEC,
            cpu_WRITE
        };

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
        McuPin* m_phi0Pin;
        McuPin* m_phi1Pin;
        McuPin* m_phi2Pin;
        McuPin* m_syncPin;
        // Interrupt Pins
        IoPin* m_irqPin;
        IoPin* m_nmiPin;
        // User Pins
        McuPin* m_rdyPin;
        McuPin* m_soPin;
        McuPin* m_dbePin;

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
        uint16_t m_opAddr;

        bool m_zeroPage;

        inline void setNZ( uint8_t val );

        void Read( uint16_t addr );
        void readMem( uint16_t addr );
        void Write( uint16_t addr, uint8_t val );

        void pushStack8( uint8_t byte );
        void popStack8();

        inline void ADC();
        inline void AND();
        inline void ASL();
        inline void BIT();
        inline void BRK();
        inline void CLC();
        inline void CLD();
        inline void CLI();
        inline void CLV();
        inline void CMP();
        inline void CPX();
        inline void CPY();
        inline void DEC();
        inline void DEX();
        inline void DEY();
        inline void EOR();
        inline void INC();
        inline void INX();
        inline void INY();
        inline void JMP();
        inline void JSR();
        inline void LDA();
        inline void LDX();
        inline void LDY();
        inline void LSR();
        inline void NOP();
        inline void ORA();
        inline void PHA();
        inline void PHP();
        inline void PLA();
        inline void PLP();
        inline void ROL();
        inline void ROR();
        inline void RTI();
        inline void RTS();
        inline void SBC();
        inline void SEC();
        inline void SED();
        inline void SEI();
        inline void STA();
        inline void STX();
        inline void STY();
        inline void TAX();
        inline void TAY();
        inline void TSX();
        inline void TXA();
        inline void TXS();
        inline void TYA();
        inline void BXX( uint8_t flag, uint8_t y );
};
#endif
