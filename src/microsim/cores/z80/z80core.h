#ifndef Z80CORE_H
#define Z80CORE_H

#include "cpubase.h"
#include "z80io.h"

#define Z80Core_MAX_T_INT 1000000   // Maximum T cycles after interrupt

class MAINMODULE_EXPORT Z80Core : public CpuBase, public Z80io
{
    public:
        Z80Core( eMcu* mcu );
        ~Z80Core();

        virtual void reset() override;
        virtual void runStep() override;
        virtual void extClock( bool clkState ) override;

        virtual int getCpuReg( QString reg ) override;
        virtual QString getStrReg( QString reg ) override;

QString showProp() { return ""; }
void setValLabelText( QString ) {;}
QStringList m_enumUids;
QString producer() { return m_enumUids.at((int)m_producer); }

        void setProducer( QString producer );
        bool cmos() { return m_cmos; }
        void setCmos( bool cmos );
        bool ioWait() { return !m_ioWait; }
        void setIoWait( bool ioWait );
        bool intVector() { return m_intVector; }
        void setIntVector( bool intVector );

    private:
        // Setting of Z80Core
        enum eProducer { pZilog = 0, pNec, pSt };
        eProducer m_producer;

        bool m_cmos;
        bool m_ioWait;
        bool m_intVector;

        // Z80Core registers
        static const uint8_t rB = 0, rC = 1, rD = 2, rE = 3, rH = 4, rL = 5, rXH = 6, rXL = 7, rYH = 8, rYL = 9;      // don't change register numbers
        static const uint8_t fC = 0x01, fN = 0x02, fP = 0x04, fX = 0x08, fH = 0x10, fY = 0x20, fZ = 0x40, fS = 0x80;  // don't change register numbers
        enum eXY { rHL = rH, rIX = rXH, rIY = rYH };
        uint8_t regs[10];   // B, C, D, E, H, L, XH, XL, YH, YL
        uint8_t regsAlt[6]; // B', C', D', E', H', L'
        uint8_t regA;
        uint8_t regF;
        uint8_t regAAlt;
        uint8_t regFAlt;
        /// uint8_t regI; // in z80io by now
        /// uint8_t regR; // in z80io by now
        uint16_t regWZ;
        uint16_t regSP;
        //uint16_t regPC; Using CpuBase::PC
        ///bool IFF1; // in z80io by now
        bool IFF2;
        bool regFChanged;
        bool lastRegFChanged;

        // Z80Core MCode variables
        enum eStateMachine{
            sNone,
            sHalt,
            sXYOffsetFetch,
            sXYFetchFinish
        };

        /// enum ePrefix{ // in z80io by now
        ///     noPrefix=0,
        ///     prefixCB,
        ///     prefixED,
        ///     prefixIX,
        ///     prefixIY
        /// };
        ///
        static const uint8_t sTableMCycles[3][256];
        static const uint8_t sTableM1TStates[3][256];
        uint8_t mc_TStates;
        uint8_t mc_MCycles;
        eStateMachine mc_StateMachine;
        ePrefix mc_prefix;
        
        // Z80Core variables
        /// ePrefix m_iSet;  // in z80io by now
        eXY XYState;

        /// uint8_t m_iReg; // in z80io by now
        uint8_t intMode;
        uint8_t rstCount;
        /// bool normalReset; // in z80io by now
        bool specialReset;

        
        // Z80Core main state machine variables
        enum eM1CycleType { tOpCodeFetch, tInt, tNMI, tHalt, tSpecialReset };
        /// uint8_t sm_TState; // in z80io by now
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
        
        inline bool parity( uint8_t reg );

        // Instruction helpers
        inline void writeMem( uint16_t addr, uint8_t data );
        //inline void sub( uint8_t &reg );
        //inline void setFlagSub( uint8_t &reg );
        inline void flags_IR();
        inline void flags_bit( const uint8_t &result );
        inline void flagsCHP( uint8_t reg );
        inline void flagsZP( uint8_t &reg );
        inline void flagsPH();

        // Instructions
        inline void scf();
        inline void ccf();

        inline void ld_r_imm( uint8_t &reg );
        inline void ld_r_indir(uint8_t &reg, const uint8_t &regH, const uint8_t &regL);
        inline void ld_r_indirXY( uint8_t &reg );
        inline void ld_r_mem( uint8_t &reg );
        inline void ld_indir_r(const uint8_t &regH, const uint8_t &regL, const uint8_t &reg);
        inline void ld_indirXY_r( const uint8_t &reg );
        inline void ld_mem_r( const uint8_t &reg );
        inline void ld_indirXY_imm();

        inline void ld_rr_imm(uint8_t &regH, uint8_t &regL);
        inline void ld_SP_imm();
        inline void ld_rr_mem(uint8_t &regH, uint8_t &regL);
        inline void ld_SP_mem();
        inline void ld_mem_rr(const uint8_t &regH, const uint8_t &regL);
        
        inline void push_rr(const uint8_t &regH, const uint8_t &regL);
        inline void pop_rr(uint8_t &regH, uint8_t &regL);
        
        inline void ex_rr_rr(uint8_t &regH1, uint8_t &regL1, uint8_t &regH2, uint8_t &regL2);
        inline void ex_SP_rr(uint8_t &regH, uint8_t &regL);
        
        inline void inc_r( uint8_t &reg );
        inline void dec_r( uint8_t &reg );
        inline void inc_indirXY();
        inline void dec_indirXY();
        
        inline void inc_rr(uint8_t &regH, uint8_t &regL);
        inline void dec_rr(uint8_t &regH, uint8_t &regL);

        enum eInstruction { iAdd, iAdc, iSub, iSbc, iAnd, iXor, iOr, iCp };
        template<eInstruction inst>
        inline void inst_indirXY();
        template<eInstruction inst>
        inline void inst_imm();
        
        inline void add_r( const uint8_t &reg );
        inline void adc_r( const uint8_t &reg );
        inline void sub_r( const uint8_t &reg );
        inline void sbc_r( const uint8_t &reg );
        inline void and_r( const uint8_t &reg );
        inline void xor_r( const uint8_t &reg );
        inline void or_r( const uint8_t &reg );
        inline void cp_r( const uint8_t &reg );
        inline void daa();
        inline void cpl();
        inline void neg();

        inline void add_rr_rr( uint8_t &regH1, uint8_t &regL1, const uint8_t &regH2, const uint8_t &regL2 );
        inline void adc_rr_rr( uint8_t &regH1, uint8_t &regL1, const uint8_t &regH2, const uint8_t &regL2 );
        inline void sbc_rr_rr( uint8_t &regH1, uint8_t &regL1, const uint8_t &regH2, const uint8_t &regL2 );
        
        inline void rlca();
        inline void rrca();
        inline void rla();
        inline void rra();
        inline void rld();
        inline void rrd();
        
        enum eInstructionCB { iRlc, iRrc, iRl, iRr, iSla, iSra, iSll, iSrl, iBit, iRes, iSet };
        template<eInstructionCB inst, uint8_t mask = 0x00>
        inline void instCB( uint8_t &reg );
        
        inline void rlc_r( uint8_t &reg );
        inline void rrc_r( uint8_t &reg );
        inline void rl_r( uint8_t &reg );
        inline void rr_r( uint8_t &reg );
        inline void sla_r( uint8_t &reg );
        inline void sra_r( uint8_t &reg );
        inline void sll_r( uint8_t &reg );
        inline void srl_r( uint8_t &reg );

        inline void in_r_imm( uint8_t &reg );
        inline void in_r_rr( uint8_t &reg, const uint8_t &regH, const uint8_t &regL );
        inline void out_imm_r( const uint8_t &reg );
        inline void out_rr_r( const uint8_t &regH, const uint8_t &regL, const uint8_t &reg );
        
        inline void djnz( uint8_t &reg );
        inline void jr( const bool &cond );
        inline void jp( const bool &cond );
        inline void call( const bool &cond );
        inline void rst(const uint8_t &addr);
        inline void ret( const bool &cond );
        inline void retn();
        inline void int_im2();

        enum eBlock { bInc, bIncRep, bDec, bDecRep };
        template<eBlock type>
        inline void ldxx();
        template<eBlock type>
        inline void cpxx();
        template<eBlock type>
        inline void inxx();
        template<eBlock type>
        inline void otxx();
        
        inline void prefix_cb();
};
#endif
