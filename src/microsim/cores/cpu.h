/***************************************************************************
 *   Copyright (C) 2022 by Santiago González                               *
 *                                                                         *
 ***( see copyright.txt file at root folder )*******************************/

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
