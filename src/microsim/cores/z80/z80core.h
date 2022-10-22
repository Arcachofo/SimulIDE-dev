#ifndef Z80CORE_H
#define Z80CORE_H

#include "cpubase.h"
#include "e-element.h"
#include "z80regs.h"

#define Z80CORE_MAX_T_INT 1000000   // Maximum T cycles after interrupt

class MAINMODULE_EXPORT Z80Core : public CpuBase, public eElement
{
    public:
        Z80Core( eMcu* mcu );
        ~Z80Core();

        virtual void stamp() override;
        virtual void runEvent() override;

        virtual void reset() override;
        virtual void runStep() override;
        virtual void extClock( bool clkState ) override;

        virtual int getCpuReg( QString reg ) override;
        virtual QString getStrReg( QString reg ) override;

        QString getStrInst();
        QString getStrMathOp( uint8_t reg );
        QString getStrFlag( uint8_t reg );
        QString getStrReg( uint8_t reg );
        QString getStrRegPair( uint8_t reg, bool set = false );
        QString getStrReg8();
        QString getStrReg16( uint8_t reg = 4 );
        QString showProp() { return ""; }
        void setValLabelText( QString ) {;}
        QString producer() { return m_enumUids.at((int)m_producer); }
        void setProducer( QString producer );
        bool cmos() { return m_cmos; }
        void setCmos( bool cmos );
        bool ioWait() { return !m_ioWait; }
        void setIoWait( bool ioWait );
        bool intVector() { return m_intVector; }
        void setIntVector( bool intVector );

    private:
        void risingEdgeDelayed();
        void fallingEdgeDelayed();
        void releaseBus( bool rel );

        // Setting of Z80Core
        enum eProducer { pZilog = 0, pNec, pSt };
        eProducer m_producer;

        enum eRegs { rB = 0, rC = 1, rD = 2, rE = 3, rH = 4, rL = 5, rXH = 6, rXL = 7, rYH = 8, rYL = 9 };      // don't change register numbers
        enum eRegPairs { rBC = rB, rDE = rD, rHL = rH, rIX = rXH, rIY = rYH, rAF = rIY + 2 };      // don't change register numbers
        
        bool m_cmos;
        bool m_ioWait;
        bool m_intVector;

        uint8_t sm_autoWait;
        bool sm_waitTState;
        bool sWait;
        bool sBusReq;
        bool sBusAck;

        enum eBusOperation { oNone, oM1, oIntAck, oMemRead, oMemWrite, oIORead, oIOWrite };
        eBusOperation mc_busOp;
        eBusOperation m_lastBusOp;

        bool m_nextClock;
        bool highImpedanceBus;

        uint64_t m_delay;
        IoPort* m_dataPort;
        IoPort* m_addrPort;

        // Z80Core sampled bus signals
        bool NMIFF;
        bool sLastNMI;
        bool sNMI;
        bool sInt;

        uint8_t sDI;
        uint8_t sDO;
        uint16_t sAO;

            // Output pins
        IoPin* m_m1Pin;
        IoPin* m_mreqPin;
        IoPin* m_iorqPin;
        IoPin* m_rdPin;
        IoPin* m_wrPin;
        IoPin* m_rfshPin;
        IoPin* m_haltPin;
        IoPin* m_busacPin;
           // Input Pins
        IoPin* m_waitPin;
        IoPin* m_intPin;
        IoPin* m_nmiPin;
        IoPin* m_busreqPin;
        IoPin* m_resetPin;


        // Z80Core registers
        uint8_t regs[10];   // B, C, D, E, H, L, XH, XL, YH, YL
        Z80RegPair regPairs[11] = { Z80RegPair( regs[rB], regs[rC] ), Z80RegPair( regs[rB], regs[rC] ),
                                    Z80RegPair( regs[rD], regs[rE] ), Z80RegPair( regs[rD], regs[rE] ),
                                    Z80RegPair( regs[rH], regs[rL] ), Z80RegPair( regs[rH], regs[rL] ),
                                    Z80RegPair( regs[rXH], regs[rXL] ), Z80RegPair( regs[rXH], regs[rXL] ),
                                    Z80RegPair( regs[rYH], regs[rYL] ), Z80RegPair( regs[rYH], regs[rYL] ),
                                    Z80RegPair( regA, regF ) };
        uint8_t regsAlt[6]; // B', C', D', E', H', L'
        uint8_t regA;
        Z80Flags regF;
        uint8_t regAAlt;
        uint8_t regFAlt;
        uint8_t regI;
        uint8_t regR;
        Z80Reg16 regWZ;
        Z80Reg16 regSP;
        bool IFF1;
        bool IFF2;

        // Z80Core MCode variables
        enum eStateMachine{
            sNone,
            sHalt,
            sXYOffsetFetch,
            sXYFetchFinish
        };

        enum ePrefix{
            noPrefix=0,
            prefixCB,
            prefixED,
            prefixIX,
            prefixIY
        };

        static const uint8_t sTableMCycles[3][256];
        static const uint8_t sTableM1TStates[3][256];
        uint8_t mc_TStates;
        uint8_t mc_MCycles;
        eStateMachine mc_StateMachine;
        ePrefix mc_prefix;
        
        // Z80Core variables
        ePrefix m_iSet;
        eRegPairs XYState;

        uint8_t m_iReg;
        uint8_t intMode;
        uint8_t rstCount;
        bool normalReset;
        bool specialReset;
        
        // Z80Core main state machine variables
        enum eM1CycleType { tOpCodeFetch, tInt, tNMI, tHalt, tSpecialReset };
        uint8_t sm_TState;
        bool sm_lastTState;
        uint32_t sm_TStatesAfterInt;
        uint8_t sm_MCycle;
        uint8_t sm_PreXYMCycle;
        eM1CycleType sm_M1CycleType;

        void clkRisingEdge();
        void clkFallingEdge();
        
        void nextTState();
        void opCodeFetch();
        void runMCode();
        
        // Instruction helpers
        inline void readMem( uint16_t addr ) { mc_busOp = oMemRead; sAO = addr; }
        inline void writeMem( uint16_t addr, uint8_t data ) { mc_busOp = oMemWrite; sAO = addr; sDO = data; }
        inline void readIO( uint16_t addr ) { mc_busOp = oIORead; sAO = addr; }
        inline void writeIO( uint16_t addr, uint8_t data ) { mc_busOp = oIOWrite; sAO = addr; sDO = data; }
        inline void noBusOp( uint16_t addr, uint8_t tstates ) { mc_busOp = oNone; sAO = addr; mc_TStates = tstates; }
        inline void pushStack8( uint8_t v ) { regSP--; writeMem( regSP, v ); }
        inline void popStack8() { readMem( regSP ); regSP++; }
        inline void jumpWZ() { m_PC = regWZ; sAO = m_PC; }
        inline void finishInst() { mc_MCycles = sm_MCycle; }
        inline uint8_t mask( const uint8_t &ireg ) { return 1 << ( (ireg >> 3) & 0x07 ); }

        // Instructions
        inline void flagsScfCcf();

        inline void ld_r_imm( uint8_t &reg );
        inline void ld_r_indir( uint8_t &reg, const Z80RegPair &rp );
        inline void ld_r_indirXY( uint8_t &reg );
        inline void ld_r_mem( uint8_t &reg );
        inline void ld_indir_r( const Z80RegPair &rp, const uint8_t &reg );
        inline void ld_indirXY_r( const uint8_t &reg );
        inline void ld_mem_r( const uint8_t &reg );
        inline void ld_indirXY_imm();

        template< typename T >
        inline void ld_rr_imm( T &rp );
        template< typename T >
        inline void ld_rr_mem( T &rp );
        template< typename T >
        inline void ld_mem_rr( T &rp );
        
        inline void push_rr( Z80RegPair &rp );
        inline void pop_rr( Z80RegPair &rp );
        
        inline void ex_rr_rr( Z80RegPair &rp, uint8_t &regH, uint8_t &regL );
        inline void ex_SP_rr( Z80RegPair &rp );

        enum eMathOp { iInc, iDec, iAdd, iAdc, iSub, iSbc, iAnd, iXor, iOr, iCp, iCpl, iNeg, iDaa };
        template<eMathOp op>
        inline void inst_indirXY();
        template<eMathOp op>
        inline void inst_imm();
        
        template< eMathOp op, bool changeAllFlags = true >
        inline void mathOp( uint8_t &regA, uint8_t reg = 0 );
        
        template< eMathOp op, typename T >
        inline void mathOp16( Z80RegPair &rp1, T &rp2 );
        
        enum eShiftOp { iRlc, iRrc, iRl, iRr, iSla, iSra, iSll, iSrl };
        template< eShiftOp op, bool changeSZP = true >
        inline void shiftOp( uint8_t &reg );
        
        enum eRotDig { iLeft, iRight };
        template< eRotDig op >
        inline void rlrd();
        
        inline void prefix_cb();
        inline void instCB( const uint8_t &ireg, uint8_t &reg );
        inline void instCBOp( const uint8_t &ireg, uint8_t &reg );

        inline void in_r_imm( uint8_t &reg );
        inline void in_r_rr( uint8_t &reg, const Z80RegPair &rp );
        inline void out_imm_r( const uint8_t &reg );
        inline void out_rr_r( const Z80RegPair &rp, const uint8_t &reg );
        
        inline void jr( const bool &cond );
        inline void jp( const bool &cond );
        inline void call( const bool &cond );
        inline void rst(const uint8_t &addr);
        inline void ret( const bool &cond = true );
        inline void retn();
        inline void int_im2();

        enum eBlock { bInc, bIncRep, bDec, bDecRep };
        template< eBlock type >
        inline void ldxx();
        template< eBlock type >
        inline void cpxx();
        template< eBlock type >
        inline void inxx();
        template< eBlock type >
        inline void otxx();
};
#endif
