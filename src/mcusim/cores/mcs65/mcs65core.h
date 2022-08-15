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
            a_NON=0,
            a_ACC,
            a_ABS,
            a_IMM,
            //a_IMPL,
            a_IND,
            a_REL
        };

        enum addrIndx_t{
            i_X=1<<0,
            i_Y=1<<1,
            i_C=1<<2,
            i_I=1<<3,
            i_Z=1<<4,
        };

        virtual void reset() override;
        virtual void runDecoder() override;
        virtual void runClock( bool clkState )override { if( !clkState ) runDecoder(); }

    private:
        inline uint8_t readDataBus();

        uint64_t m_psStep;
        int m_cycle;

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

        uint8_t m_P;   // status
        uint8_t m_SP;
        uint8_t m_Ac;
        uint8_t m_IR;
        uint8_t m_rX;
        uint8_t m_rY;
        //uint8_t* m_rI;

        cpuState_t m_cpuState;
        uint8_t m_Ocode;
        uint8_t m_group;
        uint8_t m_Amode;

        uint8_t m_IsrH;
        uint8_t m_IsrL;

        addrMode_t m_addrMode;
        uint8_t m_addrIndx;
        uint8_t m_op0;
        uint8_t m_op1;
        uint16_t m_opAddr;

        uint64_t m_tP0;
        uint64_t m_tSync;
        //uint64_t m_tMds;
        //uint64_t m_tAds;
        uint64_t m_tAcc;

        //bool m_zeroPage;
        bool m_ctrlPC;
        bool m_incPC;

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
