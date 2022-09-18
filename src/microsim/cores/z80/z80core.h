#ifndef Z80CORE_H
#define Z80CORE_H

#include "cpubase.h"
#include "e-element.h"

#define Z80Core_MAX_T_INT 1000000   // Maximum T cycles after interrupt

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

        struct sRegPair {
            uint8_t &high;
            uint8_t &low;

            sRegPair( uint8_t &h, uint8_t &l ) : high(h), low(l) { }
            void setLowByte( const uint8_t &byte ) { low = byte; }
            void setHighByte( const uint8_t &byte ) { high = byte; }
            uint8_t& getLowByte() { return low; }
            uint8_t& getHighByte() { return high; }
            void operator++( int ) { uint16_t r16 = (high << 8) | low; r16++; high = (r16 >> 8); low = r16 & 0xff; } // the return value is intentionally void
            void operator--( int ) { uint16_t r16 = (high << 8) | low; r16--; high = (r16 >> 8); low = r16 & 0xff; } // the return value is intentionally void
            operator uint16_t() const { return (high << 8) | low; }
            void operator=( const uint16_t& word ) { high = word >> 8; low = word & 0xff; } // the return value is intentionally void
        };

        struct sReg16 {
            uint16_t reg;

            void setLowByte( const uint8_t &byte ) { reg &= 0xff00; reg |= byte; }
            void setHighByte( const uint8_t &byte ) { reg &= 0x00ff; reg |= (byte << 8); }
            uint8_t getLowByte() { return reg & 0x00ff; }
            uint8_t getHighByte() { return reg >> 8; }
            void operator++( int ) { reg++; }
            void operator--( int ) { reg--; }
            operator uint16_t() const { return reg; }
            uint16_t& operator=( const uint16_t& word ) { reg = word; return reg; }
        };
        
        enum eFlags { fC = 0x01, fN = 0x02, fP = 0x04, fX = 0x08, fH = 0x10, fY = 0x20, fZ = 0x40, fS = 0x80 };  // don't change flags numbers
        
        struct sFlags {
            uint8_t flags;
            bool changed = false;
            bool lastChanged = false;
            
            // Calculation parity of bits in reg. Result is true for odd number of bits 1 and false for even number of bits 1.
            bool parity( uint8_t reg ) { reg ^= (reg >> 1); reg ^= (reg >> 2); reg ^= (reg >> 4); return (reg & 0x01); }
            void clearChanged() { lastChanged = changed; changed = false; }
            bool isChanged() { return lastChanged; }
            
            bool getS() { return flags & fS; }
            bool getZ() { return flags & fZ; }
            bool getH() { return flags & fH; }
            bool getP() { return flags & fP; }
            bool getN() { return flags & fN; }
            bool getC() { return flags & fC; }
            sFlags& setH() { flags |= fH; changed = true; return *this; }
            sFlags& setN() { flags |= fN; changed = true; return *this; }
            sFlags& setC() { flags |= fC; changed = true; return *this; }
            sFlags& resetY() { flags &= ~fY; changed = true; return *this; }
            sFlags& resetH() { flags &= ~fH; changed = true; return *this; }
            sFlags& resetX() { flags &= ~fX; changed = true; return *this; }
            sFlags& resetN() { flags &= ~fN; changed = true; return *this; }
            sFlags& resetC() { flags &= ~fC; changed = true; return *this; }
            sFlags& resetHN() { flags &= ~( fH | fN ); changed = true; return *this; }
            sFlags& negC() { flags ^= fC; changed = true; return *this; }
            sFlags& copyYX(const uint8_t &r) { flags &= ~( fY | fX ); flags |= r & ( fY | fX ); changed = true; return *this; }
            sFlags& copyNegCtoH() { flags &= ~fH; flags |= (~flags << 4) & fH; changed = true; return *this; }
            sFlags& andZ(const bool &z) { if (!z) flags &= ~fZ; changed = true; return *this; }
            sFlags& orYX(const uint8_t &r) { flags |= r & ( fY | fX ); changed = true; return *this; }
            sFlags& storeP(const bool &p) { flags &= ~fP; flags |= p << 2 ; changed = true; return *this; }
            sFlags& storeC(const bool &c) { flags &= ~fC; flags |= c ; changed = true; return *this; }
            sFlags& storeHC(const bool &hc) { flags &= ~( fH | fC); flags |= hc | (hc << 4) ; changed = true; return *this; }
            sFlags& copySzeroZ(const uint8_t &r) { flags &= ~( fS | fZ ); flags |= r & fS;
                                                   if ( r == 0x00 ) flags |= fZ;
                                                   changed = true; return *this; }
            sFlags& copySzeroZP(const uint8_t &r) { flags &= ~( fS | fZ | fP ); flags |= r & fS;
                                                    if ( r == 0x00 ) flags |= fZ | fP;
                                                     changed = true; return *this; }
            sFlags& copySYXzeroZ(const uint8_t &r) { flags &= ~( fS | fZ | fY | fX ); flags |= r & ( fS | fY | fX );
                                                     if ( r == 0x00 ) flags |= fZ;
                                                      changed = true; return *this; }
            sFlags& copySYXzeroZparityP(const uint8_t &r) { flags &= ~( fS | fZ | fY | fX | fP ); flags |= r & ( fS | fY | fX );
                                                            if ( r == 0x00 ) flags |= fZ;
                                                            if ( parity(r) == false ) flags |= fP;
                                                            changed = true; return *this; }
            sFlags& overflowHP(const uint8_t &r) { flags &= ~( fH | fP );
                                                   if ( (r & 0x08) != 0x00 ) flags |= fH;
                                                   if ( (r & 0xC0) == 0x80 || (r & 0xC0) == 0x40 ) flags |= fP;
                                                   changed = true; return *this; }
            sFlags& overflowHC(const uint8_t &r) { flags &= ~( fH | fC );
                                                   if ( (r & 0x08) != 0x00 ) flags |= fH;
                                                   if ( (r & 0x80) != 0x00 ) flags |= fC;
                                                   changed = true; return *this; }
            sFlags& overflowHPC(const uint8_t &r) { flags &= ~( fH | fP | fC );
                                                     if ( (r & 0x08) != 0x00 ) flags |= fH;
                                                     if ( (r & 0xC0) == 0x80 || (r & 0xC0) == 0x40 ) flags |= fP;
                                                     if ( (r & 0x80) != 0x00 ) flags |= fC;
                                                    changed = true; return *this; }
            sFlags& parityP(const uint8_t &r) { flags &= ~fP; if ( parity(r) == false ) flags |= fP; changed = true; return *this; }
            sFlags& copyBit1YX(const uint8_t &r) { flags &= ~( fY | fX ); flags |= r & fX; if ( (r & 0x02) != 0x00 ) flags |= fY; changed = true; return *this; }
            sFlags& copyBit7N(const uint8_t &r) { flags &= ~fN; if ( (r & 0x80) != 0x00 ) flags |= fN; changed = true; return *this; }
            sFlags& inotxxPH(const uint8_t &r1, const uint8_t &r2);
            
            operator uint8_t() const { return flags; }
            uint8_t& operator=( const uint16_t& f ) { flags = f; return flags; }
        };
        
        bool m_cmos;
        bool m_ioWait;
        bool m_intVector;

        uint8_t sm_autoWait;
        bool sm_waitTState;
        bool sWait;
        bool sBusReq;
        bool sBusAck;

        enum eBusOperation { oNone, oM1, oM1Int, oMemRead, oMemWrite, oIORead, oIOWrite };
        eBusOperation mc_busOp;
        eBusOperation m_lastBusOp;

        bool m_nextClock;
        bool highImpedanceBus;

        uint64_t m_delay;
        IoPort* m_dataPort;
        IoPort* m_addrPort;

        // Z80Core sampled bus signals
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
        sRegPair regPairs[11] = { sRegPair( regs[rB], regs[rC] ), sRegPair( regs[rB], regs[rC] ),
                                 sRegPair( regs[rD], regs[rE] ), sRegPair( regs[rD], regs[rE] ),
                                 sRegPair( regs[rH], regs[rL] ), sRegPair( regs[rH], regs[rL] ),
                                 sRegPair( regs[rXH], regs[rXL] ), sRegPair( regs[rXH], regs[rXL] ),
                                 sRegPair( regs[rYH], regs[rYL] ), sRegPair( regs[rYH], regs[rYL] ),
                                 sRegPair( regA, regF.flags ) };
        uint8_t regsAlt[6]; // B', C', D', E', H', L'
        uint8_t regA;
        sFlags regF;
        uint8_t regAAlt;
        uint8_t regFAlt;
        uint8_t regI; // in z80ioby now
        uint8_t regR; // in z80io by now
        sReg16 regWZ;
        sReg16 regSP;
        //uint16_t regPC; Using McuCore::PC
        bool IFF1; // in z80io by now
        bool IFF2;
        bool regFChanged; // to delete
        bool lastRegFChanged; // to delete

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
        ePrefix m_iSet;  // in z80io by now
        eRegPairs XYState;

        uint8_t m_iReg; // in z80io by now
        uint8_t intMode;
        uint8_t rstCount;
        bool normalReset; // in z80io by now
        bool specialReset;

        
        // Z80Core main state machine variables
        enum eM1CycleType { tOpCodeFetch, tInt, tNMI, tHalt, tSpecialReset };
        uint8_t sm_TState; // in z80io by now
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
        inline void ld_r_indir( uint8_t &reg, const sRegPair &rp );
        inline void ld_r_indirXY( uint8_t &reg );
        inline void ld_r_mem( uint8_t &reg );
        inline void ld_indir_r( const sRegPair &rp, const uint8_t &reg );
        inline void ld_indirXY_r( const uint8_t &reg );
        inline void ld_mem_r( const uint8_t &reg );
        inline void ld_indirXY_imm();

        template< typename T >
        inline void ld_rr_imm( T &rp );
        template< typename T >
        inline void ld_rr_mem( T &rp );
        template< typename T >
        inline void ld_mem_rr( T &rp );
        
        inline void push_rr( sRegPair &rp );
        inline void pop_rr( sRegPair &rp );
        
        inline void ex_rr_rr( sRegPair &rp, uint8_t &regH, uint8_t &regL );
        inline void ex_SP_rr( sRegPair &rp );

        enum eMathOp { iInc, iDec, iAdd, iAdc, iSub, iSbc, iAnd, iXor, iOr, iCp, iCpl, iNeg, iDaa };
        template<eMathOp op>
        inline void inst_indirXY();
        template<eMathOp op>
        inline void inst_imm();
        
        template< eMathOp op, bool changeAllFlags = true >
        inline void mathOp( uint8_t &regA, uint8_t reg = 0 );
        
        template< eMathOp op, typename T >
        inline void mathOp16( sRegPair &rp1, T &rp2 );
        
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
        inline void in_r_rr( uint8_t &reg, const sRegPair &rp );
        inline void out_imm_r( const uint8_t &reg );
        inline void out_rr_r( const sRegPair &rp, const uint8_t &reg );
        
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
