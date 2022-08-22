#ifndef Z80CPU_H
#define Z80CPU_H

// Macro to enable loggging during test https://github.com/raxoft/z80test into file
//#define Z80CPU_TEST
// Macro to enable Z80CPU monitor
//#define Z80CPU_MONITOR
// Macro to enable measuring time
//#define Z80CPU_LOAD

#include "iocomponent.h"
#include "e-clocked_device.h"
#ifdef Z80CPU_MONITOR
    #include "z80cpumonitor.h"
#endif

#define Z80CPU_MAX_T_INT 1000000   // Maximum T cycles after interrupt

#ifdef Z80CPU_TEST
    #include <fstream>
    #include <iomanip>
    #include <string>
#endif

class LibraryItem;
class Z80CPUMonitor;

class MAINMODULE_EXPORT Z80CPU : public IoComponent, public eClockedDevice
{
        Q_OBJECT
    public:
        Z80CPU( QObject* parent, QString type, QString id );
        ~Z80CPU();

        static Component* construct( QObject* parent, QString type, QString id );
        static LibraryItem* libraryItem();

        virtual void stamp() override;
        virtual void initialize() override;
        virtual void voltChanged() override;
        virtual void runEvent() override;
        
        double freq() { return m_freq; }
        void setFreq( double freq );
        bool extClkEnabled() { return m_extClock; }
        void enableExtClk( bool en );
        QString producer() { return m_enumUids.at((int)m_producer); }
        void setProducer( QString producer );
        bool cmos() { return m_cmos; }
        void setCmos( bool cmos );
        bool ioWait() { return !m_ioWait; }
        void setIoWait( bool ioWait );
        bool intVector() { return m_intVector; }
        void setIntVector( bool intVector );

    public slots:
#ifdef Z80CPU_MONITOR
        void showZ80CPUMonitor();
#endif
        virtual void contextMenu( QGraphicsSceneContextMenuEvent* event, QMenu* menu );

    protected:
        virtual void contextMenuEvent( QGraphicsSceneContextMenuEvent* event );

    private:
#ifdef Z80CPU_MONITOR
        Z80CPUMonitor* m_z80cpuMonitor;
#endif        
        // Setting of Z80CPU
        enum eProducer { pZilog = 0, pNec, pSt };
        bool m_extClock;
        double m_freq;
        eProducer m_producer;
        bool m_cmos;
        bool m_ioWait;
        bool m_intVector;

        // Single step commands
        bool m_stepT;
        bool m_stepMC;
        bool m_stepInst;

#ifdef Z80CPU_TEST
        std::ofstream test_file;
        bool logging;
        unsigned short log_addr;
        void store_registers();
        void log_registers();
        void log_mc();
#endif

        // Z80CPU registers
        static const unsigned char rB = 0, rC = 1, rD = 2, rE = 3, rH = 4, rL = 5, rXH = 6, rXL = 7, rYH = 8, rYL = 9;      // don't change register numbers
        static const unsigned char fC = 0x01, fN = 0x02, fP = 0x04, fX = 0x08, fH = 0x10, fY = 0x20, fZ = 0x40, fS = 0x80;  // don't change register numbers
        enum eXY { rHL = rH, rIX = rXH, rIY = rYH };
        unsigned char regs[10];   // B, C, D, E, H, L, XH, XL, YH, YL
        unsigned char regsAlt[6]; // B', C', D', E', H', L'
        unsigned char regA;
        unsigned char regF;
        unsigned char regAAlt;
        unsigned char regFAlt;
        unsigned char regI;
        unsigned char regR;
        unsigned short regWZ;
        unsigned short regSP;
        unsigned short regPC;
        bool IFF1;
        bool IFF2;
        bool regFChanged;
        bool lastRegFChanged;

        // Z80CPU MCode variables
        enum eStateMachine { sNone, sHalt, sXYOffsetFetch, sXYFetchFinish };
        enum eBusOperation { oNone, oM1, oM1Int, oMemRead, oMemWrite, oIORead, oIOWrite };
        enum ePrefix { noPrefix = 0, prefixCB = 1, prefixED = 2, prefixIX = 3, prefixIY = 4 };
        static const unsigned char sTableMCycles[3][256];
        static const unsigned char sTableM1TStates[3][256];
        unsigned char mc_TStates;
        unsigned char mc_MCycles;
        eStateMachine mc_StateMachine;
        ePrefix mc_prefix;
        eBusOperation mc_busOperation;
        
        // Z80CPU variables
        ePrefix instructionSet;
        eXY XYState;
        eBusOperation lastBusOperation;
        unsigned char instructionReg;
        unsigned char intMode;
        unsigned char resetCounter;
        bool normalReset;
        bool specialReset;
        bool highImpedanceBus;
        
        // Z80CPU main state machine variables
        enum eM1CycleType { tOpCodeFetch, tInt, tNMI, tHalt, tSpecialReset };
        unsigned char sm_TState;
        bool sm_lastTState;
        unsigned long sm_TStatesAfterInt;
        unsigned char sm_MCycle;
        unsigned char sm_PreXYMCycle;
        eM1CycleType sm_M1CycleType;
        unsigned char sm_autoWait;
        bool sm_waitTState;
        
        // Z80CPU sampled bus signals
        bool sNMI;
        bool sInt;
        bool sWait;
        bool sBusReq;
        bool sBusAck;
        unsigned char sDI;
        unsigned char sDO;
        unsigned short sAO;

        // Output pins
        IoPin* m_m1Pin;
        IoPin* m_haltPin;

        IoPin* m_mreqPin;
        IoPin* m_iorqPin;
        IoPin* m_rdPin;
        IoPin* m_wrPin;
        IoPin* m_rfshPin;
        IoPin* m_busacPin;

        // Input Pins
        IoPin* m_waitPin;
        IoPin* m_intPin;
        IoPin* m_nmiPin;
        IoPin* m_resetPin;
        IoPin* m_busreqPin;

        IoPin* m_vccPin;
        IoPin* m_gndPin;

        void reset();
        void singleT();
        void singleMC();
        void singleInst();
        
        inline void releaseDataBus();
        inline unsigned char readDataBus();
        inline void writeDataBus(unsigned char dataBus);
        inline void writeAddrBus(unsigned short addrBus);
        
        void clkRisingEdge();
        void clkRisingEdgeDelayed();
        void clkFallingEdge();
        void clkFallingEdgeDelayed();
        
        void nextTState();
        void opCodeFetch();
        void runMCode();
        
        inline bool parity(unsigned char reg);
        
        // Instructions
        inline void scf();
        inline void ccf();
        inline void flags_IR();
        inline void flags_bit(const unsigned char &result);
        
        inline void ld_r_imm(unsigned char &reg);
        inline void ld_r_indir(unsigned char &reg, const unsigned char &regH, const unsigned char &regL);
        inline void ld_r_indirXY(unsigned char &reg);
        inline void ld_r_mem(unsigned char &reg);
        inline void ld_indir_r(const unsigned char &regH, const unsigned char &regL, const unsigned char &reg);
        inline void ld_indirXY_r(const unsigned char &reg);
        inline void ld_mem_r(const unsigned char &reg);
        inline void ld_indirXY_imm();

        inline void ld_rr_imm(unsigned char &regH, unsigned char &regL);
        inline void ld_SP_imm();
        inline void ld_rr_mem(unsigned char &regH, unsigned char &regL);
        inline void ld_SP_mem();
        inline void ld_mem_rr(const unsigned char &regH, const unsigned char &regL);
        
        inline void push_rr(const unsigned char &regH, const unsigned char &regL);
        inline void pop_rr(unsigned char &regH, unsigned char &regL);
        
        inline void ex_rr_rr(unsigned char &regH1, unsigned char &regL1, unsigned char &regH2, unsigned char &regL2);
        inline void ex_SP_rr(unsigned char &regH, unsigned char &regL);
        
        inline void inc_r(unsigned char &reg);
        inline void dec_r(unsigned char &reg);
        inline void inc_indirXY();
        inline void dec_indirXY();
        
        inline void inc_rr(unsigned char &regH, unsigned char &regL);
        inline void dec_rr(unsigned char &regH, unsigned char &regL);

        enum eInstruction { iAdd, iAdc, iSub, iSbc, iAnd, iXor, iOr, iCp };
        template<eInstruction inst>
        inline void inst_indirXY();
        template<eInstruction inst>
        inline void inst_imm();
        
        inline void add_r(const unsigned char &reg);
        inline void adc_r(const unsigned char &reg);
        inline void sub_r(const unsigned char &reg);
        inline void sbc_r(const unsigned char &reg);
        inline void and_r(const unsigned char &reg);
        inline void xor_r(const unsigned char &reg);
        inline void or_r(const unsigned char &reg);
        inline void cp_r(const unsigned char &reg);
        inline void daa();
        inline void cpl();
        inline void neg();

        inline void add_rr_rr(unsigned char &regH1, unsigned char &regL1, const unsigned char &regH2, const unsigned char &regL2);
        inline void adc_rr_rr(unsigned char &regH1, unsigned char &regL1, const unsigned char &regH2, const unsigned char &regL2);
        inline void sbc_rr_rr(unsigned char &regH1, unsigned char &regL1, const unsigned char &regH2, const unsigned char &regL2);
        
        inline void rlca();
        inline void rrca();
        inline void rla();
        inline void rra();
        inline void rld();
        inline void rrd();
        
        enum eInstructionCB { iRlc, iRrc, iRl, iRr, iSla, iSra, iSll, iSrl, iBit, iRes, iSet };
        template<eInstructionCB inst, unsigned char mask = 0x00>
        inline void instCB(unsigned char &reg);
        
        inline void rlc_r(unsigned char &reg);
        inline void rrc_r(unsigned char &reg);
        inline void rl_r(unsigned char &reg);
        inline void rr_r(unsigned char &reg);
        inline void sla_r(unsigned char &reg);
        inline void sra_r(unsigned char &reg);
        inline void sll_r(unsigned char &reg);
        inline void srl_r(unsigned char &reg);

        inline void in_r_imm(unsigned char &reg);
        inline void in_r_rr(unsigned char &reg, const unsigned char &regH, const unsigned char &regL);
        inline void out_imm_r(const unsigned char &reg);
        inline void out_rr_r(const unsigned char &regH, const unsigned char &regL, const unsigned char &reg);
        
        inline void djnz(unsigned char &reg);
        inline void jr(const bool &cond);
        inline void jp(const bool &cond);
        inline void call(const bool &cond);
        inline void rst(const unsigned char &addr);
        inline void ret(const bool &cond);
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
        
        friend class Z80CPUMonitor;
};

#endif
