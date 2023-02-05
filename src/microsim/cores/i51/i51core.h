/***************************************************************************
 *   Copyright (C) 2020 by Santiago González                               *
 *                                                                         *
 ***( see copyright.txt file at root folder )*******************************/

#ifndef I51CORE_H
#define I51CORE_H

#include "mcucpu.h"

/*enum EM8051_EXCEPTION
{
    EXCEPTION_STACK,             // stack address > 127 with no upper memory, or roll over
    EXCEPTION_ACC_TO_A,          // acc-to-a move operation; illegal (acc-to-acc is ok, a-to-acc is ok..)
    EXCEPTION_IRET_PSW_MISMATCH, // psw not preserved over interrupt call (doesn't care about P, F0 or UNUSED)
    EXCEPTION_IRET_SP_MISMATCH,  // sp not preserved over interrupt call
    EXCEPTION_IRET_ACC_MISMATCH, // acc not preserved over interrupt call
    EXCEPTION_ILLEGAL_OPCODE     // for the single 'reserved' opcode in the architecture
};*/

class MAINMODULE_EXPORT I51Core : public McuCpu, public eElement
{
    public:
        I51Core( eMcu* mcu );
        ~I51Core();

        enum { P=0,k,OV,RS0,RS1,F0,AC,Cy }; // Status bits

        enum SFR_REGS // SFR register locations
        {
            REG_SBUF = 0x99,
            REG_B    = 0xF0,
            REG_DPL  = 0x82,
            REG_DPH  = 0x83,
        };

        enum cpuState_t{
            cpu_RESET=0,
            cpu_FETCH,
            cpu_OPERAND,
            cpu_EXEC,
            cpu_WRITE
        };

        enum addrMode_t{
            aNONE=0,
            aDIRE=1,
            aINDI=1<<1,
            aIMME=1<<2,
            aRELA=1<<3,

            aORIG=1<<4,
            aPGM =1<<5,
            aBIT =1<<6
        };

        virtual void stamp() override;
        virtual void runEvent() override;

        virtual void reset() override;
        virtual void runStep() override;

    protected:

        uint64_t m_psStep;  // Half Clock cycle ps = 1/24 Machine cycle, = 1/12 Read cycle.
        int m_cycle;

        bool m_readExtPGM;

        cpuState_t m_cpuState;

        uint16_t m_tmpPC;
        uint8_t m_pgmData;
        uint8_t m_opcode;
        uint8_t* m_acc;
        
        QVector<uint8_t> m_dataEvent;
        uint8_t m_addrMode;
        uint16_t m_opAddr;
        uint8_t m_op0;
        uint8_t m_op2;
        uint8_t m_RxAddr;
        uint8_t m_bitAddr;
        uint8_t m_bitMask;
        bool    m_invert;

        bool m_upperData;

        IoPin* m_eaPin;

        //McuPin* m_rwPin; // !Write / Read Pin
        //McuPin* m_rePin; // !Read Pin
        IoPin* m_enPin; // !Enable Pin
        IoPin* m_laPin; //  Latch Enable in Low+High Address mode (8051 ALE)
                        //  Acting on Data Pins. Active High

        McuPort* m_dataPort;
        McuPort* m_addrPort;

        enum memState_t{
            mem_IDLE=0,
            mem_LAEN,
            mem_ADDR,
            mem_LADI,
            mem_DATA,
            mem_READ
        };
        memState_t m_memState;
        bool m_read;
        bool m_extPGM;

        uint8_t  m_addrH;
        uint32_t m_addr;
        uint32_t m_data;

        // All these times Respect to cycle start:
        // |--CYCLE-----------------------------------|
        //NO uint64_t m_laEnSetTime; // |--laEnSet--|
        uint64_t m_addrSetTime; // |--addrSet------|
        uint64_t m_laEnEndTime; // |--laEnEnd----------|
        uint64_t m_readSetTime; // |--readSet--------------|...|
        uint64_t m_writeSetTime;// |--writeSet-------------|...|
        uint64_t m_readBusTime; // |--readBus-------------------------|
        uint64_t m_dataTime;    // to store previous times






        inline void readOperand();
        inline void Exec();
        inline void Decode();

        inline void operRgx();
        inline void operInd();
        inline void operI08();
        inline void operDir();
        inline void operRel();

        inline void addrRgx();
        inline void addrInd();
        inline void addrI08();
        inline void addrDir();
        inline void addrBit( bool invert=false );

        inline uint16_t checkAddr( uint16_t addr )
        {
            if( m_upperData && (addr > m_lowDataMemEnd) ) addr += m_regEnd ;
            return addr;
        }

        inline uint8_t getValue( uint16_t addr ) // Read Fake Input instead
        {
            if( addr == REG_SBUF ) addr++;     // Fake Uart Input Register
            return GET_RAM( addr );
        }

        inline uint8_t GET_RAM( uint16_t addr ) override
        {
            addr = checkAddr( addr );
            return McuCpu::GET_RAM( addr );
        }
        inline void SET_RAM( uint16_t addr , uint8_t val )
        {
            addr = checkAddr( addr );
            McuCpu::SET_RAM( addr, val );
        }

        inline void    pushStack8( uint8_t v );
        inline uint8_t popStack8();

        inline void addFlags( uint8_t value1, uint8_t value2, uint8_t acc );
        inline void subFlags( uint8_t value1, uint8_t value2 );

        // Instructions --------------------------------
        inline void JMP();
        inline void SJMP();
        inline void AJMP();
        inline void LJMP();
        inline void ACALL();
        inline void LCALL();
        inline void RET();

        inline void JB();
        inline void JBC();
        inline void JNB();
        inline void JC();
        inline void JNC();
        inline void JZ();
        inline void JNZ();
        inline void DJNZ();
        inline void CJNE();

        inline void PUSH();
        inline void POP();

        inline void CPLc();
        inline void MOVbc() ;
        inline void SETBc();
        inline void MOVc() ;
        inline void ORLc();
        inline void ANLc();
        inline void CLRc();
        inline void CLRb();
        inline void CPLb();
        inline void SETBb();

        inline void CLRa();
        inline void CPLa();
        inline void DAa();
        inline void SWAPa();

        inline void MULab();
        inline void DIVab();
        inline void INCd();
        inline void INC();
        inline void DEC();
        inline void ADD();
        inline void ADDC();
        inline void SUBB();
        inline void ORLm();
        inline void ANLm();
        inline void XRLm();
        inline void ORLa();
        inline void ANLa();
        inline void XCH();
        inline void XCHD();
        inline void XRLa();
        inline void RR();
        inline void RRC();
        inline void RL();
        inline void RLC();

        inline void MOVCp();
        inline void MOVd();
        inline void MOVCd();
        inline void mov_indir_rx_imm();
        inline void mov_mem_a();
        inline void MOVr();
        inline void MOVa();
        inline void MOVm();

        inline void movx_a_indir_dptr();
        inline void movx_a_indir_rx();
        inline void movx_indir_dptr_a();
        inline void movx_indir_rx_a();
};

#endif
