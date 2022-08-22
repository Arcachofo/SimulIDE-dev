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

#ifndef CPU_H
#define CPU_H

#include "cpubase.h"
#include "cpuextmem.h"

class Cpu;
typedef void (Cpu::* funcPt_t)();

class MAINMODULE_EXPORT Cpu : public CpuBase, public CpuExtMem
{
    public:
        Cpu( eMcu* mcu );
        ~Cpu();

        virtual void reset() override;
        virtual void runStep() override;
        virtual void extClock( bool clkState ) override;

        enum cpuState_t{
            sRESET=0,
            sFETCH,
            sDECODE,
            sOPERS,
            sEXEC,
            sRESULT
        };
        enum addrMode_t{
            aNONE=0,
            aACCU,
            aABSO,
            aIMME,
            aINDI,
            aRELA
        };
        enum instType_t{
            iNONE=0,
            iJUMP,
            iREOP,
            iRMWI
        };

    protected:
        void clkRising();
        void clkFalling();

        void fetch();
        void decode();
        void operands();
        void execute();
        void result();

        void addrMode( addrMode_t mode );
        void instType( instType_t type );

        //void readMem();
        //void writeMem();

        //void addOp( void (Cpu::* func)() );
        //void addMemOp( memOp_t op );

        cpuState_t m_state;

        bool m_nextClock;

        uint8_t m_opCode;

        uint32_t m_opAddr;
        //uint8_t m_sDI;              // Data readen from Memory

        //int m_nOps;                // Number of Operations
        //int m_opIndex;               // Current Operation index
        funcPt_t m_instExec; // List of Operations to perform

        // Instructions
        void LDA();
};
#endif
