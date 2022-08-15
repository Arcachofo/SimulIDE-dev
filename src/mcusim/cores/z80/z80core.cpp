
#include "z80core.h"
#include "z80cycles.h"
#include "mcupin.h"
#include "simulator.h"

Z80Core::Z80Core( eMcu* mcu )
       : McuCore( mcu )
       , Z80io( mcu )
{
    // CPU registers to show in Monitor
    // By now only uint8_t
    /// TODO: Add other data types.
    m_cpuRegs.insert("T_STATE", &sm_TState );
    m_cpuRegs.insert("OpCode", &m_iReg );
    mcu->getCpuTable()->setRegisters( m_cpuRegs.keys() );

    m_STATUS = &regF;
    mcu->setStatusBits({"C","N","PV","y","H","x","Z","S"});

    // initialization Z80Core settings
    m_producer = pZilog;                 // producer is Zilog - behaviour of instructions SCF and CCF
    m_cmos = false;                      // version of processor - behaviour of instruction OUT (C),0 / OUT (C),FF
    m_ioWait = true;                     // insert one wait state during I/O operations
    m_intVector = false;                 // interrupt vector for mode 2 is read from data bu

    m_delay = 10e3; // 10 ns

    m_resetPin  = mcu->getPin("RESET");
}
Z80Core::~Z80Core() {}

void Z80Core::reset()
{
    Simulator::self()->cancelEvents( this );

    releaseBus( false ); /// What is the state of addr Bus at reset???

    normalReset = false;                /// reset flag normalReset
    specialReset = false;               /// reset flag specialReset
    rstCount = 0;                       /// reset TState counter for reset
    m_nextClock = true; /// ???

    // Reset Z80Core registers
    regA = 0xFF;
    regF = 0xFF;
    regAAlt = 0xFF;
    regFAlt = 0xFF;
    regI = 0;
    regR = 0;
    regWZ = 0;
    regSP = 0xFFFF;
    PC = 0;
    IFF1 = false;
    IFF2 = false;
    regFChanged = false;
    lastRegFChanged = false;

    // Reset Z80Core MCode variables
    mc_TStates = 4;
    mc_MCycles = 1;
    mc_StateMachine = sNone;
    mc_prefix = noPrefix;
    mc_busOp = oNone;

    // Reset Z80Core variables
    m_iSet = noPrefix;
    XYState = rHL;
    m_lastBusOp = oNone;
    m_iReg = 0x00;
    intMode = 0;
    highImpedanceBus = true;

    // Reset Z80Core main state machine variables
    sm_TState = 0;
    sm_lastTState = true;
    sm_TStatesAfterInt = Z80Core_MAX_T_INT;
    sm_MCycle = 1;
    sm_PreXYMCycle = 1;
    sm_M1CycleType = tOpCodeFetch;
    sm_waitTState = false;
    sm_autoWait = 0;
    sm_waitTState = false;

    // Reset Z80Core sampled bus signals
    sNMI = false;
    sInt = false;
    sWait = false;
    sBusReq = false;
    sBusAck = false;
    sDI = 0x00;
    sDO = 0x00;
    sAO = 0x0000;
}

void Z80Core::runClock( bool clkState ) // External Clock
{
    if( clkState == m_nextClock ) runDecoder();
}

void Z80Core::runDecoder() // Called every 1 full Clock cycle
{
    if( m_nextClock ) clkRisingEdge();
    else              clkFallingEdge();
    m_nextClock = !m_nextClock;
}

void Z80Core::clkRisingEdge() // Execution of instruction and sampling bus signal at clock rising edge
{
    //  Reset is accepted after three TStates
    if( !m_resetPin->getInpState() ){               // if reset input is active, then count TStates until it reachs value 2, then set flag normalReset
        if( !normalReset ){                         // the flag normalReset is not set yet
            if( rstCount == 2 ) normalReset = true; // set flag normalReset
            else                rstCount++;         // increase TStates counter for reset
        }
    }else {                                         // if reset input is not active, then count TStates for reset down until it reach value 0, then reset CPU
        if( normalReset ){                          // the flag normalReset is still set
            if( rstCount > 0 ) rstCount--;          // decrease TStates counter for reset
            else               reset();             // when TStates counter for reset reach 0 reset CPU
        }
        else rstCount = 0;                          // restart TStates counter for reset when the reset pulse is shorter than 3 TStates
    }
    // Increase counter TStates after interrupt (it is limited to Z80Core_MAX_T_INT)
    if( sm_TStatesAfterInt < Z80Core_MAX_T_INT ) sm_TStatesAfterInt++;

    // Increase TStaste and execute instructions only when the bus is used by cpu, otherwise cpu is halted
    if( !normalReset && ( !sBusReq || !sBusAck ) )
    {
        sBusAck = false;                             // finish bus request
        m_lastBusOp = mc_busOp;                      // store previous bus op.

        // Execute instruction at rising edge of TState 1
        if( sm_lastTState && !sBusReq )             // is executed at last TState, but after bus request and release when it is inserted
            runMCode();                             // execute instruction - this function change number of machine cycles and TStates
        nextTState();                               // go to the next TState

        if( sm_TState == 3 && sm_MCycle == 1 ) opCodeFetch(); // Op Code Fetch at rising edge of TState 3

        // Op Code Fetch at rising edge of TState 4 and machine cycle 7 in case of prefix CB
        // This is simple version of function opCodeFetch()
        // Op Code is read from sDI register instead of data bus, because the machine cycle 7 is memory read cycle and not op code fetch cycle
        // and the data are valid at different clock edge.
        if( sm_TState == 4 && sm_MCycle == 7 && mc_prefix == prefixCB )
        {
            m_iReg = sDI;                                   // reading opcode from data bus
            PC++;                                        // increase program counter PC
            m_iSet = prefixCB;                              // switch instruction set to prefix CB
            mc_MCycles = sTableMCycles[prefixCB][m_iReg];   // set number of machine cycles for instruction
        }
    }
    sNMI    = !m_nmiPin->getInpState(); // Sampling of bus signals NMI, INT and BUSREQ
    sInt    = !m_intPin->getInpState();
    sBusReq = !m_busreqPin->getInpState(); /// At Rising edge latst T State ??? - I guess it doesn't matter

    // RisingEdgeDelayed
    Simulator::self()->addEvent( m_delay, this );
}

void Z80Core::clkFallingEdge() // Sampling bus signal at clock falling edge
{
    sWait = !m_waitPin->getInpState(); // Sampling wait signal

    /// Done at fallingEdgeDelayed(), is OK???
    // Sampling data bus for Memory read and Input/Output read cycle
    /*if ( (sm_TState == 3) && ( (mc_busOp == oMemRead) || (mc_busOp == oIORead) ) && (sBusAck == false) ) {
        sDI = readDataBus();
    }*/

    // FallingEdgeDelayed
    Simulator::self()->addEvent( m_delay, this );
}

// Increasing TState, if it is last TState then TState is reset and MCycle is increased
// It also contains handling of bus request, halt, interrupt and maskable interrupt
void Z80Core::nextTState()
{
    if( sm_TState != 2 || sWait == false ) // Hold in TState 2 when Wait signal is actived
    {
        if( sm_lastTState ) // If the TState is the last state then it is necessary to increase machine cycle
        {
            if( mc_StateMachine == sHalt) // Set Machine cycle to halt machine cycle
            {
                sm_M1CycleType = tHalt;                             // machine cycle halt
                mc_StateMachine = sNone;                            // reset flag
            }
            if( sBusReq ) sBusAck = true; // The bus is release when it is required by signal BusReq
            else {
                sm_TState = 1;                                      // reset TState to first state

                if( mc_StateMachine == sXYOffsetFetch ) // set machine cycle to IX/IY Address offset fetch cycle (Machine cycle number 6)
                {
                    sm_PreXYMCycle = sm_MCycle;                     // store actual machine cycle to be able to return (cycles are inserted)
                    sm_MCycle = 6;                                  // address offset fetch cycle is cycle number 6
                    mc_StateMachine = sNone;                        // reset flag
                }
                else if( sm_MCycle == 7 || mc_StateMachine == sXYFetchFinish )   // last IX/IY fetch cycle is 7, but it can be finish earlier by flag
                {
                    sm_MCycle = sm_PreXYMCycle + 1;             // restore machine cycle to cycle before IX/IY Address offset fetch cycle plus one
                    mc_StateMachine = sNone;                    // reset flag
                }
                else if( sm_MCycle == mc_MCycles )              // Last Machine cycle of instruction
                {
                    sm_MCycle = 1;                              // set machine cycle number 1
                    mc_busOp = oM1;                             // bus operaton Op Code Fetch
                    if( sm_M1CycleType != tHalt ) sm_M1CycleType = tOpCodeFetch; // machine cycle Op Code Fetch only when the previous Machine cycle is not halt machine cycle

                    if( specialReset ) {             // if the special reset is required then the next machine cycle is special reset cycle
                        sm_M1CycleType = tSpecialReset;
                        specialReset = false;               // reset special reset flag
                    }
                    if( sNMI && mc_prefix == noPrefix ) // Non maskable interrupt only when instruction is completed with all prefixes
                    {
                        sm_M1CycleType = tNMI;              // machine cycle nonmaskable interrupt, bus op. Op Code Fetch
                        IFF1 = false;                       // disable interrupt
                    }
                    else if( sInt && IFF1 && mc_prefix == noPrefix )// Interrupt only when instruction is completed with all prefixes and interrupt is enabled
                    {
                        mc_busOp = oM1Int;              // bus op. interrupt
                        sm_M1CycleType = tInt;          // machine cycle interrupt
                        sm_autoWait = 1;                // one wait state during INT M1 cycle TState 1
                        IFF1 = false;                   // disable interrupt
                        IFF2 = false;
                        sm_TStatesAfterInt = 0;         // reset TStates after interrupt counter
                    }
                }
                else sm_MCycle++;                            // increase machine cycle
            }
        } else {
            sm_waitTState = (sm_autoWait > 0);                      // increase TState when wait state is not required
            if( sm_waitTState == false )
            {
                sm_TState++;                                        // increase TState
                // I/O operations and interrupt require one T2 wait state extra (it is possible to disable it by variable m_ioWait)
                if( m_ioWait && sm_TState == 2 && (mc_busOp == oIORead || mc_busOp == oIOWrite || mc_busOp == oM1Int) )
                    sm_autoWait = 1;                                // set number of inserted wait states to 1
                if( sm_TState == 2 && !m_resetPin->getInpState() )  //  when the RESET signal is set at rising edge of TState 2 the special reset will be executed
                    specialReset = true;                            // set special reset flag
            }
            else sm_autoWait--;                                      // decrease number of inserted wait states
        }
        sm_lastTState = (sm_TState == mc_TStates);                  // set flag of the last TState
    }
    else sm_waitTState = true;                                      // when the TState = 2 and Wait is active it is a wait TState
}

// Read instruction to instruction register. The source of instruction depends on type of machine cycle 1. It can be read from data bus or instruction NOP or RST 38H.
// The instruction set is switched according to previous prefix instructions.
// The number of machine cycles and TStates for machine cycle 1 is set for the instruction.
// The program counter is increased in case of not handling interrupt or halt.
void Z80Core::opCodeFetch()
{
    switch( sm_M1CycleType ) // Fetching opcode
    {
        case tOpCodeFetch: m_iReg = m_dataPort->getPortState();   // reading opcode from data bus

            PC++;                                            // increase program counter PC

            switch( mc_prefix ){ // Handle prefixes
                case noPrefix: XYState = rHL; m_iSet = noPrefix; break; // no prefix - register pair HL and basic instruction set
                case prefixCB:                m_iSet = prefixCB; break; // prefix CB - no change register pair and instruction set CB
                case prefixED: XYState = rHL; m_iSet = prefixED; break; // prefix ED - register pair HL and instruction set ED
                case prefixIX: XYState = rIX; m_iSet = noPrefix; break; // prefix IX - register pair IX and basic instruction set
                case prefixIY: XYState = rIY; m_iSet = noPrefix; break; // prefix IY - register pair IY and basic instruction set
            }
            // Set number of machine cycles and TStates for instruction
            mc_MCycles = sTableMCycles[m_iSet][m_iReg];
            mc_TStates = sTableM1TStates[m_iSet][m_iReg];
            break;
                            // Fetching opcode for Interrupt (IM0, IM1 and IM2)
        case tInt:
            switch( intMode ){
                case 0:  m_iReg = m_dataPort->getPortState();; break; // from data bus
                case 1:  m_iReg = 0xFF; break;          // RST 38H
                case 2:  m_iReg = 0x00;                 // NOP for IM2 has 5 machine cycles and 5 TStates
                         mc_MCycles = 5;
                         mc_TStates = 5;
                         break;
                default: m_iReg = 0x00; break;
            }
            // Set number of machine cycles and TStates for instruction except IM2
            if (intMode != 2) {
                mc_MCycles = sTableMCycles[m_iSet][m_iReg];
                mc_TStates = sTableM1TStates[m_iSet][m_iReg];
            }
            break;
                            // Fetching opcode for NMI
        case tNMI: m_iReg = 0x00; mc_MCycles = 3; mc_TStates = 5; break; // NOP for NMI has 3 machine cycles and 5 TStates

                            // Fetching opcode for special reset - set program counter PC to zero and execute instruction NOP
        case tSpecialReset: PC = 0;                                  // set program counter PC to zero
                            m_iSet = noPrefix;                          // no prefix - register pair HL and basic instruction set

                            // Intentionally missing break after case Fetching opcode for Halt
        case tHalt:         m_iReg = 0x00;                              // NOP
                            mc_MCycles = sTableMCycles[m_iSet][m_iReg];
                            mc_TStates = sTableM1TStates[m_iSet][m_iReg];
    }
}

// execution
// It is called at rising edge of last TState of every machine cycle - the internal CPU timing is not exact
void Z80Core::runMCode()
{
    mc_TStates = 3;                                             // default number of TStates is 3
    sAO = PC;                                                // default value for address bus is program counter PC
    mc_prefix = noPrefix;                                       // default value for prefixes is no prefix
    lastRegFChanged = regFChanged;                              // store flag of changing register F by previous instruction - it is needed by instructions SCF and CCF
    regFChanged = false;                                        // register F is not changed before instruction execution

    if (m_iSet == noPrefix) // Execute instructions without prefix
        switch(m_iReg) {
                       // NOP is used also at interrupt machine cycles
                       // When INT is actived and interrupt mode is set to 2 the interrupt vector is read from data bus and corresponding function is called
            case 0x00: if( sm_M1CycleType == tInt && intMode == 2 ) int_im2();                       // NOP
                       if( sm_M1CycleType == tNMI )                 rst(0x66); // When NMI is actived the function at address 0x0066 is called
                       break;
            case 0x01: ld_rr_imm(regs[rB], regs[rC]); break;                                            // LD BC,NN
            case 0x02: ld_indir_r(regs[rB], regs[rC], regA); break;                                     // LD (BC),A
            case 0x03: inc_rr(regs[rB], regs[rC]); break;                                               // INC BC
            case 0x04: inc_r(regs[rB]); break;                                                          // INC B
            case 0X05: dec_r(regs[rB]); break;                                                          // DEC B
            case 0x06: ld_r_imm(regs[rB]); break;                                                       // LD B,N
            case 0x07: rlca(); break;                                                                   // RLCA
            case 0x08: ex_rr_rr(regA, regF, regAAlt, regFAlt); break;                                   // EX AF,AF'
            case 0x09: add_rr_rr(regs[XYState], regs[XYState + 1], regs[rB], regs[rC]); break;          // ADD HL,BC  ADD IX,BC  ADD IY,BC
            case 0x0A: ld_r_indir(regA, regs[rB], regs[rC]); break;                                     // LD A,(BC)
            case 0x0B: dec_rr(regs[rB], regs[rC]); break;                                               // DEC BC
            case 0x0C: inc_r(regs[rC]); break;                                                          // INC C
            case 0X0D: dec_r(regs[rC]); break;                                                          // DEC C
            case 0x0E: ld_r_imm(regs[rC]); break;                                                       // LD C,N
            case 0x0F: rrca(); break;                                                                   // RRCA
            case 0x10: djnz(regs[rB]); break;                                                           // DJNZ N
            case 0x11: ld_rr_imm(regs[rD], regs[rE]); break;                                            // LD DE,NN
            case 0x12: ld_indir_r(regs[rD], regs[rE], regA); break;                                     // LD (DE),A
            case 0x13: inc_rr(regs[rD], regs[rE]); break;                                               // INC DE
            case 0x14: inc_r(regs[rD]); break;                                                          // INC D
            case 0X15: dec_r(regs[rD]); break;                                                          // DEC D
            case 0x16: ld_r_imm(regs[rD]); break;                                                       // LD D,N
            case 0x17: rla(); break;                                                                    // RLA
            case 0x18: jr(true); break;                                                                 // JR N
            case 0x19: add_rr_rr(regs[XYState], regs[XYState + 1], regs[rD], regs[rE]); break;          // ADD HL,DE  ADD IX,DE  ADD IY,DE
            case 0x1A: ld_r_indir(regA, regs[rD], regs[rE]); break;                                     // LD A,(DE)
            case 0x1B: dec_rr(regs[rD], regs[rE]); break;                                               // DEC DE
            case 0x1C: inc_r(regs[rE]); break;                                                          // INC E
            case 0X1D: dec_r(regs[rE]); break;                                                          // DEC E
            case 0x1E: ld_r_imm(regs[rE]); break;                                                       // LD E,N
            case 0x1F: rra(); break;                                                                    // RRA
            case 0x20: jr((regF & fZ) == 0); break;                                                     // JR NZ,N
            case 0x21: ld_rr_imm(regs[XYState], regs[XYState + 1]); break;                              // LD HL,NN  LD IX,NN  LD IY,NN
            case 0x22: ld_mem_rr(regs[XYState], regs[XYState + 1]); break;                              // LD (NN),HL  LD (NN),IX  LD (NN),IY
            case 0x23: inc_rr(regs[XYState], regs[XYState + 1]); break;                                 // INC HL  INC IX  INC IY
            case 0x24: inc_r(regs[XYState]); break;                                                     // INC H  INC XH  INC YH
            case 0X25: dec_r(regs[XYState]); break;                                                     // DEC H  DEC XH  DEC YH
            case 0x26: ld_r_imm(regs[XYState]); break;                                                  // LD H,N  LD XH,N  LD YH,N
            case 0x27: daa(); break;                                                                    // DAA
            case 0x28: jr((regF & fZ) == fZ); break;                                                    // JR Z,N
            case 0x29: add_rr_rr(regs[XYState], regs[XYState+1], regs[XYState], regs[XYState+1]); break; // ADD HL,HL  ADD IX,IX  ADD IY,IY
            case 0x2A: ld_rr_mem(regs[XYState], regs[XYState + 1]); break;                              // LD HL,(NN)  LD IX,(NN)  LD IY,(NN)
            case 0x2B: dec_rr(regs[XYState], regs[XYState + 1]); break;                                 // DEC HL  DEC IX  DEC IY
            case 0x2C: inc_r(regs[XYState + 1]); break;                                                 // INC L  INC XL  INC YL
            case 0X2D: dec_r(regs[XYState + 1]); break;                                                 // DEC L  DEC XL  DEC YL
            case 0x2E: ld_r_imm(regs[XYState + 1]); break;                                              // LD L,N  LD XL,N  LD YL,N
            case 0x2F: cpl(); break;                                                                    // CPL
            case 0x30: jr((regF & fC) == 0); break;                                                     // JR NC,N
            case 0x31: ld_SP_imm(); break;                                                              // LD SP,NN
            case 0x32: ld_mem_r(regA); break;                                                           // LD (NN),A  
            case 0x33: regSP++; break;                                                                  // INC SP
            case 0x34: inc_indirXY(); break;                                                            // INC (HL)  INC (IX)  INC (IY)
            case 0x35: dec_indirXY(); break;                                                            // DEC (HL)  DEC (IX)  DEC (IY)
            case 0x36: ld_indirXY_imm(); break;                                                         // LD (HL),N  LD (IX+d),N  LD (IY+d),N
            case 0x37: scf(); break;                                                                    // SCF
            case 0x38: jr((regF & fC) == fC); break;                                                    // JR C,N
            case 0x39: add_rr_rr(regs[XYState], regs[XYState + 1], regSP >> 8, regSP & 0x00ff); break;  // ADD HL,SP  ADD IX,SP  ADD IY,SP
            case 0x3A: ld_r_mem(regA); break;                                                           // LD A,(NN)
            case 0x3B: regSP--; break;                                                                  // DEC SP
            case 0x3C: inc_r(regA); break;                                                              // INC A
            case 0X3D: dec_r(regA); break;                                                              // DEC A
            case 0x3E: ld_r_imm(regA); break;                                                           // LD A,N
            case 0x3F: ccf(); break;                                                                    // CCF
            case 0x40: break;                                                                           // LD B,B
            case 0x41: regs[rB] = regs[rC]; break;                                                      // LD B,C
            case 0x42: regs[rB] = regs[rD]; break;                                                      // LD B,D
            case 0x43: regs[rB] = regs[rE]; break;                                                      // LD B,E
            case 0x44: regs[rB] = regs[XYState]; break;                                                 // LD B,H  LD B,XH  LD B,YH
            case 0x45: regs[rB] = regs[XYState + 1]; break;                                             // LD B,L  LD B,XL  LD B,YL
            case 0x46: ld_r_indirXY(regs[rB]); break;                                                   // LD B,(HL)  LD B,(IX+d)  LD B,(IY+d)
            case 0x47: regs[rB] = regA; break;                                                          // LD B,A
            case 0x48: regs[rC] = regs[rB]; break;                                                      // LD C,B
            case 0x49: break;                                                                           // LD C,C
            case 0x4A: regs[rC] = regs[rD]; break;                                                      // LD C,D
            case 0x4B: regs[rC] = regs[rE]; break;                                                      // LD C,E
            case 0x4C: regs[rC] = regs[XYState]; break;                                                 // LD C,H  LD C,XH  LD C,YH
            case 0x4D: regs[rC] = regs[XYState + 1]; break;                                             // LD C,L  LD C,XL  LD C,YL
            case 0x4E: ld_r_indirXY(regs[rC]); break;                                                   // LD C,(HL)  LD C,(IX+d)  LD C,(IY+d)
            case 0x4F: regs[rC] = regA; break;                                                          // LD C,A
            case 0x50: regs[rD] = regs[rB]; break;                                                      // LD D,B
            case 0x51: regs[rD] = regs[rC]; break;                                                      // LD D,C
            case 0x52: break;                                                                           // LD D,D
            case 0x53: regs[rD] = regs[rE]; break;                                                      // LD D,E
            case 0x54: regs[rD] = regs[XYState]; break;                                                 // LD D,H  LD D,XH  LD D,YH
            case 0x55: regs[rD] = regs[XYState + 1]; break;                                             // LD D,L  LD D,XL  LD D,YL
            case 0x56: ld_r_indirXY(regs[rD]); break;                                                   // LD D,(HL)  LD D,(IX+d)  LD D,(IY+d)
            case 0x57: regs[rD] = regA; break;                                                          // LD D,A
            case 0x58: regs[rE] = regs[rB]; break;                                                      // LD E,B
            case 0x59: regs[rE] = regs[rC]; break;                                                      // LD E,C
            case 0x5A: regs[rE] = regs[rD]; break;                                                      // LD E,D
            case 0x5B: break;                                                                           // LD E,E
            case 0x5C: regs[rE] = regs[XYState]; break;                                                 // LD E,H  LD E,XH  LD E,YH
            case 0x5D: regs[rE] = regs[XYState + 1]; break;                                             // LD E,L  LD E,XL  LD E,YL
            case 0x5E: ld_r_indirXY(regs[rE]); break;                                                   // LD E,(HL)  LD E,(IX+d)  LD E,(IY+d)
            case 0x5F: regs[rE] = regA; break;                                                          // LD E,A
            case 0x60: regs[XYState] = regs[rB]; break;                                                 // LD H,B  LD XH,B  LD YH,B
            case 0x61: regs[XYState] = regs[rC]; break;                                                 // LD H,C  LD XH,C  LD YH,C
            case 0x62: regs[XYState] = regs[rD]; break;                                                 // LD H,D  LD XH,D  LD YH,D
            case 0x63: regs[XYState] = regs[rE]; break;                                                 // LD H,E  LD XH,E  LD YH,E
            case 0x64: break;                                                                           // LD H,H  LD XH,XH  LD XH,YH
            case 0x65: regs[XYState] = regs[XYState + 1]; break;                                        // LD H,L  LD XH,XL  LD XH,YL
            case 0x66: ld_r_indirXY(regs[rH]); break;                                                   // LD H,(HL)  LD H,(IX+d)  LD H,(IY+d)
            case 0x67: regs[XYState] = regA; break;                                                     // LD H,A  LD XH,A  LD YH,A
            case 0x68: regs[XYState + 1] = regs[rB]; break;                                             // LD L,B  LD XL,B  LD YL,B
            case 0x69: regs[XYState + 1] = regs[rC]; break;                                             // LD L,C  LD XL,B  LD YL,B
            case 0x6A: regs[XYState + 1] = regs[rD]; break;                                             // LD L,D  LD XL,B  LD YL,B
            case 0x6B: regs[XYState + 1] = regs[rE]; break;                                             // LD L,E  LD XL,B  LD YL,B
            case 0x6C: regs[XYState + 1] = regs[XYState]; break;                                        // LD L,H  LD XL,XH  LD YL,YH
            case 0x6D: break;                                                                           // LD L,L  LD XL,XL  LD YL,YL
            case 0x6E: ld_r_indirXY(regs[rL]); break;                                                   // LD L,(HL)  LD L,(IX+d)  LD L,(IY+d)
            case 0x6F: regs[XYState + 1] = regA; break;                                                 // LD L,A  LD XL,A  LD YL,A
            case 0x70: ld_indirXY_r(regs[rB]); break;                                                   // LD (HL),B  LD (IX+d),B  LD (IY+d),B
            case 0x71: ld_indirXY_r(regs[rC]); break;                                                   // LD (HL),C  LD (IX+d),C  LD (IY+d),C
            case 0x72: ld_indirXY_r(regs[rD]); break;                                                   // LD (HL),D  LD (IX+d),D  LD (IY+d),D
            case 0x73: ld_indirXY_r(regs[rE]); break;                                                   // LD (HL),E  LD (IX+d),E  LD (IY+d),E
            case 0x74: ld_indirXY_r(regs[rH]); break;                                                   // LD (HL),H  LD (IX+d),H  LD (IY+d),H
            case 0x75: ld_indirXY_r(regs[rL]); break;                                                   // LD (HL),L  LD (IX+d),L  LD (IY+d),L
            case 0x76: mc_StateMachine = sHalt; break;                                                  // HALT
            case 0x77: ld_indirXY_r(regA); break;                                                       // LD (HL),A  LD (IX+d),A  LD (IY+d),A
            case 0x78: regA = regs[rB]; break;                                                          // LD A,B
            case 0x79: regA = regs[rC]; break;                                                          // LD A,C
            case 0x7A: regA = regs[rD]; break;                                                          // LD A,D
            case 0x7B: regA = regs[rE]; break;                                                          // LD A,E
            case 0x7C: regA = regs[XYState]; break;                                                     // LD A,H  LD A,XH  LD A,YH
            case 0x7D: regA = regs[XYState + 1]; break;                                                 // LD A,L  LD A,XL  LD A,YL
            case 0x7E: ld_r_indirXY(regA); break;                                                       // LD A,(HL)  LD A,(IX+d)  LD A,(IY+d)
            case 0x7F: break;                                                                           // LD A,A
            case 0x80: add_r(regs[rB]); break;                                                          // ADD A,B
            case 0x81: add_r(regs[rC]); break;                                                          // ADD A,C
            case 0x82: add_r(regs[rD]); break;                                                          // ADD A,D
            case 0x83: add_r(regs[rE]); break;                                                          // ADD A,E
            case 0x84: add_r(regs[XYState]); break;                                                     // ADD A,H  ADD A,XH  ADD A,YH
            case 0x85: add_r(regs[XYState + 1]); break;                                                 // ADD A,L  ADD A,XL  ADD A,YL
            case 0x86: inst_indirXY<iAdd>(); break;                                                     // ADD A,(HL)  ADD A,(IX+d)  ADD A,(IY+d)
            case 0x87: add_r(regA); break;                                                              // ADD A,A
            case 0x88: adc_r(regs[rB]); break;                                                          // ADC A,B
            case 0x89: adc_r(regs[rC]); break;                                                          // ADC A,C
            case 0x8A: adc_r(regs[rD]); break;                                                          // ADC A,D
            case 0x8B: adc_r(regs[rE]); break;                                                          // ADC A,E
            case 0x8C: adc_r(regs[XYState]); break;                                                     // ADC A,H  ADC A,XH  ADC A,YH
            case 0x8D: adc_r(regs[XYState + 1]); break;                                                 // ADC A,L  ADC A,XL  ADC A,YL
            case 0x8E: inst_indirXY<iAdc>(); break;                                                     // ADC A,(HL)  ADC A,(IX+d)  ADC A,(IY+d)
            case 0x8F: adc_r(regA); break;                                                              // ADC A,A
            case 0x90: sub_r(regs[rB]); break;                                                          // SUB B
            case 0x91: sub_r(regs[rC]); break;                                                          // SUB C
            case 0x92: sub_r(regs[rD]); break;                                                          // SUB D
            case 0x93: sub_r(regs[rE]); break;                                                          // SUB E
            case 0x94: sub_r(regs[XYState]); break;                                                     // SUB H  SUB XH  SUB YH
            case 0x95: sub_r(regs[XYState + 1]); break;                                                 // SUB L  SUB XL  SUB YL
            case 0x96: inst_indirXY<iSub>(); break;                                                     // SUB (HL)  SUB (IX+d)  SUB (IY+d)
            case 0x97: sub_r(regA); break;                                                              // SUB A
            case 0x98: sbc_r(regs[rB]); break;                                                          // SBC A,B
            case 0x99: sbc_r(regs[rC]); break;                                                          // SBC A,C
            case 0x9A: sbc_r(regs[rD]); break;                                                          // SBC A,D
            case 0x9B: sbc_r(regs[rE]); break;                                                          // SBC A,E
            case 0x9C: sbc_r(regs[XYState]); break;                                                     // SBC A,H  SBC A,XH  SBC A,YH
            case 0x9D: sbc_r(regs[XYState + 1]); break;                                                 // SBC A,L  SBC A,XL  SBC A,YL
            case 0x9E: inst_indirXY<iSbc>(); break;                                                     // SBC A,(HL)  SBC A,(IX+d)  SBC A,(IY+d)
            case 0x9F: sbc_r(regA); break;                                                              // SBC A,A
            case 0xA0: and_r(regs[rB]); break;                                                          // AND B
            case 0xA1: and_r(regs[rC]); break;                                                          // AND C
            case 0xA2: and_r(regs[rD]); break;                                                          // AND D
            case 0xA3: and_r(regs[rE]); break;                                                          // AND E
            case 0xA4: and_r(regs[XYState]); break;                                                     // AND H  AND XH  AND YL
            case 0xA5: and_r(regs[XYState + 1]); break;                                                 // AND L  AND XL  AND YL
            case 0xA6: inst_indirXY<iAnd>(); break;                                                     // AND (HL)  AND (IX+d)  AND (IY+d)
            case 0xA7: and_r(regA); break;                                                              // AND A
            case 0xA8: xor_r(regs[rB]); break;                                                          // XOR B
            case 0xA9: xor_r(regs[rC]); break;                                                          // XOR C
            case 0xAA: xor_r(regs[rD]); break;                                                          // XOR D
            case 0xAB: xor_r(regs[rE]); break;                                                          // XOR E
            case 0xAC: xor_r(regs[XYState]); break;                                                     // XOR H  XOR XH  XOR YH
            case 0xAD: xor_r(regs[XYState + 1]); break;                                                 // XOR L  XOR XL  XOR YL
            case 0xAE: inst_indirXY<iXor>(); break;                                                     // XOR (HL)  XOR (IX+d)  XOR (IY+d)
            case 0xAF: xor_r(regA); break;                                                              // XOR A
            case 0xB0: or_r(regs[rB]); break;                                                           // OR B
            case 0xB1: or_r(regs[rC]); break;                                                           // OR C
            case 0xB2: or_r(regs[rD]); break;                                                           // OR D
            case 0xB3: or_r(regs[rE]); break;                                                           // OR E
            case 0xB4: or_r(regs[XYState]); break;                                                      // OR H  OR XH  OR YL
            case 0xB5: or_r(regs[XYState + 1]); break;                                                  // OR L  OR XL  OR YL
            case 0xB6: inst_indirXY<iOr>(); break;                                                      // OR (HL)  OR (IX+d)  OR (IY+d)
            case 0xB7: or_r(regA); break;                                                               // OR A
            case 0xB8: cp_r(regs[rB]); break;                                                           // CP B
            case 0xB9: cp_r(regs[rC]); break;                                                           // CP C
            case 0xBA: cp_r(regs[rD]); break;                                                           // CP D
            case 0xBB: cp_r(regs[rE]); break;                                                           // CP E
            case 0xBC: cp_r(regs[XYState]); break;                                                      // CP H  CP XH  CP YH
            case 0xBD: cp_r(regs[XYState + 1]); break;                                                  // CP L  CP XL  CP YL
            case 0xBE: inst_indirXY<iCp>(); break;                                                      // CP (HL)  CP (IX+d)  CP (IY+d)
            case 0xBF: cp_r(regA); break;                                                               // CP A
            case 0xC0: ret((regF & fZ) == 0); break;                                                    // RET NZ
            case 0xC1: pop_rr(regs[rB], regs[rC]); break;                                               // POP BC
            case 0xC2: jp((regF & fZ) == 0); break;                                                     // JP NZ,NN
            case 0xC3: jp(true); break;                                                                 // JP NN
            case 0xC4: call((regF & fZ) == 0); break;                                                   // CALL NZ,NN
            case 0xC5: push_rr(regs[rB], regs[rC]); break;                                              // PUSH BC
            case 0xC6: inst_imm<iAdd>(); break;                                                         // ADD A,N
            case 0xC7: rst(0x00); break;                                                                // RST 00H
            case 0xC8: ret((regF & fZ) == fZ); break;                                                   // RET Z
            case 0xC9: ret(true); break;                                                                // RET
            case 0xCA: jp((regF & fZ) == fZ); break;                                                    // JP Z,NN
            case 0xCB: prefix_cb(); break;                                                              // PREFIX CB
            case 0xCC: call((regF & fZ) == fZ); break;                                                  // CALL Z,NN
            case 0xCD: call(true); break;                                                               // CALL NN
            case 0xCE: inst_imm<iAdc>(); break;                                                         // ADC A,N
            case 0xCF: rst(0x08); break;                                                                // RST 08H
            case 0xD0: ret((regF & fC) == 0); break;                                                    // RET NC
            case 0xD1: pop_rr(regs[rD], regs[rE]); break;                                               // POP DE
            case 0xD2: jp((regF & fC) == 0); break;                                                     // JP NC,NN
            case 0xD3: out_imm_r(regA); break;                                                          // OUT (N),A
            case 0xD4: call((regF & fC) == 0); break;                                                   // CALL NC,NN
            case 0xD5: push_rr(regs[rD], regs[rE]); break;                                              // PUSH DE
            case 0xD6: inst_imm<iSub>(); break;                                                         // SUB N
            case 0xD7: rst(0x10); break;                                                                // RST 10H
            case 0xD8: ret((regF & fC) == fC); break;                                                   // RET C
            case 0xD9: ex_rr_rr(regs[rB], regs[rC], regsAlt[rB], regsAlt[rC]);                          // EXX
                       ex_rr_rr(regs[rD], regs[rE], regsAlt[rD], regsAlt[rE]);
                       ex_rr_rr(regs[rH], regs[rL], regsAlt[rH], regsAlt[rL]);
                       break;
            case 0xDA: jp((regF & fC) == fC); break;                                           // JP C,NN
            case 0xDB: in_r_imm(regA); break;                                                  // IN A,(N)
            case 0xDC: call((regF & fC) == fC); break;                                         // CALL C,NN
            case 0xDD: mc_prefix = prefixIX; break;                                            // PREFIX IX
            case 0xDE: inst_imm<iSbc>(); break;                                                // SBC A,N
            case 0xDF: rst(0x18); break;                                                       // RST 18H
            case 0xE0: ret((regF & fP) == 0); break;                                           // RET PO
            case 0xE1: pop_rr(regs[XYState], regs[XYState + 1]); break;                        // POP HL  POP IX  POP IY
            case 0xE2: jp((regF & fP) == 0); break;                                            // JP PO,NN
            case 0xE3: ex_SP_rr(regs[XYState], regs[XYState + 1]); break;                      // EX (SP),HL  EX (SP),IX  EX (SP),IY
            case 0xE4: call((regF & fP) == 0); break;                                          // CALL PO,NN
            case 0xE5: push_rr(regs[XYState], regs[XYState + 1]); break;                       // PUSH HL  PUSH IX  PUSH IY
            case 0xE6: inst_imm<iAnd>(); break;                                                // AND N
            case 0xE7: rst(0x20); break;                                                       // RST 20H
            case 0xE8: ret((regF & fP) == fP); break;                                          // RET PE
            case 0xE9: PC = (regs[XYState] << 8) | regs[XYState + 1];                       // JP (HL)  JP (IX)  JP (IY)
                       sAO = PC;
                       break;
            case 0xEA: jp((regF & fP) == fP); break;                                           // JP PE,NN
            case 0xEB: ex_rr_rr(regs[rD], regs[rE], regs[rH], regs[rL]); break;                // EX DE,HL
            case 0xEC: call((regF & fP) == fP); break;                                         // CALL PE,NN
            case 0xED: mc_prefix = prefixED; break;                                            // PREFIX ED
            case 0xEE: inst_imm<iXor>(); break;                                                // XOR N
            case 0xEF: rst(0x28); break;                                                       // RST 28H
            case 0xF0: ret((regF & fS) == 0); break;                                           // RET P
            case 0xF1: pop_rr(regA, regF); break;                                              // POP AF
            case 0xF2: jp((regF & fS) == 0); break;                                            // JP P,NN
            case 0xF3: IFF1 = false; IFF2 = false; break;                                      // DI
            case 0xF4: call((regF & fS) == 0); break;                                          // CALL P,NN
            case 0xF5: push_rr(regA, regF); break;                                             // PUSH AF
            case 0xF6: inst_imm<iOr>(); break;                                                 // OR N
            case 0xF7: rst(0x30); break;                                                       // RST 30H
            case 0xF8: ret((regF & fS) == fS); break;                                          // RET M
            case 0xF9: regSP = (regs[XYState] << 8) | regs[XYState + 1]; break;                // LD SP,HL  LD SP,IX  LP SP,IY
            case 0xFA: jp((regF & fS) == fS); break;                                           // JP M,NN
            case 0xFB: IFF1 = true; IFF2 = true; sInt = false; break;                          // EI
            case 0xFC: call((regF & fS) == fS); break;                                         // CALL M,NN
            case 0xFD: mc_prefix = prefixIY; break;                                            // PREFIX IY
            case 0xFE: inst_imm<iCp>(); break;                                                 // CP N
            case 0xFF: rst(0x38); break;                                                       // RST 38H
        }

#define Z80Core_INSTRUCTION(op, instruction) \
            case (op+0): instCB<instruction>(regs[rB]); break;\
            case (op+1): instCB<instruction>(regs[rC]); break;\
            case (op+2): instCB<instruction>(regs[rD]); break;\
            case (op+3): instCB<instruction>(regs[rE]); break;\
            case (op+4): instCB<instruction>(regs[rH]); break;\
            case (op+5): instCB<instruction>(regs[rL]); break;\
            case (op+6): { uint8_t none; instCB<instruction>(none); } break;\
            case (op+7): instCB<instruction>(regA); break;

#define Z80Core_INSTRUCTION_MASK(op, instruction, mask) \
            case (op+0): instCB<instruction, mask>(regs[rB]); break;\
            case (op+1): instCB<instruction, mask>(regs[rC]); break;\
            case (op+2): instCB<instruction, mask>(regs[rD]); break;\
            case (op+3): instCB<instruction, mask>(regs[rE]); break;\
            case (op+4): instCB<instruction, mask>(regs[rH]); break;\
            case (op+5): instCB<instruction, mask>(regs[rL]); break;\
            case (op+6): { uint8_t none; instCB<instruction, mask>(none); } break;\
            case (op+7): instCB<instruction, mask>(regA); break;

    // Execute instructions with prefix CB
    if (m_iSet == prefixCB)
        switch(m_iReg) {
            Z80Core_INSTRUCTION(0x00, iRlc)                                                   // RLC r  RLC (HL)  LD r,RLC(IX+d)  LD r,RLC(IY+d)
            Z80Core_INSTRUCTION(0x08, iRrc)                                                   // RRC r  RRC (HL)  LD r,RRC(IX+d)  LD r,RRC(IY+d)
            Z80Core_INSTRUCTION(0x10, iRl)                                                    // RL r  RL (HL)  LD r,RL(IX+d)  LD r,RL(IY+d)
            Z80Core_INSTRUCTION(0x18, iRr)                                                    // RR r  RR (HL)  LD r,RR(IX+d)  LD r,RR(IY+d)
            Z80Core_INSTRUCTION(0x20, iSla)                                                   // SLA r  SLA (HL)  LD r,SLA(IX+d)  LD r,SLA(IY+d)
            Z80Core_INSTRUCTION(0x28, iSra)                                                   // SRA r  SRA (HL)  LD r,SRA(IX+d)  LD r,SRA(IY+d)
            Z80Core_INSTRUCTION(0x30, iSll)                                                   // SLL r  SLL (HL)  LD r,SLL(IX+d)  LD r,SLL(IY+d)
            Z80Core_INSTRUCTION(0x38, iSrl)                                                   // SRLL r  SRL (HL)  LD r,SRL(IX+d)  LD r,SRL(IY+d)
            Z80Core_INSTRUCTION_MASK(0x40, iBit, 0x01)                                        // BIT 0,r  BIT 0,(HL)  BIT 0,(IX+d)  BIT 0,(IY+d)
            Z80Core_INSTRUCTION_MASK(0x48, iBit, 0x02)                                        // BIT 1,r  BIT 1,(HL)  BIT 1,(IX+d)  BIT 1,(IY+d)
            Z80Core_INSTRUCTION_MASK(0x50, iBit, 0x04)                                        // BIT 2,r  BIT 2,(HL)  BIT 2,(IX+d)  BIT 2,(IY+d)
            Z80Core_INSTRUCTION_MASK(0x58, iBit, 0x08)                                        // BIT 3,r  BIT 3,(HL)  BIT 3,(IX+d)  BIT 3,(IY+d)
            Z80Core_INSTRUCTION_MASK(0x60, iBit, 0x10)                                        // BIT 4,r  BIT 4,(HL)  BIT 4,(IX+d)  BIT 4,(IY+d)
            Z80Core_INSTRUCTION_MASK(0x68, iBit, 0x20)                                        // BIT 5,r  BIT 5,(HL)  BIT 5,(IX+d)  BIT 5,(IY+d)
            Z80Core_INSTRUCTION_MASK(0x70, iBit, 0x40)                                        // BIT 6,r  BIT 6,(HL)  BIT 6,(IX+d)  BIT 6,(IY+d)
            Z80Core_INSTRUCTION_MASK(0x78, iBit, 0x80)                                        // BIT 7,r  BIT 7,(HL)  BIT 7,(IX+d)  BIT 7,(IY+d)
            Z80Core_INSTRUCTION_MASK(0x80, iRes, 0xfe)                                        // RES 0,r  RES 0,(HL)  RES 0,(IX+d)  RES 0,(IY+d)
            Z80Core_INSTRUCTION_MASK(0x88, iRes, 0xfd)                                        // RES 1,r  RES 1,(HL)  RES 1,(IX+d)  RES 1,(IY+d)
            Z80Core_INSTRUCTION_MASK(0x90, iRes, 0xfb)                                        // RES 2,r  RES 2,(HL)  RES 2,(IX+d)  RES 2,(IY+d)
            Z80Core_INSTRUCTION_MASK(0x98, iRes, 0xf7)                                        // RES 3,r  RES 3,(HL)  RES 3,(IX+d)  RES 3,(IY+d)
            Z80Core_INSTRUCTION_MASK(0xA0, iRes, 0xef)                                        // RES 4,r  RES 4,(HL)  RES 4,(IX+d)  RES 4,(IY+d)
            Z80Core_INSTRUCTION_MASK(0xA8, iRes, 0xdf)                                        // RES 5,r  RES 5,(HL)  RES 5,(IX+d)  RES 5,(IY+d)
            Z80Core_INSTRUCTION_MASK(0xB0, iRes, 0xbf)                                        // RES 6,r  RES 6,(HL)  RES 6,(IX+d)  RES 6,(IY+d)
            Z80Core_INSTRUCTION_MASK(0xB8, iRes, 0x7f)                                        // RES 7,r  RES 7,(HL)  RES 7,(IX+d)  RES 7,(IY+d)
            Z80Core_INSTRUCTION_MASK(0xC0, iSet, 0x01)                                        // SET 0,r  SET 0,(HL)  SET 0,(IX+d)  SET 0,(IY+d)
            Z80Core_INSTRUCTION_MASK(0xC8, iSet, 0x02)                                        // SET 1,r  SET 1,(HL)  SET 1,(IX+d)  SET 1,(IY+d)
            Z80Core_INSTRUCTION_MASK(0xD0, iSet, 0x04)                                        // SET 2,r  SET 2,(HL)  SET 2,(IX+d)  SET 2,(IY+d)
            Z80Core_INSTRUCTION_MASK(0xD8, iSet, 0x08)                                        // SET 3,r  SET 3,(HL)  SET 3,(IX+d)  SET 3,(IY+d)
            Z80Core_INSTRUCTION_MASK(0xE0, iSet, 0x10)                                        // SET 4,r  SET 4,(HL)  SET 4,(IX+d)  SET 4,(IY+d)
            Z80Core_INSTRUCTION_MASK(0xE8, iSet, 0x20)                                        // SET 5,r  SET 5,(HL)  SET 5,(IX+d)  SET 5,(IY+d)
            Z80Core_INSTRUCTION_MASK(0xF0, iSet, 0x40)                                        // SET 6,r  SET 6,(HL)  SET 6,(IX+d)  SET 6,(IY+d)
            Z80Core_INSTRUCTION_MASK(0xF8, iSet, 0x80)                                        // SET 7,r  SET 7,(HL)  SET 7,(IX+d)  SET 7,(IY+d)
        }
        
    // Execute instructions with prefix ED
    if (m_iSet == prefixED)
        switch(m_iReg) {
            case 0x40: in_r_rr(regs[rB], regs[rB], regs[rC]); break;                          // IN B,(C)
            case 0x41: out_rr_r(regs[rB], regs[rC], regs[rB]); break;                         // OUT (C),B
            case 0x42: sbc_rr_rr(regs[rH], regs[rL], regs[rB], regs[rC]); break;              // SBC HL,BC
            case 0x43: ld_mem_rr(regs[rB], regs[rC]); break;                                  // LD (NN),BC
            case 0x44: neg(); break;                                                          // NEG
            case 0x45: retn(); break;                                                         // RETN
            case 0x46: intMode = 0; break;                                                    // IM 0
            case 0x47: regI = regA; break;                                                    // LD I,A
            case 0x48: in_r_rr(regs[rC], regs[rB], regs[rC]); break;                          // IN C,(C)
            case 0x49: out_rr_r(regs[rB], regs[rC], regs[rC]); break;                         // OUT (C),C
            case 0x4A: adc_rr_rr(regs[rH], regs[rL], regs[rB], regs[rC]); break;              // ADC HL,BC
            case 0x4B: ld_rr_mem(regs[rB], regs[rC]); break;                                  // LD BC,(NN)
            case 0x4C: neg(); break;                                                          // NEG
            case 0x4D: retn(); break;                                                         // RETI
            case 0x4E: intMode = 0; break;                                                    // IM 0/1
            case 0x4F: regR = regA; break;                                                    // LD R,A
            case 0x50: in_r_rr(regs[rD], regs[rB], regs[rC]); break;                          // IN D,(C)
            case 0x51: out_rr_r(regs[rB], regs[rC], regs[rD]); break;                         // OUT (C),D
            case 0x52: sbc_rr_rr(regs[rH], regs[rL], regs[rD], regs[rE]); break;              // SBC HL,DE
            case 0x53: ld_mem_rr(regs[rD], regs[rE]); break;                                  // LD (NN),DE
            case 0x54: neg(); break;                                                          // NEG
            case 0x55: retn(); break;                                                         // RETN
            case 0x56: intMode = 1; break;                                                    // IM 1
            case 0x57: regA = regI; flags_IR(); break;                                        // LD A,I
            case 0x58: in_r_rr(regs[rE], regs[rB], regs[rC]); break;                          // IN E,(C)
            case 0x59: out_rr_r(regs[rB], regs[rC], regs[rE]); break;                         // OUT (C),E
            case 0x5A: adc_rr_rr(regs[rH], regs[rL], regs[rD], regs[rE]); break;              // ADC HL,DE
            case 0x5B: ld_rr_mem(regs[rD], regs[rE]); break;                                  // LD DE,(NN)
            case 0x5C: neg(); break;                                                          // NEG
            case 0x5D: retn(); break;                                                         // RETN
            case 0x5E: intMode = 2; break;                                                    // IM 2
            case 0x5F: regA = regR; flags_IR(); break;                                        // LD A,R
            case 0x60: in_r_rr(regs[rH], regs[rB], regs[rC]); break;                          // IN H,(C)
            case 0x61: out_rr_r(regs[rB], regs[rC], regs[rH]); break;                         // OUT (C),H
            case 0x62: sbc_rr_rr(regs[rH], regs[rL], regs[rH], regs[rL]); break;              // SBC HL,HL
            case 0x63: ld_mem_rr(regs[rH], regs[rL]); break;                                  // LD (NN),HL
            case 0x64: neg(); break;                                                          // NEG
            case 0x65: retn(); break;                                                         // RETN
            case 0x66: intMode = 0; break;                                                    // IM 0
            case 0x67: rrd(); break;                                                          // RRD
            case 0x68: in_r_rr(regs[rL], regs[rB], regs[rC]); break;                          // IN L,(C)
            case 0x69: out_rr_r(regs[rB], regs[rC], regs[rL]); break;                         // OUT (C),L
            case 0x6A: adc_rr_rr(regs[rH], regs[rL], regs[rH], regs[rL]); break;              // ADC HL,HL
            case 0x6B: ld_rr_mem(regs[rH], regs[rL]); break;                                  // LD HL,(NN)
            case 0x6C: neg(); break;                                                          // NEG
            case 0x6D: retn(); break;                                                         // RETN
            case 0x6E: intMode = 0; break;                                                    // IM 0/1
            case 0x6F: rld(); break;                                                          // RLD
            case 0x70: { uint8_t none; in_r_rr(none, regs[rB], regs[rC]); } break;            // IN (C)
            case 0x71: out_rr_r(regs[rB], regs[rC], (m_cmos == false) ? 0x00 : 0xff); break;  // OUT (C),0  OUT(C),FF
            case 0x72: sbc_rr_rr(regs[rH], regs[rL], regSP >> 8, regSP & 0x00ff); break;      // SBC HL,SP
            case 0x73: ld_mem_rr(regSP >> 8, regSP & 0x00ff); break;                          // LD (NN),SP
            case 0x74: neg(); break;                                                          // NEG
            case 0x75: retn(); break;                                                         // RETN
            case 0x76: intMode = 1; break;                                                    // IM 1
                                                                                              // 0x77 undocumented
            case 0x78: in_r_rr(regA, regs[rB], regs[rC]); break;                              // IN C,(C)
            case 0x79: out_rr_r(regs[rB], regs[rC], regA); break;                             // OUT (C),C
            case 0x7A: adc_rr_rr(regs[rH], regs[rL], regSP >> 8, regSP & 0x00ff); break;      // ADC HL,SP
            case 0x7B: ld_SP_mem(); break;                                                    // LD SP,(NN)
            case 0x7C: neg(); break;                                                          // NEG
            case 0x7D: retn(); break;                                                         // RETN
            case 0x7E: intMode = 2; break;                                                    // IM 2
                                                                                              // 0x7F undocumented
            case 0xA0: ldxx<bInc>(); break;                                                   // LDI
            case 0xA1: cpxx<bInc>(); break;                                                   // CPI
            case 0xA2: inxx<bInc>(); break;                                                   // INI
            case 0xA3: otxx<bInc>(); break;                                                   // OUTI

            case 0xA8: ldxx<bDec>(); break;                                                   // LDD
            case 0xA9: cpxx<bDec>(); break;                                                   // CPD
            case 0xAA: inxx<bDec>(); break;                                                   // IND
            case 0xAB: otxx<bDec>(); break;                                                   // OUTD

            case 0xB0: ldxx<bIncRep>(); break;                                                // LDIR
            case 0xB1: cpxx<bIncRep>(); break;                                                // CPIR
            case 0xB2: inxx<bIncRep>(); break;                                                // INIR
            case 0xB3: otxx<bIncRep>(); break;                                                // OTIR

            case 0xB8: ldxx<bDecRep>(); break;                                                // LDDR
            case 0xB9: cpxx<bDecRep>(); break;                                                // CPDR
            case 0xBA: inxx<bDecRep>(); break;                                                // INDR
            case 0xBB: otxx<bDecRep>(); break;                                                // OTDR
        }
    // If next machine cycle is machine cycle 1, set TStates for next machine cycle - default value for first machine cycle is 4 TStates
    if( sm_MCycle == mc_MCycles ) mc_TStates = 4;
}

bool Z80Core::parity( uint8_t reg ) // Calculation parity of bits in reg. Result is true for odd number of bits 1 and false for even number of bits 1.
{
    reg ^= (reg >> 1);
    reg ^= (reg >> 2);
    reg ^= (reg >> 4);
    return ( (reg & 0x01) == 1 ? true : false );
}

void Z80Core::scf()// SCF - set carry flag
{
    regF &= ~(fN | fH);                                           // reset flags N and H
    regF |= fC;                                                   // set flag C
    /// Repeated below
    switch(m_producer) {                                          // different producer of Z80 change flags X and Y differenty
                 // Producer Zilog copy flags X and Y from accumulator when the previous instruction changed register F
                 // otherwise flags X and Y are ored with their previous value
    case pZilog: if( lastRegFChanged == true ) regF &= ~(fX | fY);// if flag register F was changed by previous instruction then reset flags X and Y
                 break;
                 // Producer NEC always copy flags X and Y from accumulator when the previous instruction changed register F
    case pNec:   regF &= ~(fX | fY);                              // reset flags X and Y
                 break;
                 // Producer ST copy flags Y from accumulator when the previous instruction changed register F otherwise flag Y is ored
                 // with their previous value, flag X is alwasy copied from accumulator when the previous instruction changed register F
    case pSt:    regF &= ~fX;                                     // reset flags X
                 if( lastRegFChanged == true ) regF &= ~fY;       // if flag register F was changed by previous instruction then reset flags Y
                 break;
    }
    regF |= regA & (fX | fY);                        // copy flags X and Y from accumulator
    regFChanged = true;                              // set flag change of register F - needed for instructions SCF and CCF
}

void Z80Core::ccf()// CCF - complement carry flag
{
    regF &= ~(fN | fH);                                         // reset flags N and H
    if ((regF & fC) != 0) regF |= fH;                           // copy flag C to flag H
    regF ^= fC;                                                 // invert flag C
    /// Repeated above
    switch(m_producer) {                                        // different producer of Z80 change flags X and Y differenty
                     // Producer Zilog copy flags X and Y from accumulator when the previous instruction changed register F
                     // otherwise flags X and Y are ored with their previous value
        case pZilog: if (lastRegFChanged == true) regF &= ~(fX | fY); // if flag register F was changed by previous instruction then reset flags X and Y
                     break;
                     // Producer NEC always copy flags X and Y from accumulator when the previous instruction changed register F
        case pNec:   regF &= ~(fX | fY);                         // reset flags X and Y
                     break;
                     // Producer ST copy flags Y from accumulator when the previous instruction changed register F otherwise flag Y is ored
                     // with their previous value, flag X is alwasy copied from accumulator when the previous instruction changed register F
        case pSt:    regF &= ~fX;                               // reset flags X
                     if (lastRegFChanged == true) regF &= ~fY;  // if flag register F was changed by previous instruction then reset flags Y
    }
    regF |= regA & (fX | fY);                  // copy flags X and Y from accumulator
    regFChanged = true;                        // set flag change of register F - needed for instructions SCF and CCF
}

void Z80Core::flags_IR()// Set flags for instruction LD A,I and LD A,R
{
    regF &= fC;                                                 // keep flag C only, reset the rest
    if( IFF2 == true ) regF |= fP;                              // copy IFF2 to flag P
    regF |= regA & (fX | fY | fS);                              // copy flags X, Y and S from accumulator
    if (regA == 0) regF |= fZ;                                  // set flag Z if accumulator is zero
    regFChanged = true;                                         // set flag change of register F - needed for instructions SCF and CCF
}

void Z80Core::flags_bit( const uint8_t &result )// Set flags for instruction BIT except flags X and Y, they must be set outside of this function
{
    regF &= fC;                                                 // keep flag C only, reset the rest
    regF |= result & fS;                                        // copy flag S from result
    if (result == 0) regF |= fZ | fP;                           // set flags Z and P if result is zero
    regF |= fH;                                                 //set flag H
    regFChanged = true;                                         // set flag change of register F - needed for instructions SCF and CCF
}

void Z80Core::ld_r_imm( uint8_t &reg ) //s LD r,N - load register r from byte after instruction
{
    switch(sm_MCycle) {
        case 1: mc_busOp = oMemRead; PC++; break;            // next bus op. is read byte from address PC
        case 2: reg = sDI;                                      // store byte after instruction to register
    }
}

void Z80Core::ld_r_indir(uint8_t &reg, const uint8_t &regH, const uint8_t &regL )//s LD A,(BC) and LD A,(DE) - load register from memory address stored in register pair
{
    switch(sm_MCycle) {
        case 1: regWZ = (regH << 8) | regL;                     // set memory pointer
                mc_busOp = oMemRead;                            // next bus op. is read byte from memory
                sAO = regWZ;                                    // set address for address bus
                break;
        case 2: reg = sDI;                                      // store byte from memory to register
                regWZ++;                                        // increase memory pointer
    }
}

void Z80Core::ld_r_indirXY( uint8_t &reg )//s LD r,(HL), LD r,(IX+d) and LD r,(IY+d) - load resister from memory address stored in register pair
{
    switch(sm_MCycle) {
                // Inicialization - if the instruction has prefix then the address offset fetch cycle is inserted
        case 1: if( XYState != rHL ) {                          // test if it is used register IX or IY
                    mc_StateMachine = sXYOffsetFetch;           // set address offset fetch flag - following machine cycle is machine cycle 6
                    mc_busOp = oMemRead;                        // next bus op. is read byte from memory at address PC
                } else {
                    mc_busOp = oMemRead;                        // next bus op. is read byte from memory
                    sAO = (regs[rH] << 8) | regs[rL];           // set address for address bus
                }
                break;
                // Read byte from memory and store it to register
        case 2: reg = sDI;                                      // store byte from memory to register
                break;
                // Address offset fetch
        case 6: regWZ = ( (regs[XYState] << 8) | regs[XYState + 1] ) + static_cast<signed char>(sDI);   // calculation of memory pointer
                mc_busOp = oNone;                               // next bus op. is no operation

                mc_TStates = 5;                                 // set number of TStates for machine cycle 7
                break;
                // Increase program counter and inicialization reading from memory; following machine cycle is machine cycle 2
        case 7: mc_busOp = oMemRead;                            // next bus op. is read byte from memory
                sAO = regWZ;                                    // set address for address bus
                PC++;                                        // increase program counter PC
    }
}

void Z80Core::ld_r_mem( uint8_t &reg )// LD A,(NN) - load register from memory addres from bytes after instruction
{
    switch(sm_MCycle) {
        case 1: mc_busOp = oMemRead; PC++; break; // next bus op. is read byte from address PC
                // Read low byte of address and store it to memory pointer
        case 2: regWZ = (regWZ & 0xff00) | sDI;                 // low byte is stored to low byte of memory pointer
                mc_busOp = oMemRead; PC++; break; // next bus op. is read byte from address PC
                // Read high byte of address and store it to memory pointer
        case 3: regWZ = (regWZ & 0x00ff) | (sDI << 8);          // high byte is stored to high byte of memory pointer
                mc_busOp = oMemRead;                            // next bus op. is read byte from memory
                sAO = regWZ;                                    // set address for address bus
                break;
                // Read byte from memory and store it to register
        case 4: reg = sDI;                                      // store byte from memory to register
                regWZ++;                                        // increase memory pointer
    }
}

void Z80Core::ld_indir_r( const uint8_t &regH, const uint8_t &regL, const uint8_t &reg )//s LD (BC),A and LD (DE),A - store register to memory address stored in register pair
{
    switch(sm_MCycle) {
        case 1: regWZ = (regH << 8) | regL;                     // set memory pointer
                writeMem( regWZ, reg ); break;
                // Write byte from register to memory
        case 2: regWZ++;                                        // increase memory pointer
                regWZ = (regWZ && 0x00ff) | (regA << 8);        // accumulator is stored to high byte of memory pointer
    }
}

void Z80Core::ld_indirXY_r( const uint8_t &reg )//s LD (HL),r, LD (IX+d),r and LD (IY+d),r - store resister r to memory address stored in register pair
{
    switch(sm_MCycle) {
                // Inicialization - if the instruction has prefix then the address offset fetch cycle is inserted
        case 1: if (XYState != rHL) {                           // test if it is used register IX or IY
                    mc_StateMachine = sXYOffsetFetch;           // set address offset fetch flag - following machine cycle is machine cycle 6
                    mc_busOp = oMemRead;                        // next bus op. is read byte from memory at address PC
                }
                else writeMem( (regs[rH] << 8) | regs[rL], reg );
                break;
        case 2: break;// Write byte from register to memory
                // Address offset fetch
        case 6: regWZ = ( (regs[XYState] << 8) | regs[XYState + 1] ) + static_cast<signed char>(sDI);   // calculation of memory pointer
                mc_busOp = oNone;                               // next bus op. is no operation
                mc_TStates = 5;                                 // set number of TStates for machine cycle 7
                break;
                // Increase program counter and inicialization writting to memory; following machine cycle is machine cycle 2
        case 7: writeMem( regWZ, reg );                         // next bus op. is write byte to memory
                PC++;                                        // increase program counter PC
    }
}

void Z80Core::ld_mem_r( const uint8_t &reg )// LD (NN),A - strore register to memory addres from bytes after instruction
{
    switch(sm_MCycle) {
        case 1: mc_busOp = oMemRead; PC++; break;            // next bus op. is read byte from address PC
        case 2: regWZ = (regWZ & 0xff00) | sDI;                 // low byte is stored to low byte of memory pointer
                mc_busOp = oMemRead; PC++; break;            // next bus op. is read byte from address PC
        case 3: regWZ = (regWZ & 0x00ff) | (sDI << 8);          // high byte is stored to high byte of memory pointer
                writeMem( regWZ, reg );                         // next bus op. is write byte to memory
                regWZ++;  break;                                // increase memory pointer
                // Write byte from register to memory
        case 4: regWZ = (regWZ && 0x00ff) | (regA << 8);        // accumulator is stored to high byte of memory pointer
                break;
    }
}

void Z80Core::ld_indirXY_imm() //s LD (HL),N, LD (IX+d),N and LD (IY+d),N - store byte after instruction to memory address stored in register pair
{
    switch(sm_MCycle) {
                // Inicialization - if the instruction has prefix then the address offset fetch cycle is inserted
        case 1: if (XYState != rHL) {                           // test if it is used register IX or IY
                    mc_StateMachine = sXYOffsetFetch;           // set address offset fetch flag - next machine cycle is machine cycle 6
                    mc_busOp = oMemRead;                 // next bus op. is read byte from memory at address PC
                } else { /// Always oMemRead???
                    mc_busOp = oMemRead;                       // next bus op. is read byte from memory at address PC
                }
                PC++;                                        // increase program counter PC
                break;
                // Read byte after instruction
        case 2: mc_busOp = oMemWrite;                           // next bus op. is write byte to memory
                if (XYState != rHL) sAO = regWZ;                // test if it is used register IX or IY
                else sAO = (regs[rH] << 8) | regs[rL];          // set address for address bus
                sDO = sDI;                                      // set data for data bus
                break;
        case 3: break;// Write byte after instruction to memory
                // Address offset fetch
        case 6: regWZ = ( (regs[XYState] << 8) | regs[XYState + 1] ) + static_cast<signed char>(sDI);   // calculation of memory pointer
                mc_StateMachine = sXYFetchFinish;               // set address offset fetch finish flag - next machine cycle is machine cycle 2
                mc_busOp = oMemRead; PC++; break;            // next bus op. is read byte from address PC
    }
}

void Z80Core::ld_rr_imm(uint8_t &regH, uint8_t &regL)//s LD rr,NN - load register pair rr from two bytes after instruction
{
    switch(sm_MCycle) {
        case 1: mc_busOp = oMemRead; PC++; break;            // next bus op. is read byte from address PC
                // Read byte after instruction and store it to low byte of register pair
        case 2: regL = sDI;                                     // store byte after instruction to low byte of register pair
                mc_busOp = oMemRead; PC++; break;            // next bus op. is read byte from address PC
                // Read byte after instruction and store it to high byte of register pair
        case 3: regH = sDI;                                     // store byte after instruction to high byte of register pair
                break;
    }
}

void Z80Core::ld_SP_imm() //s LD SP,NN - load stack pointer from two bytes after instruction
{
    switch(sm_MCycle) {
        case 1: mc_busOp = oMemRead; PC++; break;            // next bus op. is read byte from address PC
                // Read byte after instruction and store it to low byte of stack pointer
        case 2: regSP = (regSP & 0xFF00) | sDI;                 // store byte after instruction to low byte of stack pointer
                mc_busOp = oMemRead; PC++; break;            // next bus op. is read byte from address PC
                // Read byte after instruction and store it to high byte of stack pointer
        case 3: regSP = (regSP & 0x00FF) | (static_cast<uint16_t>(sDI) << 8); // store byte after instruction to high byte of stack pointer
                break;
    }
}

void Z80Core::ld_rr_mem(uint8_t &regH, uint8_t &regL) // LD rr,(NN) - load register pair rr from memory addres from bytes after instruction
{
    switch(sm_MCycle) {
        case 1: mc_busOp = oMemRead; PC++; break;            // next bus op. is read byte from memory at address PC
                // Read low byte of address and store it to memory pointer
        case 2: regWZ = (regWZ & 0xff00) | sDI;                 // low byte is stored to low byte of memory pointer
                mc_busOp = oMemRead; PC++; break;            // next bus op. is read byte from address PC
                // Read high byte of address and store it to memory pointer
        case 3: regWZ = (regWZ & 0x00ff) | (sDI << 8);          // high byte is stored to high byte of memory pointer
                mc_busOp = oMemRead;                            // next bus op. is read byte from memory
                sAO = regWZ;                                    // set address for address bus
                regWZ++;                                        // increase memory pointer
                break;
                // Read low byte from memory and store it to low byte of register pair
        case 4: regL = sDI;                                     // store byte from memory to low byte of register pair
                mc_busOp = oMemRead;                            // next bus op. is read byte from memory
                sAO = regWZ;                                    // set address for address bus
                break;
                // Read high byte from memory and store it to high byte of register pair
        case 5: regH = sDI;                                     // store byte from memory to high byte of register pair
                break;
    }
}

void Z80Core::ld_SP_mem() // LD SP,(NN) - load stack pointer from memory addres from bytes after instruction
{
    switch(sm_MCycle) {
        case 1: mc_busOp = oMemRead; PC++; break;            // next bus op. is read byte from address PC
                // Read low byte of address and store it to memory pointer
        case 2: regWZ = (regWZ & 0xff00) | sDI;                 // low byte is stored to low byte of memory pointer
                mc_busOp = oMemRead; PC++; break;            // next bus op. is read byte from address PC
                // Read high byte of address and store it to memory pointer
        case 3: regWZ = (regWZ & 0x00ff) | (sDI << 8);          // high byte is stored to high byte of memory pointer
                mc_busOp = oMemRead;                     // next bus op. is read byte from memory
                sAO = regWZ;                                    // set address for address bus
                regWZ++;                                        // increase memory pointer
                break;
                // Read low byte from memory and store it to low byte of stack pointer
        case 4: regSP = (regSP & 0xff00) | sDI;                 // store byte from memory to low byte of stack pointer
                mc_busOp = oMemRead;                     // next bus op. is read byte from memory
                sAO = regWZ;                                    // set address for address bus
                break;
                // Read high byte from memory and store it to high byte of stack pointer
        case 5: regSP = (regSP & 0x00ff) | (static_cast<uint16_t>(sDI) << 8);     // store byte from memory to high byte of stack pointer
                break;
    }
}

void Z80Core::ld_mem_rr(const uint8_t &regH, const uint8_t &regL) // LD rr,(NN) - store register pair rr to memory addres from bytes after instruction
{
    switch(sm_MCycle) {
        case 1: mc_busOp = oMemRead; PC++; break;            // next bus op. is read byte from address PC
        case 2: regWZ = (regWZ & 0xff00) | sDI;              // Read low byte of address and store it to memory pointer
                mc_busOp = oMemRead; PC++; break;            // next bus op. is read byte from address PC
        case 3: regWZ = (regWZ & 0x00ff) | (sDI << 8);       // Read high byte of address and store it to memory pointer
                writeMem( regWZ, regL );                     // high byte is stored to high byte of memory pointer
                regWZ++;                                     // increase memory pointer
                break;
        case 4: writeMem( regWZ, regH ); break;               // Write low byte from low byte of register pair to memory
        case 5: break;                                        // Write high byte from high byte of register pair to memory
    }
}

void Z80Core::push_rr(const uint8_t &regH, const uint8_t &regL) // PUSH rr - store register pair rr to stack
{
    switch(sm_MCycle) {                                        /// ???
        case 1: regSP--; writeMem( regSP, regH ); break;       // decrease SP // Inicialization
        case 2: regSP--; writeMem( regSP, regL ); break;       // decrease SP // Write low byte from low byte of register pair to stack
        case 3: break;                                         // Write high byte from high byte of register pair to stack
    }
}

void Z80Core::pop_rr(uint8_t &regH, uint8_t &regL) // POP rr - load register pair rr from stack
{
    switch(sm_MCycle) {
        case 1: mc_busOp = oMemRead;                            // next bus op. is read byte from address PC
                sAO = regSP;                                    // set address for address bus
                regSP++;                                        // increase stack pointer SP
                break;
                // Read low byte from stack to low byte of register pair
        case 2: regL = sDI;                                     // low byte is stored to low byte of register pair
                mc_busOp = oMemRead;                            // next bus op. is read byte from address PC
                sAO = regSP;                                    // set address for address bus
                regSP++;                                        // increase stack pointer SP
                break;
                // Read high byte from stack to low byte of register pair
        case 3: regH = sDI;                                     // high byte is stored to high byte of register pair
                break;
    }
}

void Z80Core::ex_rr_rr(uint8_t &regH1, uint8_t &regL1, uint8_t &regH2, uint8_t &regL2)//s EX AF,AF', EX DE,HL and EXX - exchange registers
{
    uint8_t tmp;
    tmp = regH1;                                                // exchange high byte of register pair
    regH1 = regH2;
    regH2 = tmp;
    tmp = regL1;                                                // exchange low byte of register pair
    regL1 = regL2;
    regL2 = tmp;
}

void Z80Core::ex_SP_rr(uint8_t &regH, uint8_t &regL) // EX (SP),HL, EX (SP),IX and EX (SP),IY - exchange register pair and last value in stack
{
    switch(sm_MCycle) {
        case 1: mc_busOp = oMemRead;                     // next bus op. is read low byte from stack
                sAO = regSP;                                    // set address for address bus
                break;
                // Read low byte from stack and exchange it with low byte of register pair
        case 2: regWZ = (regWZ & 0xff00) | sDI;                 // low byte from stack is stored to low byte of WZ
                writeMem( regSP, regL );                        // next bus op. is write low byte to stack
                regL = sDI;                                     // value read from data bus
                mc_TStates = 4;                                 // set number of TStates for machine cycle 3
                break;
                // Write low byte to stack
        case 3: regSP++;                                        // increase stack pointer
                mc_busOp = oMemRead;                     // next bus op. is read high byte from stack
                sAO = regSP;                                    // set address for address bus
                break;
                //  Read high byte from stack and exchange it with high byte of register pair
        case 4: regWZ = (regWZ & 0x00ff) | (sDI << 8);          // high byte from stack is stored to high byte of WZ
                writeMem( regSP, regH);                         // next bus op. is write high byte to stack
                regH = sDI;                                     // high byte is stored to high byte of register pair
                mc_TStates = 5;                                 // set number of TStates for machine cycle 5
                break;
                // Write high byte to stack
        case 5: regSP--;                                        // decrease stack pointer back
                break;
    }
}

void Z80Core::inc_r( uint8_t &reg ) // INC r - increase of register r by 1
{
    reg++;                                                      // increase register by 1
    ///
    regF &= fC;                                                 // keep flag C only, reset the rest
    regF |= reg & (fX | fY | fS);                               // copy flags X, Y and S from register
    if (reg == 0) regF |= fZ;                                   // set flag Z if register is zero
    if ( (reg & 0x0f) == 0x00 ) regF |= fH;           // set flag H if low for bits are zeros
    if (reg == 0x80)  regF |= fP;                               // set flag P if register is 0x80 after increase
    regFChanged = true;                                         // set flag change of register F - needed for instructions SCF and CCF
}

void Z80Core::dec_r( uint8_t &reg ) // DEC r - decrease of register r by 1
{
    reg--;                                                      // decrease register by 1
    ///
    regF &= fC;                                                 // keep flag C only, reset the rest
    regF |= fN;                                                 // set flag N
    regF |= reg & (fX | fY | fS);                               // copy flags X, Y and S from register
    if (reg == 0) regF |= fZ;                                   // set flag Z if register is zero
    if ( (reg & 0x0f) == 0x0f ) regF |= fH;            // set flag H if low for bits are ones
    if (reg == 0x7f) regF |= fP;                                // set flag P if register is 0x7f after decrease
    regFChanged = true;                                         // set flag change of register F - needed for instructions SCF and CCF
}

void Z80Core::inc_indirXY() // INC (HL), INC (IX+d) and INC (IY+d) - increase byte at memory address stored in register pair
{
    switch(sm_MCycle) {
                // Inicialization - if the instruction has prefix then the address offset fetch cycle is inserted
        case 1: if (XYState != rHL) {                           // test if it is used register IX or IY
                    mc_StateMachine = sXYOffsetFetch;           // set offset fetch flag - following machine cycle is machine cycle 6
                    mc_busOp = oMemRead;                        // next bus op. is read byte from memory at address PC
                } else {
                    mc_busOp = oMemRead;                        // next bus op. is read byte from memory
                    sAO = (regs[rH] << 8) | regs[rL];           // set address for address bus
                }
                mc_TStates = 4;                                 // set number of TStates for machine cycle 2
                break;
                // Read byte from memory and increase it
        case 2: inc_r(sDI);                                     // increase byte
                mc_busOp = oMemWrite;                           // next bus op. is write byte to memory
                if (XYState != rHL) sAO = regWZ;                // test if it is used register IX or IY// set address for address bus
                else sAO = (regs[rH] << 8) | regs[rL];          // set address for address bus
                sDO = sDI;                                      // set value for data bus
                break;
        case 3: break; // Write byte to memory
                // Address offset fetch
        case 6: regWZ = ( (regs[XYState] << 8) | regs[XYState + 1] ) + static_cast<signed char>(sDI);   // calculation of memory pointer
                mc_busOp = oNone;                        // next bus op. is no operation
                mc_TStates = 5;                                 // set number of TStates for machine cycle 7
                break;
                // Increase program counter and inicialization reading from memory; following machine cycle is machine cycle 2
        case 7: mc_busOp = oMemRead;                     // next bus op. is read byte from memory
                sAO = regWZ;                                    // set address for address bus
                PC++;                                        // increase program counter PC
                break;
    }
}
/// Almost copy of inc_indirXY()
void Z80Core::dec_indirXY()  // DEC (HL), DEC (IX+d) and DEC (IY+d) - decrease byte at memory address stored in register pair
{
    switch(sm_MCycle) {
                // Inicialization - if the instruction has prefix then the address offset fetch cycle is inserted
        case 1: if (XYState != rHL) {                           // test if it is used register IX or IY
                    mc_StateMachine = sXYOffsetFetch;           // set offset fetch flag - following machine cycle is machine cycle 6
                    mc_busOp = oMemRead;                        // next bus op. is read byte from memory at address PC
                } else {
                    mc_busOp = oMemRead;                        // next bus op. is read byte from memory
                    sAO = (regs[rH] << 8) | regs[rL];           // set address for address bus
                }
                mc_TStates = 4;                                 // set number of TStates for machine cycle 2
                break;
                // Read byte from memory and decrease it
        case 2: dec_r(sDI);                                     // decrease byte
                mc_busOp = oMemWrite;                           // next bus op. is write byte to memory
                if (XYState != rHL) sAO = regWZ;                // test if it is used register IX or IY set address for address bus
                else sAO = (regs[rH] << 8) | regs[rL];          // set address for address bus
                sDO = sDI;                                      // set value for data bus
                break;
                // Write byte to memory
        case 3: break;
                // Address offset fetch
        case 6: regWZ = ( (regs[XYState] << 8) | regs[XYState + 1] ) + static_cast<signed char>(sDI);   // calculation of memory pointer
                mc_busOp = oNone;                        // next bus op. is no operation
                mc_TStates = 5;                                 // set number of TStates for machine cycle 7
                break;
                // Increase program counter and inicialization reading from memory; following machine cycle is machine cycle 2
        case 7: mc_busOp = oMemRead;                     // next bus op. is read byte from memory
                sAO = regWZ;                                    // set address for address bus
                PC++;                                        // increase program counter PC
                break;
    }
}

void Z80Core::inc_rr( uint8_t &regH, uint8_t &regL ) // INC rr - increase of register pair rr by 1
{
    uint16_t r16 = (regH << 8) | regL;                    // compose low and high byte of register pair
    r16++;                                                // increase register pair
    regH = (r16 >> 8);                                    // store high byte to register pair
    regL = r16 & 0xff;                                    // store low byte to register pair
}

void Z80Core::dec_rr( uint8_t &regH, uint8_t &regL ) // DEC rr - decrease of register pair rr by 1
{
    uint16_t r16 = (regH << 8) | regL;                    // compose low and high byte of register pair
    r16--;                                                // decrease register pair
    regH = (r16 >> 8);                                    // store high byte to register pair
    regL = r16 & 0xff;                                    // store low byte to register pair
}

//s ADD A,(HL), ADD A,(IX+d) and ADD A,(IY+d) - addition of byte at memory address stored in register pair to accumulator
//s ADC A,(HL), ADC A,(IX+d) and ADC A,(IY+d) - addition of byte at memory address stored in register pair and flag C to accumulator
//s SUB (HL), SUB (IX+d) and SUB (IY+d) - subtraction of byte at memory address stored in register pair from accumulator
//s SBC A,(HL), SBC A,(IX+d) and SBC A,(IY+d) - subtraction of byte at memory address stored in register pair and flag C from accumulator
//s AND (HL), AND (IX+d) and AND (IY+d) - bit operator and of byte at memory address stored in register pair and accumulator
//s XOR (HL), XOR (IX+d) and XOR (IY+d) - bit operator xor of byte at memory address stored in register pair and accumulator
//s OR (HL), OR (IX+d) and OR (IY+d) - bit operator or of byte at memory address stored in register pair and accumulator
//s CP (HL), CP (IX+d) and CP (IY+d) - comparison of byte at memory address stored in register pair with accumulator
// This function is a tempate with paramter iAdd, iAdc, iSub, iSbc, iAnd, iXor, iOr or iCp which selects required matematical operation
template<Z80Core::eInstruction inst>
inline void Z80Core::inst_indirXY()
{
    switch(sm_MCycle) {
                // Inicialization - if the instruction has prefix then the address offset fetch cycle is inserted
        case 1: if (XYState != rHL) {                           // test if it is used register IX or IY
                    mc_StateMachine = sXYOffsetFetch;           // set address offset fetch flag - following machine cycle is machine cycle 6
                    mc_busOp = oMemRead;                 // next bus op. is read byte from memory at address PC
                } else {
                    mc_busOp = oMemRead;                 // next bus op. is read byte from memory
                    sAO = (regs[rH] << 8) | regs[rL];           // set address for address bus
                }
                break;
                // Read byte from memory and do selected matematical operation with accumulator
        case 2: switch(inst) {
                    case iAdd: add_r(sDI); break;               // addition
                    case iAdc: adc_r(sDI); break;               // addition with flag C
                    case iSub: sub_r(sDI); break;               // subtraction
                    case iSbc: sbc_r(sDI); break;               // subtraction with flag C
                    case iAnd: and_r(sDI); break;               // bit operator and
                    case iXor: xor_r(sDI); break;               // bit operator xor
                    case iOr : or_r(sDI); break;                // bit operator or
                    case iCp : cp_r(sDI); break;                // comparison
                }
                break;
                // Address offset fetch
        case 6: regWZ = ( (regs[XYState] << 8) | regs[XYState + 1] ) + static_cast<signed char>(sDI);   // calculation of memory pointer
                mc_busOp = oNone;                               // next bus op. is no operation
                mc_TStates = 5;                                 // set number of TStates for machine cycle 7
                break;
                // Increase program counter and inicialization reading from memory; following machine cycle is machine cycle 2
        case 7: mc_busOp = oMemRead;                            // next bus op. is read byte from memory
                sAO = regWZ;                                    // set address for address bus
                PC++;                                        // increase program counter PC
                break;
    }
}

// ADD A,N - addition of byte after instruction to accumulator
// ADC A,N - addition of byte after instruction and flag C to accumulator
// SUB N - subtraction of byte after instruction from accumulator
// SBC A,N - subtraction of byte after instruction and flag C from accumulator
// AND N - bit operator and of byte after instruction and accumulator
// XOR N - bit operator xor of byte after instruction and accumulator
// OR N - bit operator or of byte after instruction and accumulator
// CP N - comparison of byte after instruction with accumulator
// This function is a tempate with parameter iAdd, iAdc, iSub, iSbc, iAnd, iXor, iOr or iCp which selects required matematical operation
template<Z80Core::eInstruction inst>
inline void Z80Core::inst_imm()
{
    switch(sm_MCycle) {
                // Inicialization
        case 1: mc_busOp = oMemRead;                     // next bus op. is read byte from memory at address PC
                PC++;                                        // increase program counter PC
                break;
                // Read byte after instruction and do selected matematical operation with accumulator
        case 2: switch(inst) {
                    case iAdd: add_r(sDI); break;               // addition
                    case iAdc: adc_r(sDI); break;               // addition with flag C
                    case iSub: sub_r(sDI); break;               // subtraction
                    case iSbc: sbc_r(sDI); break;               // subtraction with flag C
                    case iAnd: and_r(sDI); break;               // bit operator and
                    case iXor: xor_r(sDI); break;               // bit operator xor
                    case iOr : or_r(sDI); break;                // bit operator or
                    case iCp : cp_r(sDI); break;                // comparison
                }
                break;
    }
}

// ADD A,r - addition of register r to accumulator
// The truth table for overflow bits is followed
// (A - addend, r - addend, A+r - result)
//
//  A  r  A+r  overflow
//  0  0   0       0
//  0  0   1       0
//  0  1   0       1
//  0  1   1       0
//  1  0   0       1
//  1  0   1       0
//  1  1   0       1
//  1  1   1       1
//
// The mathematical equation for overflow is followed
// overflow = A*r + /(A+r)*r + /(A+r)*A = A*r + /(A+r)*(r + A)
void Z80Core::add_r( const uint8_t &reg )
{
    uint8_t ovf = regA;                                   // store value of accumulator before addition for overflow calculation
    regA += reg;                                                // add register to accumulator
    /// Flags
    regF = regA & (fX | fY | fS);                               // reset flag N and copy flags X, Y and S from accumulator
    if (regA == 0) regF |= fZ;                                  // set flag Z if accumulator is zero

    ovf = (ovf & reg) | (~regA & (reg | ovf));                  // overflow bits calculation
    flagsCHP( ovf );
}

// ADC A,r - addition of register r and flag C to accumulator
// The function is similar to ADD A,r
void Z80Core::adc_r( const uint8_t &reg )
{
    uint8_t ovf = regA;                                   // store value of accumulator before addition for overflow calculation
    regA += reg + ((regF & fC) != 0);                           // add register and flag C to accumulator

    /// Flags
    regF = regA & (fX | fY | fS);                               // reset flag N and copy flags X, Y and S from accumulator
    if (regA == 0) regF |= fZ;                                  // set flag Z if accumulator is zero

    ovf = (ovf & reg) | (~regA & (reg | ovf));                  // overflow bits calculation
    flagsCHP( ovf );
}

// SUB r - subtraction of register r from accumulator
// The truth table for borrow bits is followed
// (A - minuend, r - subtrahend, A+r - result)
//
//  A  r  A+r  borrow
//  0  0   0      0
//  0  0   1      1
//  0  1   0      1
//  0  1   1      1
//  1  0   0      0
//  1  0   1      0
//  1  1   0      0
//  1  1   1      1
//
// The mathematical equation for borrow is followed
// borrow = /A*r + (A+r)*r + (A+r)*/A = /A*r + (A+r)*(r + /A)
void Z80Core::sub_r( const uint8_t &reg )
{
    uint8_t brw = regA;                                         // store value of accumulator before subtraction for borrow calculation
    regA -= reg;                                                // subtract register from accumulator

    /// Flags
    regF = regA & (fX | fY | fS);                               // copy flags X, Y and S from accumulator
    regF |= fN;                                                 // set flag N
    if( regA == 0 ) regF |= fZ;                                  // set flag Z if accumulator is zero

    brw = (~brw & reg) | (regA & (reg | ~brw));                 // borrow bits calculation
    flagsCHP( brw );
}

// SBC A,r - subtraction of register r and flag C from accumulator
// The function is similar to SUB r
void Z80Core::sbc_r( const uint8_t &reg )
{
    uint8_t brw = regA;                                         // store value of accumulator before subtraction for borrow calculation
    regA -= reg + ((regF & fC) != 0);                           // subtract register and flag C from accumulator

    /// Flags
    regF = regA & (fX | fY | fS);                               // copy flags X, Y and S from accumulator
    regF |= fN;                                                 // set flag N
    if( regA == 0 ) regF |= fZ;                                 // set flag Z if accumulator is zero

    brw = (~brw & reg) | (regA & (reg | ~brw));                 // borrow bits calculation
    flagsCHP( brw );
}

void Z80Core::and_r( const uint8_t &reg )// AND r - bit operator and of register r and accumulator
{
    regA &= reg;                                                // bit operator and
    regF = regA & (fX | fY | fS);                               // reset flags C, N, copy flags X, Y and S from accumulator
    regF |= fH;                                                 // set flag H
    flagsZP( regA );
}

void Z80Core::xor_r( const uint8_t &reg )// XOR r - bit operator xor of register r and accumulator
{
    regA ^= reg;                                                // bit operator xor
    regF = regA & (fX | fY | fS);                               // reset flags C, N, H, copy flags X, Y and S from accumulator
    flagsZP( regA );
}

void Z80Core::or_r( const uint8_t &reg )// OR r - bit operator or of register r and accumulator
{
    regA |= reg;
    regF = regA & (fX | fY | fS);                               // reset flags C, N, H, copy flags X, Y and S from accumulator
    flagsZP( regA );
}

// CP r - comparison of register r with accumulator
// It is the same instruction as SUB r, but the result is not stored to accumulator
/// Why flags X and Y from register and not from difference? not the same?
void Z80Core::cp_r( const uint8_t &reg )
{
    uint8_t tmp;
    tmp = regA - reg;                                           // subtract register from accumulator without storing it
    regF = reg & (fX | fY);                                     // copy flags X and Y from register
    regF |= tmp & fS;                                           // copy flag S from the calculated difference
    regF |= fN;                                                 // set flag N
    if (tmp == 0) regF |= fZ;                                   // set flag Z if calculated difference is zero

    tmp = (~regA & reg) | (tmp & (reg | ~regA));                // borrow bits calculation
    flagsCHP( tmp );
}
/*
 * void Z80Core::sub( uint8_t &reg )
{
    uint8_t brw = regA;
    m_tmp = regA - reg;                                        // subtract register from accumulator without storing it
    regF = reg & (fX | fY);                                    // copy flags X and Y from register
    regF |= m_tmp & fS;                                        // copy flag S from the calculated difference
    regF |= fN;                                                // set flag N
    if (m_tmp == 0) regF |= fZ;                                  // set flag Z if calculated difference is zero

    brw = (~brw & reg) | (m_tmp & (reg | ~brw) );               // borrow bits calculation
    /// Flags 2
    if( (brw & 0x80) != 0x00 ) regF |= fC;                    // set flag C if borrow from 8. bit
    if( (brw & 0x08) != 0x00 ) regF |= fH;                    // set flag H if borrow form 4. bit
    if( (brw & 0xC0) == 0x80  || (brw & 0xC0) == 0x40 ) regF |= fP; // set flag P if 7. or 6. bit borrow, but not simultaneously
    regFChanged = true;                                         // set flag change of register F - needed for instructions SCF and CCF
}*/

void Z80Core::daa()// DAA - decimal adjust of accumulator
{
    uint16_t tmp = regA;                                        // extend accumulator to 16 bits
     if ((regF & fN) == 0) {                                    // if flag N is not set the last operation was addition
        // Decimal adjust after addition
        if( (tmp & 0x000f) > 9 || (regF & fH) != 0 ) {          // if low nibble > 9 or flag H is set then the number is not valid and will be adjusted
            if ((tmp & 0x000f) > 9) regF |= fH;                 // if low nibble > 9 it means that it overflows after addition number 6: set flag H
            else                   regF &= ~fH;                 // else reset flag H
            tmp += 0x0006;                                      // add to low nibble difference between number 0x09 and 0x10 in hexadecimal
        }
        if( (tmp & 0xfff0) > 0x0090 || (regF & fC) != 0 ) {     // if high nibble> 9 or flag C is set then the number is not valid and will be adjusted
            tmp += 0x0060;                                      // add to high nibble difference between number 0x09 and 0x10 in hexadecimal
        }                                                       // overflow is stored in bit 8 of extended accumulator
    } else {                                                    // if flag N is set the last operation was subtraction
        // Decimal adjust after subtraction
        if( (tmp & 0x000f) > 9 || (regF & fH) != 0 ) {          // if low nibble > 9 or flag H is set then the number is not valid and will be adjusted
            if ((tmp & 0x000f) > 5) regF &= ~fH;                // if low nibble > 5 it does not borrow after subtraction number 6: reset flag H
            tmp -= 0x0006;                                      // subtract difference between number 0x09 and 0x10 in hexadecimal from the low nibble
            tmp &= 0x00ff;                                      // reset borrow in extended accumulator
        }
        if( regA > 0x0099 || (regF & fC) != 0 ) tmp -= 0x0160; // if number > 0x99 or flag C is set then the number is not valid and will be adjusted
                                                               // subtract difference between number 0x09 and 0x10 in hexadecimal  from the high nibble
                                                               // and set borrow in extended accumulator
    }
    regA = tmp & 0x00ff;                                        // copy low byte of extended accumulator to accumulator
    regF &= ~(fP | fX | fY | fZ | fS);                          // reset flags P, X, Y, Z and S
    regF |= regA & (fX | fY | fS);                              // copy flags X, Y and S from accumulator                               
    if ((tmp & 0xff00) != 0) regF |= fC;                        // logical OR overflow or borrow from extended accumulator and flag C
    flagsZP( regA );
}

void Z80Core::cpl() // CPL - accumulator one's complement
{
    regA = ~regA;                                               // accumulator one's complement
    regF &= ~(fX | fY);                                         // reset flags X and Y
    regF |= regA & (fX | fY);                                   // copy flags X and Y from accumulator
    regF |= fH | fN;                                            // set flags H and N
    regFChanged = true;                                         // set flag change of register F - needed for instructions SCF and CCF
}

// NEG - accumulator two's complement
// calculates accumulator = (0 - accumulator)
// (0 - minuend, A - subtrahend, 0-A - result)
//
//  0  A  0-A  borrow
//  0  0   0      0
//  0  0   1      1
//  0  1   0      1
//  0  1   1      1
//
// The mathematical equation for borrow is followed
// borrow = A + (0-A)
void Z80Core::neg()
{
    uint8_t brw = regA;
    regA = ~regA + 1;                                           // accumulator two's complement
    regF = regA & (fX | fY | fS);                               // copy flags X, Y and S from accumulator
    regF |= fN;                                                 // set flag N
    if( regA == 0 ) regF |= fZ;                                  // set flag Z if accumulator is zero

    brw |= regA;                                                // borrow bits calculation
    flagsCHP( brw );
}

// ADD rr,rr - addition of register pair rr and other register pair rr
// The addition is calculated in two steps. At first the low bytes are added and then the high bytes are added.
void Z80Core::add_rr_rr( uint8_t &regH1, uint8_t &regL1, const uint8_t &regH2, const uint8_t &regL2 )
{
    static uint16_t ovf;
    switch(sm_MCycle) {
    case 1: mc_busOp = oNone;      break;                   // next bus op. is no operation
            // Addition of low bytes
    case 2: ovf = regL1 + regL2;                            // add low bytes of register pairs
            regL1 = (ovf & 0x00ff);                         // store low byte of the result to register
            ovf &= 0xff00;                                  // keep overflow flag only
            regWZ = ( (regH1 << 8) | regL1 );               // set memory pointer
            mc_busOp = oNone;                               // next bus op. is no operation
            break;
            // Addition of high bytes
    case 3: ovf |= regH1;                                   // store value of high byte before addition for overflow calculation
            regH1 += regH2 + (ovf > 0x00ff);                // add high bytes of register pairs
            regF &= ~(fC | fN | fX | fH | fY);              // reset flags C, N, X, H and Y
            regF |= regH1 & (fX | fY);                      // copy flags X and Y from high byte of register pair
            ovf = (ovf & regH2) | (~regH1 & (regH2 | ovf)); // overflow bits calculation
            ///
            if ( (ovf & 0x80) != 0x00 ) regF |= fC;         // set flag C if 7. bit of high byte overflow
            if ( (ovf & 0x08) != 0x00 ) regF |= fH;         // set flag H if 3. bit of high byte overflow
            regFChanged = true;                             // set flag change of register F - needed for instructions SCF and CCF
            regWZ++;                                        // increase memory pointer
    }
}

// ADC rr,rr - addition of register pair rr and flag C and other register pair rr
// The addition is calculated in two steps. At first the low bytes are added and then the high bytes are added.
void Z80Core::adc_rr_rr( uint8_t &regH1, uint8_t &regL1, const uint8_t &regH2, const uint8_t &regL2 )
{
    static uint16_t ovf;
    switch(sm_MCycle) {
    case 1: mc_busOp = oNone;                        // next bus op. is no operation
            break;
            // Addition of low bytes and flag C
    case 2: ovf = regL1 + regL2 + ((regF & fC) != 0);       // add low bytes of register pairs and flag C
            regL1 = (ovf & 0x00ff);                         // store low byte of the result to register
            ovf &= 0xff00;                                  // keep overflow flag only
            regWZ = ( (regH1 << 8) | regL1 );               // set memory pointer
            mc_busOp = oNone;                               // next bus op. is no operation
            break;
            // Addition of high bytes
    case 3: ovf |= regH1;                                   // store value of high byte before addition for overflow calculation
            regH1 += regH2 + (ovf > 0x00ff);                // add high bytes of register pairs
            regF = regH1 & (fX | fY | fS);                  // reset flag N and copy flags X, Y and S from high byte of register pair
            if( regH1 == 0 && regL1 == 0 ) regF |= fZ;      // set flag Z if register pair is zero
            ovf = (ovf & regH2) | (~regH1 & (regH2 | ovf)); // overflow bits calculation
            flagsCHP( ovf );
            regWZ++;                                        // increase memory pointer
    }
}

// SBC rr,rr - subtraction of register pair rr and flag C from other regiser pair rr
// The subtraction is calculated in two steps. At first the low bytes are subtracted and then the high bytes are subtracted.
void Z80Core::sbc_rr_rr( uint8_t &regH1, uint8_t &regL1, const uint8_t &regH2, const uint8_t &regL2 )
{
    static uint16_t brw;
    switch(sm_MCycle) {
    case 1: mc_busOp = oNone;                        // next bus op. is no operation
            break;
            // Subtraction of low bytes and flag C
    case 2: brw = regL1 - (regL2 + ((regF & fC) != 0));     // subtract low byte of register pair and flag C from low byte of other register pair
            regL1 = (brw & 0x00ff);                         // store low byte of the result to register
            brw &= 0xff00;                                  // keep borrow flag only
            regWZ = (regH1 << 8) | regL1;                   // set memory pointer
            mc_busOp = oNone;                               // next bus op. is no operation
            break;
            // Subtraction of high bytes
    case 3: brw |= regH1;                                   // store value of high byte before subtraction for borrow calculation
            regH1 -= regH2 + (brw > 0x00ff);                // subtract high bytes of register pairs
            regF = regH1 & (fX | fY | fS);                  // copy flags X, Y and S from high byte of register pair
            regF |= fN;                                     // set flag N
            if( regH1 == 0 && regL1 == 0 ) regF |= fZ;      // set flag Z if register pair is zero
            brw = (~brw & regH2) | (regH1 & (regH2 | ~brw));// borrow bits calculation
            flagsCHP( brw );
            regWZ++;                                        // increase memory pointer
    }
}

void Z80Core::rlca() // RLCA - rotation left circular of accumulator
{
    regA = (regA << 1) | (regA >> 7);                           // rotation 1 bit left
    /// Flags 3
    regF &= ~(fC | fN | fX | fH | fY);                          // reset flags C, N, X, H, Y
    regF |= regA & (fX | fY);                                   // copy flags X and Y from accumulator
    if( (regA & 0x01) != 0 ) regF |= fC;                        // copy bit 0 after rotation of accumulator to flag C
    regFChanged = true;                                         // set flag change of register F - needed for instructions SCF and CCF
}

void Z80Core::rrca()// RRCA - rotation right circular of accumulator
{
    regA = (regA >> 1) | (regA << 7);                           // rotation 1 bit right
    /// Flags 3-2
    regF &= ~(fC | fN | fX | fH | fY);                          // reset flags C, N, X, H, Y
    regF |= regA & (fX | fY);                                   // copy flags X and Y from accumulator
    if( (regA & 0x80) != 0 ) regF |= fC;                        // copy bit 7 after rotation of accumulator to flag C
    regFChanged = true;                                         // set flag change of register F - needed for instructions SCF and CCF
}

void Z80Core::rla()// RLA - rotation left of accumulator
{
    uint8_t tmp = regA;
    regA = (regA << 1) | ( (regF & fC) != 0 );                  // shift 1 bit left and copy flag C to bit 0
    /// Flags 3
    regF &= ~(fC | fN | fX | fH | fY);                          // reset flags C, N, X, H, Y
    regF |= regA & (fX | fY);                                   // copy flags X and Y from accumulator
    if ( (tmp & 0x80) != 0 ) regF |= fC;                        // copy bit 7 before rotation of accumulator to flag C
    regFChanged = true;                                         // set flag change of register F - needed for instructions SCF and CCF
}

void Z80Core::rra()// RRA - rotation right of accumulator
{
    uint8_t tmp = regA;
    regA = (regA >> 1) | ( ( (regF & fC) != 0 ) << 7);          // shift 1 bit right and copy flag C to bit 7
    /// Flags 3-2
    regF &= ~(fC | fN | fX | fH | fY);                          // reset flags C, N, X, H, Y
    regF |= regA & (fX | fY);                                   // copy flags X and Y from accumulator
    if ( (tmp & 0x01) != 0 ) regF |= fC;                        // copy bit 0 before rotation of accumulator to flag C
    regFChanged = true;                                         // set flag change of register F - needed for instructions SCF and CCF
}

// RLD - rotation left digit
// Rotate 12 bit number 4 bit left where the low 8 bits of the number is stored in memory 
// at address in register pair HL and the high 4 bits of number is stored in accumulator
void Z80Core::rld()
{
    static uint8_t tmp;
    switch(sm_MCycle) {
    case 1: regWZ = (regs[rH] << 8) | regs[rL];             // set memory pointer
            mc_busOp = oMemRead;                     // next bus op. is read byte from memory
            sAO = regWZ;                                    // set address for address bus
            break;
            // Read byte from memory and rotate accumulator
    case 2: tmp = regA;                                     // store accumulator for next machine cycle
            regA = (regA & 0xf0) | (sDI >> 4);              // copy high nibble of read byte to accumulator
            /// Flags 4
            regF &= fC;                                     // keep flag C only, reset the rest
            regF |= regA & (fX | fY | fS);                  // copy flags X, Y and S from accumulator
            if (regA == 0) regF |= fZ;                      // set flag Z if accumulator is zero
            if ( parity(regA) == false ) regF |= fP;        // set flag P if accumulator parity is even
            regFChanged = true;                             // set flag change of register F - needed for instructions SCF and CCF
            mc_busOp = oNone;                               // next bus op. is no operation
            mc_TStates = 4;                                 // set number of TStates for machine cycle 3
            break;
            // Rotate byte from memory
    case 3: writeMem( regWZ, (sDI << 4) | (tmp & 0x0f) );   // copy low nibble of read byte to high nibble and copy low nibble from stored accumulator
            break;                                          // from previous machine cycle to low nibble
    case 4: regWZ++;                                        // increase memory pointer
    }
}

// RRD - rotation right digit
// Rotate 12 bit number 4 bit right where the low 8 bits of the number is stored in accumulator
// and the high 4 bits of number is stored in memory at address in register pair HL
void Z80Core::rrd()
{
    static uint8_t tmp;
    switch(sm_MCycle) {
    case 1: regWZ = (regs[rH] << 8) | regs[rL];             // set memory pointer
            mc_busOp = oMemRead;                     // next bus op. is read byte from memory
            sAO = regWZ;                                    // set address for address bus
            break;
            // Read byte from memory and rotate accumulator
    case 2: tmp = regA;                                     // store accumulator for next machine cycle
            regA = (regA & 0xf0) | (sDI & 0x0f);            // copy low nibble of read byte to accumulator
            /// Flags 4
            regF &= fC;                                     // keep flag C only, reset the rest
            regF |= regA & (fX | fY | fS);                  // copy flags X, Y and S from accumulator
            if (regA == 0) regF |= fZ;                      // set flag Z if accumulator is zero
            if ( parity(regA) == false ) regF |= fP;         // set flag P if accumulator parity is even
            regFChanged = true;                             // set flag change of register F - needed for instructions SCF and CCF
            mc_busOp = oNone;                               // next bus op. is no operation
            mc_TStates = 4;                                 // set number of TStates for machine cycle 3
            break;
            // Rotate byte from memory
    case 3: writeMem( regWZ, (sDI >> 4) | (tmp << 4) );     // copy high nibble of read byte to low nibble and copy low nibble from stored accumulator
            break;                                          // from previous machine cycle to high nibble
    case 4: regWZ++;                                        // increase memory pointer
    }
}

// RLC r, RLC (HL), RLC (IX+d), RLC (IY+d), LD r,RLC (IX+d) and LD r,RLC (IY+d) - rotation left circular of register or byte at memory address stored in register pair
// RRC r, RRC (HL), RRC (IX+d), RRC (IY+d), LD r,RRC (IX+d) and LD r,RRC (IY+d) - rotation right circular of register or byte at memory address stored in register pair
// RL  r, RL  (HL), RL  (IX+d), RL  (IY+d), LD r,RL  (IX+d) and LD r,RL  (IY+d) - rotation left of register or byte at memory address stored in register pair
// RR  r, RR  (HL), RR  (IX+d), RR  (IY+d), LD r,RR  (IX+d) and LD r,RR  (IY+d) - rotation right of register or byte at memory address stored in register pair
// SLA r, SLA (HL), SLA (IX+d), SLA (IY+d), LD r,SLA (IX+d) and LD r,SLA (IY+d) - shift left aritmetic of register or byte at memory address stored in register pair
// SRA r, SRA (HL), SRA (IX+d), SRA (IY+d), LD r,SRA (IX+d) and LD r,SRA (IY+d) - shift right aritmetic of register or byte at memory address stored in register pair
// SLL r, SLL (HL), SLL (IX+d), SLL (IY+d), LD r,SLL (IX+d) and LD r,SLL (IY+d) - shift left logical of register or byte at memory address stored in register pair
// SRL r, SRL (HL), SRL (IX+d), SRL (IY+d), LD r,SRL (IX+d) and LD r,SRL (IY+d) - shift right logical of register or byte at memory address stored in register pair

// BIT b,r, BIT b,(HL), BIT b,(IX+d) and BIT b,(IY+d) - test bit of register or byte at memory address stored in register pair
// RES b,r, RES b,(HL), RES b,(IX+d), RES b,(IY+d), LD r,RES b,(IX+d) and LD r,RES b,(IY+d) - reset bit of register or byte at memory address stored in register pair
// SET b,r, SET b,(HL), SET b,(IX+d), SET b,(IY+d), LD r,SET b,(IX+d) and LD r,SET b,(IY+d) - set bit of register or byte at memory address stored in register pair

// This function is a tempate with parameter iRlc, iRrc, iRl, iRr, iSla, iSra, iSll, iSrl, iBit, iRes or iSet which selects required operation
//
//s RLC r, RRC r, ... have only one machine cycle after prefix CB therefore only machine cycle 1 is executed
//
//s RLC (HL), RRC (HL), ... have three machine cycles after prefix CB therefore machine cycle from 1 to 3 are executed in sequence. The bit instrucion
// is one machine cycle shorter. The result is stored to register therefore it must be used some unused variable as function parameter.
//
// with prefix IX and IY have three machine cycles, but the address offset fetch are not after instruction itself, but after instruction PREFIX CB.
// In this case the sequence of machine cycles is machine cycle 1 of PREFIX CB, machine cycle 6 of PREFIX CB, machine cycle 7 of instruction, machine cycle 2
// of instruction and machine cycle 3 of instruction. The bit instrucion is one machine cycle shorter.  Machine cycle 1 of instruction is never executed.
// The result is stored to register therefore if it is not intended some unused variable must be used as function parameter.
template<Z80Core::eInstructionCB inst, uint8_t mask = 0x00>
void Z80Core::instCB( uint8_t &reg )
{
    switch(sm_MCycle) {
                // Inicialization or doing selected operation with register
    case 1: if (mc_MCycles == 1) {                          // if instruction has only one machine cycle matematical operation is with register
                switch(inst) {
                    case iRlc: rlc_r(reg); break;           // rotation left circular of register
                    case iRrc: rrc_r(reg); break;           // rotation right circular of register
                    case iRl : rl_r(reg); break;            // rotation left of register
                    case iRr : rr_r(reg); break;            // rotation right of register
                    case iSla: sla_r(reg); break;           // shift left aritmetic of register
                    case iSra: sra_r(reg); break;           // shift right aritmetic of register
                    case iSll: sll_r(reg); break;           // shift left logical of register
                    case iSrl: srl_r(reg); break;           // shift right logical of register
                    case iBit: flags_bit(reg & mask);       // test bit of register
                               regF |= reg & (fX | fY);     // copy flags X and Y from register
                               break;
                    case iRes: reg &= mask; break;          // reset bit of register
                    case iSet: reg |= mask; break;          // set bit of register
                }
            } else {                                        // if instruction has more than one machine cycle matematical operation is with byte at memory address stored in register pair
                mc_busOp = oMemRead;                 // next bus op. is read byte from memory
                sAO = (regs[rH] << 8) | regs[rL];           // set address for address bus
            }
            break;
            // Read byte from memory and do selected operation with byte at memory address stored in register pair
    case 2: switch(inst) {
                case iRlc: rlc_r(sDI); break;               // rotation left circular of byte at memory address stored in register pair
                case iRrc: rrc_r(sDI); break;               // rotation right circular of byte at memory address stored in register pair
                case iRl : rl_r(sDI); break;                // rotation left of byte at memory address stored in register pair
                case iRr : rr_r(sDI); break;                // rotation right of byte at memory address stored in register pair
                case iSla: sla_r(sDI); break;               // shift left aritmetic of byte at memory address stored in register pair
                case iSra: sra_r(sDI); break;               // shift right aritmetic of byte at memory address stored in register pair
                case iSll: sll_r(sDI); break;               // shift left logical of byte at memory address stored in register pair
                case iSrl: srl_r(sDI); break;               // shift right logical of byte at memory address stored in register pair
                case iBit: flags_bit(sDI & mask);           // test bit of byte at memory address stored in register pair
                           regF |= (regWZ >> 8) & (fX | fY);// copy flags X and Y from memory pointer
                           break;
                case iRes: sDI &= mask; break;              // reset bit of byte at memory address stored in register pair
                case iSet: sDI |= mask; break;              // set bit of byte at memory address stored in register pair
            }
            if( inst == iBit ) mc_MCycles = 2;              // if the operation is bit test the last machine cycle is machine cycle 2
                                                            // set number of machine cycles to 2 to finish instruction BIT
            else {                                          // the operation except bit test has last machine cycle 3
                mc_MCycles = 3;                             // set number of machine cycles to 3
                mc_busOp = oMemWrite;                       // next bus op. is write byte to memory
                if (XYState != rHL) sAO = regWZ;            // test if it is used register IX or IY
                else     sAO = (regs[rH] << 8) | regs[rL];  // set address for address bus
                sDO = sDI;                                  // set data for data bus
            }
            break;
            // Write byte to memory and to register
    case 3: reg = sDO;                                      // copy byte from memory also to register
            break;
            // New opcode fetch and program counter PC increase is in function clkRisingEdge()
            // Inicialization reading from memory; following machine cycle is machine cycle 2
    case 7: mc_busOp = oMemRead;                            // next bus op. is read byte from memory
            sAO = regWZ;                                    // set address for address bus
            mc_TStates = 4;                                 // set number of TStates for machine cycle 2
    }
}

void Z80Core::rlc_r( uint8_t &reg )// RLC r - rotation left circular of register r
{
    reg = (reg << 1) | (reg >> 7);                              // rotation 1 bit left
    regF = reg & (fX | fY | fS);                                // reset flags N, H, copy flags X, Y and S from accumulator
    if ( (reg & 0x01) != 0 ) regF |= fC;                        // copy bit 0 after rotation of accumulator to flag C
    flagsZP( reg );
}

void Z80Core::rl_r( uint8_t &reg )// RL r - rotation left of register r
{
    uint8_t tmp = reg;
    reg = (reg << 1) | ( (regF & fC) != 0 );                    // shift 1 bit left and copy flag C to bit 0
    regF = reg & (fX | fY | fS);                                // reset flags N, H, copy flags X, Y and S from accumulator
    if( (tmp & 0x80) != 0 ) regF |= fC;                         // copy bit 7 before rotation of accumulator to flag C
    flagsZP( reg );
}

void Z80Core::rrc_r( uint8_t &reg )// RRC r - rotation right circular of register r
{
    reg = (reg >> 1) | (reg << 7);                              // rotation 1 bit right
    regF = reg & (fX | fY | fS);                                // reset flags N, H, copy flags X, Y and S from accumulator
    if( (reg & 0x80) != 0 ) regF |= fC;                         // copy bit 7 after rotation of accumulator to flag C
    flagsZP( reg );
}

void Z80Core::rr_r( uint8_t &reg )// RL r - rotation right of register r
{
    uint8_t tmp = reg;
    reg = (reg >> 1) | ( ( (regF & fC) != 0 ) << 7);            // shift 1 bit right and copy flag C to bit 7
    regF = reg & (fX | fY | fS);                                // reset flags N, H, copy flags X, Y and S from accumulator
    if( (tmp & 0x01) != 0 ) regF |= fC;                         // copy bit 0 before rotation of accumulator to flag C
    flagsZP( reg );
}

void Z80Core::sla_r( uint8_t &reg )// SLA r - shift left aritmetic of register r
{
    uint8_t tmp = reg;
    reg = (reg << 1);                                           // shift 1 bit left
    regF = reg & (fX | fY | fS);                                // reset flags N, H, copy flags X, Y and S from accumulator
    if( (tmp & 0x80) != 0 ) regF |= fC;                         // copy bit 7 before shift of accumulator to flag C
    flagsZP( reg );
}

void Z80Core::sra_r( uint8_t &reg )// SRA r - shift right aritmetic of register r
{
    uint8_t tmp = reg;
    reg = (reg >> 1) | (reg & 0x80);                            // shift 1 bit right and keep bit 7
    regF = reg & (fX | fY | fS);                                // reset flags N, H, copy flags X, Y and S from accumulator
    if ( (tmp & 0x01) != 0 ) regF |= fC;                        // copy bit 0 before shift of accumulator to flag C
    flagsZP( reg );
}

void Z80Core::sll_r( uint8_t &reg )// SLL r - shift left logical of register r
{
    uint8_t tmp = reg;
    reg = (reg << 1) | 0x01;                                    // shift 1 bit left and set bit 0
    regF = reg & (fX | fY | fS);                                // reset flags N, H, copy flags X, Y and S from accumulator
    if ( (tmp & 0x80) != 0 ) regF |= fC;                        // copy bit 7 before shift of accumulator to flag C
    flagsZP( reg );
}

void Z80Core::srl_r( uint8_t &reg )// SRL r - shift right logical of register r
{
    uint8_t tmp = reg;
    reg = (reg >> 1);                                           // shift 1 bit right
    regF = reg & (fX | fY | fS);                                // reset flags N, H, copy flags X, Y and S from accumulator
    if ( (tmp & 0x01) != 0 ) regF |= fC;                        // copy bit 0 before shift of accumulator to flag C
    flagsZP( reg );
}

void Z80Core::in_r_imm( uint8_t &reg )// IN r,(N) - load register r from I/O address from byte after instruction
{
    switch(sm_MCycle) {
    case 1: mc_busOp = oMemRead; PC++; break;            // next bus op. is read byte from address PC
            // Read byte after instruction and store it memory pointer
    case 2: regWZ = (reg << 8) | sDI;                       // set memory pointer
            mc_busOp = oIORead;                             // next bus op. is read byte from I/O
            sAO = regWZ;                                    // set address for address bus
            break;
    case 3: reg = sDI;                                      // store byte from I/O to register
            regWZ++;                                        // increase memory pointer
    }
}

void Z80Core::in_r_rr( uint8_t &reg, const uint8_t &regH, const uint8_t &regL )// IN r,(rr) - load register r from I/O address stored in register pair
{
    switch(sm_MCycle) {
    case 1: regWZ = (regH << 8) | regL;                     // set memory pointer
            mc_busOp = oIORead;                             // next bus op. is read byte from I/O
            sAO = regWZ;                                    // set address for address bus
            break;
    case 2: reg = sDI;                                      // store byte from I/O to register
            regF &= fC;                                     // keep flag C only, reset the rest
            regF |= reg & (fX | fY | fS);                   // copy flags X, Y and S from register
            flagsZP( reg );
            regWZ++;                                        // increase memory pointer
    }
}

void Z80Core::out_imm_r( const uint8_t &reg ) // OUT (N),r - store register r to I/O address from byte after instruction
{
    switch(sm_MCycle) {
    case 1: mc_busOp = oMemRead; PC++; break;            // next bus op. is read byte from address PC
            // Read byte after instruction and store it memory pointer
    case 2: regWZ = (reg << 8) | sDI;                       // set memory pointer
            mc_busOp = oIOWrite;                            // next bus op. is write byte to I/O
            sAO = regWZ;                                    // set address for address bus
            sDO = reg;                                      // set data for data bus
            break;
    case 3: regWZ++;                                        // increase memory pointer
    }
}

void Z80Core::out_rr_r( const uint8_t &regH, const uint8_t &regL, const uint8_t &reg ) // OUT (rr),r - store register r to I/O address stored in register pair
{
    switch(sm_MCycle) {
    case 1: regWZ = (regH << 8) | regL;                     // set memory pointer
            mc_busOp = oIOWrite;                            // next bus op. is write byte to I/O
            sAO = regWZ;                                    // set address for address bus
            sDO = reg;                                      // set data for data bus
            break;
    case 2: regWZ++;                                        // increase memory pointer
    }
}

void Z80Core::djnz( uint8_t &reg ) // DJNZ N - decrease register r and if it is not zero then jump to relative address from byte after instruction
{
    switch(sm_MCycle) {
            // Inicialization and decrement of loop counter
    case 1: reg--;                                          // decrease register
            mc_busOp = oMemRead; PC++; break;            // next bus op. is read byte from address PC
            // Read byte after instruction and if the register is zero then finish instruction
    case 2: if (reg != 0) {                                 // if register is not zero then jump to relative address otherwise the instruction is finished
                mc_busOp = oNone;                           // next bus op. is no operation
                mc_TStates = 5;                             // set number of TStates for machine cycle 3
            }
            else mc_MCycles = 2;                            // set number of machine cycles to 2 - finish instruction
            break;
            // Calculation and set program counter PC
    case 3: PC += static_cast<signed char>(sDI);         // calculation program counter
            regWZ = PC;                                  // set memory pointer
            sAO = PC;                                    // set address for address bus
    }
}

void Z80Core::jr( const bool &cond )// JR N and JR cond,N - if condition cond is true then jump to relative address from byte after instruction
{
    switch( sm_MCycle ) {
    case 1: mc_busOp = oMemRead; PC++; break;            // next bus op. is read byte from address PC
            // Read byte after instruction and if the condition is false then finish instruction

            /// Repeated above
    case 2: if (cond) {                                     // if condition is true then jump to relative address otherwise the instruction is finished
                mc_busOp = oNone;                           // next bus op. is no operation
                mc_TStates = 5;                             // set number of TStates for machine cycle 3
            }
            else mc_MCycles = 2;                             // set number of machine cycles to 2 - finish instruction
            break;
            // Calculation and set program counter PC
    case 3: PC += static_cast<signed char>(sDI);         // calculation program counter
            regWZ = PC;                                  // set memory pointer
            sAO = PC;                                    // set address for address bus
    }
}

void Z80Core::jp( const bool &cond ) // JP NN and JP cond,NN - if condition cond is true then jump to address from bytes after instruction
{
    switch(sm_MCycle) {
    case 1: mc_busOp = oMemRead; PC++; break;            // next bus op. is read byte from address PC
            // Read byte after instruction and store it to low byte of memory pointer
    case 2: regWZ = (regWZ & 0xff00) | sDI;                 // low byte is stored to low byte of memory pointer
            mc_busOp = oMemRead; PC++; break;            // next bus op. is read byte from address PC
            // Read byte after instruction and store it to high byte of memory pointer and if the condition is true change program counter PC
    case 3: regWZ = (regWZ & 0x00ff) | (sDI << 8);          // high byte is stored to high byte of memory pointer
            if (cond) {                                     // if condition is true then jump to address of memory pointer
                PC = regWZ;                              // set program counter
                sAO = PC;                                // set address for address bus
            }
    }
}

void Z80Core::call( const bool &cond )// CALL NN and CALL cond,NN - if condition cond is true then store program counter PC to stack and jump to address from bytes after instruction
{
    switch(sm_MCycle) {
    case 1: mc_busOp = oMemRead; PC++; break;            // next bus op. is read byte from address PC
            // Read byte after instruction and store it to low byte of memory pointer
    case 2: regWZ = (regWZ & 0xff00) | sDI;                 // low byte is stored to low byte of memory pointer
            mc_busOp = oMemRead;                            // next bus op. is read byte from address PC
            PC++;                                        // increase program counter PC
            mc_TStates = 4;                                 // set number of TStates for machine cycle 3
            break;
            // Read byte after instruction and store it to high byte of memory pointer and if the condition is false then finish instruction
    case 3: regWZ = (regWZ & 0x00ff) | (sDI << 8);          // high byte is stored to high byte of memory pointer
            if (cond) {                                     // if condition is true then store program counter PC to stack and jump to address otherwise the instruction is finished
                regSP--;                                    // decrease stack pointer SP
                writeMem( regSP, (PC >> 8) );            // next bus op. is write byte to stack
            }
            else mc_MCycles = 3;                             // set number of machine cycles to 3 - finish instruction
            break;
            // Write low byte of program counter PC to stack
    case 4: regSP--;                                        // decrease stack pointer SP
            writeMem( regSP, PC & 0xff ); break;         // next bus op. is write byte to stack
            // Write high byte of program counter PC to stack and change program counter PC
    case 5: PC = regWZ;                                  // set program counter
            sAO = PC;                                    // set address for address bus
    }
}

void Z80Core::rst( const uint8_t &addr )//s RST NN - store PC to stack and jump to address NN
{
    switch(sm_MCycle) {
    case 1: regSP--;                                        // decrease stack pointer SP
            writeMem( regSP, PC >> 8 ); break;           // next bus op. is write byte to stack
            // Write high byte of program counter PC to stack
    case 2: regSP--;                                        // decrease stack pointer SP
            writeMem( regSP, PC & 0xff ); break;         // next bus op. is write byte to stack
            // Write low byte of program counter PC to stack and change program counter PC
    case 3: regWZ = addr;                                   // set memory pointer
            PC = addr;                                   // set program counter
            sAO = PC;                                    // set address for address bus
    }
}

void Z80Core::ret( const bool &cond ) //s RET and RET cond - if condition cond is true then load program counter PC from stack
{
    switch(sm_MCycle) {
    case 1: if (cond){ mc_busOp = oMemRead; PC++; }      // if condition is true then load program counter PC from stack otherwise the instruction is finished
            else mc_MCycles = 1;                            // set number of machine cycles to 1 - finish instruction
            break;
            // Read low byte of program counter PC from stack
    case 2: regWZ = (regWZ & 0xff00) | sDI;                 // low byte is stored to low byte of memory pointer
            mc_busOp = oMemRead;                            // next bus op. is read byte from stack
            sAO = regSP;                                    // set address for address bus
            regSP++;                                        // increase stack pointer SP
            break;
            // Read high byte of program counter PC from stack
    case 3: regWZ = (regWZ & 0x00ff) | (sDI << 8);          // high byte is stored to high byte of memory pointer
            PC = regWZ;                                  // set program counter
            sAO = PC;                                    // set address for address bus
    }
}

void Z80Core::retn()//s RETI and RETN - load program counter PC from stack and copy flag IFF2 to flag IFF1
{
    switch(sm_MCycle) {
    case 1: mc_busOp = oMemRead;                            // next bus op. is read byte from stack
            sAO = regSP;                                    // set address for address bus
            regSP++;                                        // increase stack pointer SP
            break;
            // Read low byte of program counter PC from stack
    case 2: regWZ = (regWZ & 0xff00) | sDI;                 // low byte is stored to low byte of memory pointer
            mc_busOp = oMemRead;                            // next bus op. is read byte from stack
            sAO = regSP;                                    // set address for address bus
            regSP++;                                        // increase stack pointer SP
            break;
            // Read high byte of program counter PC from stack
    case 3: regWZ = (regWZ & 0x00ff) | (sDI << 8);          // high byte is stored to high byte of memory pointer
            PC = regWZ;                                  // set program counter
            sAO = PC;                                    // set address for address bus
            IFF1 = IFF2;                                    // copy flag IFF2 to flag IFF1
    }
}

// NOP during INT IM2 - When INT is actived and interrupt mode is set to 2 the interrupt vector is read from data bus and corresponding function is called
void Z80Core::int_im2()
{
    switch(sm_MCycle) {
            // Read interrupt vector and store it to memory pointer
    case 1: regWZ = (regWZ & 0xff00) | sDI;                 // low byte is stored to low byte of memory pointer
            regSP--;                                        // decrease stack pointer SP
            writeMem( regSP, PC >> 8 );                  // next bus op. is write byte to stack
            break;
            // Write high byte of program counter PC to stack
    case 2: regSP--;                                        // decrease stack pointer SP
            writeMem( regSP, PC & 0xff );                // next bus op. is write byte to stack
            break;
            // Write low byte of program counter PC to stack and calculate addrese in interrupt vector table
    case 3: PC = (regI << 8) | ((m_intVector == true) ? 0xff : (regWZ & 0xff));
            mc_busOp = oMemRead;                            // next bus op. is read byte from address PC
            PC++;                                        // increase program counter PC
            break;
            // Read low byte from interrupt vector table
    case 4: regWZ = (regWZ & 0xff00) | sDI;                 // low byte is stored to low byte of memory pointer
            mc_busOp = oMemRead;                            // next bus op. is read byte from address PC
            break;
            // Read high byte from interrupt vector table and change program counter PC
    case 5: regWZ = (regWZ & 0x00ff) | (sDI << 8);          // high byte is stored to high byte of memory pointer
            PC = regWZ;                                  // set program counter
            sAO = PC;                                    // set address for address bus
    }
}

//s LDI, LDD, LDIR, LDDR - block transfers from memory to memory
// This function is a tempate with parameter bInc, bIncRep, bDec or bDecRep which selects increase or decrease of memory address and repeatibility
template<Z80Core::eBlock type>
inline void Z80Core::ldxx()
{
    switch(sm_MCycle) {
            // Inicialization and decrease counter of bytes to transfer
    case 1: dec_rr(regs[rB], regs[rC]);                     // decrease counter of bytes to transfer
            mc_busOp = oMemRead;                            // next bus op. is read byte from memory
            sAO = (regs[rH] << 8) | regs[rL];               // set address for address bus
            break;
            // Read byte from memory and increase/decrease register with source address
    case 2: if( type == bInc || type == bIncRep )           // if the instruction is LDI or LDIR
                inc_rr(regs[rH], regs[rL]);                 // increase source address

            if( type == bDec || type == bDecRep )           // if the instruction is LDD or LDDR
                dec_rr(regs[rH], regs[rL]);                 // decrease source address

            writeMem( (regs[rD] << 8) | regs[rE], sDI );    // next bus op. is write byte to memory
            mc_TStates = 5;                                 // set number of TStates for machine cycle 3
            break;
            // Write byte to memory and increase/decrease register with destination address and check of termination of instruction
    case 3: if( type == bInc || type == bIncRep )          // if the instruction is LDI or LDIR
                inc_rr(regs[rD], regs[rE]);                 // increase destination address

            if( type == bDec || type == bDecRep )           // if the instruction is LDD or LDDR
                dec_rr(regs[rD], regs[rE]);                 // decrease destination address

            regFChanged = true;                             // set flag change of register F - needed for instructions SCF and CCF
            if ( regs[rB] != 0 || regs[rC] != 0 ) {         // if counter of bytes to transfer is not zero then repeat instructions LDIR or LDDR or finish instruction LDI or LDD
                regF |= fP;                                 // set flag P - indication that counter of bytes to transfer is not zero
                if( type == bIncRep || type == bDecRep ) {  // if the instruction is LDIR or LDDR then continue exucuting instruction
                    mc_busOp = oNone;                       // next bus op. is no operation
                    mc_TStates = 5;                         // set number of TStates for machine cycle 4
                } else {                                    // if the instruction is LDI or LDD then finish instruction
                    regF &= ~(fN | fX | fH | fY);           // reset flags N, P, X, H and Y
                    regF |= (sDI + regA) & fX;              // copy flags X from sum of accumulator and last transfered byte
                    regF |= ((sDI + regA) << 4) & fY;       // copy flags Y from sum of accumulator and last transfered byte
                }
            } else {                                        // if counter of bytes to transfer is zero then finish instruction
                regF &= ~(fN | fP | fX | fH | fY);          // reset flags N, P, X, H and Y
                regF |= (sDI + regA) & fX;                  // copy flags X from sum of accumulator and last transfered byte
                regF |= ((sDI + regA) << 4) & fY;           // copy flags Y from sum of accumulator and last transfered byte
                mc_MCycles = 3;                             // set number of machine cycles to 3 - finish instruction
            }
            break;
            // Decrease program counter PC to first byte of this instrucion
    case 4: PC -= 2;                                     // set program counter PC back to this instruction
            regF &= ~(fN |fX | fH | fY);                    // reset flags N and H
            regF |= (PC >> 8) & (fX | fY);               // copy flags X and Y from program counter PC
            regWZ = PC + 1;                              // set memory pointer
            sAO = PC;                                    // set address for address bus
    }
}

//s CPI, CPD, CPIR, CPDR - block comparison between memory and accumulator
// This function is a tempate with parameter bInc, bIncRep, bDec or bDecRep which selects increase or decrease of memory address and repeatibility
template<Z80Core::eBlock type>
inline void Z80Core::cpxx()
{
    static uint8_t flags;
    switch(sm_MCycle) {
            // Inicialization and decrease counter of bytes to transfer
    case 1: dec_rr(regs[rB], regs[rC]);                     // decrease counter of bytes to compare
            mc_busOp = oMemRead;                            // next bus op. is read byte from memory
            sAO = (regs[rH] << 8) | regs[rL];               // set address for address bus
            break;
            // Read byte from memory and compare it with accumulator and increase/decrease register with memory address
    case 2: flags = regF;                                   // store flag C, because this instruction doesn't change flag C
            cp_r(sDI);                                      // compare read byte and accumulator and set flags
            if( type == bInc || type == bIncRep ) {         // if the instruction is CPI or CPIR
                inc_rr(regs[rH], regs[rL]);                 // increase memory address for comparison
                regWZ++;                                    // increase memory pointer
            }
            if ( type == bDec || type == bDecRep ) {        // if the instruction is CPD or CPDR
                dec_rr(regs[rH], regs[rL]);                 // decrease memory address for comparison
                regWZ--;                                    // decrease memory pointer
            }
            mc_busOp = oNone;                               // next bus op. is no operation
            mc_TStates = 5;                                 // set number of TStates for machine cycle 3
            break;
            // Check of termination of instruction
    case 3: regFChanged = true;                             // set flag change of register F - needed for instructions SCF and CCF
            regF &= ~(fC | fP | fX | fY);                   // reset flags C, P, X and Y
            regF |= (flags & fC);                           // restore flag C to state before exucution of this instruction
            flags = regA - sDI - ((regF & fH) != 0);        // flags calculation
            regF |= flags & fX;                             // copy flag X
            regF |= (flags << 4) & fY;                      // copy flag Y

            if( regs[rB] != 0 || regs[rC] != 0 ) regF |= fP;// if counter of bytes to compare is not zero then set flag P

            if( (regF & fZ) == 0 && (regF & fP) != 0
             && ( type == bIncRep || type == bDecRep ) ) {  // if byte is not found and counter of bytes to compare is not zero then repeat instructions CPIR or CPDR or finish instruction CPI or CPD
                mc_busOp = oNone;                           // next bus op. is no operation
                mc_TStates = 5;                             // set number of TStates for machine cycle 4
            } else mc_MCycles = 3;                          // if byte was found or counter of bytes to transfer is zero then finish instruction
            break;
            // Decrease program counter PC to first byte of this instrucion
    case 4: PC -= 2;                                     // set program counter PC back to this instruction
            regF &= ~(fX | fY);                             // reset flags X and Y
            regF |= (PC >> 8) & (fX | fY);               // copy flags X and Y from program counter PC
            regWZ = PC + 1;                              // set memory pointer
            sAO = PC;                                    // set address for address bus
    }
}

// INI, IND, INIR, INDR - block transfers from I/O address to memory
// This function is a tempate with parameter bInc, bIncRep, bDec or bDecRep which selects increase or decrease of memory address and repeatibility
template<Z80Core::eBlock type>
inline void Z80Core::inxx()
{
    uint16_t ioq;
    switch(sm_MCycle) {
            // Inicialization and decrease counter of bytes to transfer
    case 1: regWZ = (regs[rB] << 8) | regs[rC];             // set memory pointer
            mc_busOp = oIORead;                             // next bus op. is read byte from I/O
            sAO = (regs[rB] << 8) | regs[rC];               // set address for address bus
            regs[rB]--;                                     // decrease counter of bytes to transfer
            break;
            // Read byte from I/O address
    case 2: writeMem( (regs[rH] << 8) | regs[rL], sDI );    // next bus op. is write byte to memory
            mc_TStates = 5;                                 // set number of TStates for machine cycle 3
            break;
            // Write byte to memory, increase/decrease register with memory address and check of termination of instruction
    case 3: if( type == bInc || type == bIncRep ) {         // if the instruction is INI or INIR
                inc_rr(regs[rH], regs[rL]);                 // increase memory address for transfer
                regWZ++;                                    // increase memory pointer
            }
            if( type == bDec || type == bDecRep ) {         // if the instruction is IND or INDR
                dec_rr(regs[rH], regs[rL]);                 // decrease memory address to transfer
                regWZ--;                                    // decrease memory pointer
            }
            regFChanged = true;                             // set flag change of register F - needed for instructions SCF and CCF
            regF = regs[rB] & (fX | fY | fS);               // copy flags X, Y and S from counter of bytes to transfer and reset the rest
            if( (sDI & 0x80) != 0 ) regF |= fN;             // copy flag N from bit 7 of tranfered byte

            if( type == bInc || type == bIncRep )           // if the instruction is INI or INIR
                ioq = sDI + ((regs[rC] + 1) & 0x00ff);      // calculation of variable ioq

            if( type == bDec || type == bDecRep )           // if the instruction is IND or INDR
                ioq = sDI + ((regs[rC] - 1) & 0x00ff);      // calculation of variable ioq

            if( (ioq & 0x0100) != 0 ) regF |= fC | fH;      // set flags C and H when the variable ioq overflow
            ioq = (ioq & 0x07) ^ regs[rB];                  // calculation of variable ioq

            if( parity(ioq) == false ) regF |= fP;          // set flag P if parity of variable ioq is even
            if( regs[rB] != 0) {                            // if counter of bytes to transfer is not zero then repeat instructions INIR or INDR or finish instruction INI or IND
                if( type == bIncRep || type == bDecRep ) {  // if the instruction is INIR or INDR then continue exucuting instruction
                    mc_busOp = oNone;                       // next bus op. is no operation
                    mc_TStates = 5;                         // set number of TStates for machine cycle 3
                }
            } else { regF |= fZ; mc_MCycles = 3; }          // if counter of bytes to transfer is zero then finish instruction
            break;

            /// Repeated below
            // Decrease program counter PC to first byte of this instrucion
    case 4: PC -= 2;                                     // set PC back to this instruction
            regF &= ~(fX | fY);                             // reset flags X and Y
            regF |= (PC >> 8) & (fX | fY);               // copy flags X and Y from PC
            flagsPH();// Calculation of flags P and H according to https://github.com/hoglet67/Z80Decoder/wiki/Undocumented-Flags
            sAO = PC;                                    // set address for address bus
    }
}

// OUTI, OUTD, OTIR, OTDR - block transfers from memory to I/O address
// This function is a tempate with parameter bInc, bIncRep, bDec or bDecRep which selects increase or decrease of memory address and repeatibility
template<Z80Core::eBlock type>
inline void Z80Core::otxx()
{
    uint16_t ioq;
    switch(sm_MCycle) {
            // Inicialization and decrease counter of bytes to transfer
    case 1: mc_busOp = oMemRead;                            // next bus op. is read byte from memory
            sAO = (regs[rH] << 8) | regs[rL];               // set address for address bus
            regs[rB]--;                                     // decrease counter of bytes to transfer
            regWZ = (regs[rB] << 8) | regs[rC];             // set memory pointer
            break;
            // Read byte from memory
    case 2: writeMem( (regs[rB] << 8) | regs[rC], sDI );    // next bus op. is write byte to I/O
            mc_TStates = 5;                                 // set number of TStates for machine cycle 3
            break;
            // Write byte to I/O address, increase/decrease register with memory address and check of termination of instruction
    case 3: if ( (type == bInc) || (type == bIncRep) ) {    // if the instruction is OUTI or OTIR
                inc_rr(regs[rH], regs[rL]);                 // increase memory address for transfer
                regWZ++;                                    // increase memory pointer
            }
            if ( (type == bDec) || (type == bDecRep) ) {    // if the instruction is OUTD or OTDR
                dec_rr(regs[rH], regs[rL]);                 // decrease memory address for transfer
                regWZ--;                                    // decrease memory pointer
            }
            regFChanged = true;                             // set flag change of register F - needed for instructions SCF and CCF
            regF = regs[rB] & (fX | fY | fS);               // copy flags X, Y and S from counter of bytes to transfer and reset the rest
            if ((sDI & 0x80) != 0) regF |= fN;              // copy flag N from bit 7 of tranfered byte
            ioq = sDI + regs[rL];                           // calculation of variable ioq

            if( (ioq & 0x0100) != 0 ) regF |= fC | fH;      // set flags C and H when the variable ioq overflow
            ioq = (ioq & 0x07) ^ regs[rB];                  // calculation of variable ioq

            if( parity(ioq) == false ) regF |= fP;          // set flag P if parity of variable ioq is even
            if (regs[rB] != 0) {                            // if counter of bytes to transfer is not zero then repeat instructions OTIR or OTDR or finish instruction OUTI or OUTD
                if( type == bIncRep || type == bDecRep ) {  // if the instruction is OTIR or OTDR then continue exucuting instruction
                    mc_busOp = oNone;                       // next bus op. is no operation
                    mc_TStates = 5;                         // set number of TStates for machine cycle 3
                }
            } else { regF |= fZ; mc_MCycles = 3; }          // if counter of bytes to transfer is zero then finish instruction
            break;

            /// Repeated above
            // Decrease program counter PC to first byte of this instrucion
    case 4: PC -= 2;                                     // set program counter PC back to this instruction
            regF &= ~(fX | fY);                             // reset flags X and Y
            regF |= (PC >> 8) & (fX | fY);               // copy flags X and Y from program counter PC
            flagsPH();// Calculation of flags P and H according to https://github.com/hoglet67/Z80Decoder/wiki/Undocumented-Flags
            sAO = PC;                                    // set address for address bus
    }
}

void Z80Core::flagsPH()// Calculation of flags P and H according to https://github.com/hoglet67/Z80Decoder/wiki/Undocumented-Flags
{
    if ((regF & fC) == 0) regF ^= ( (parity((regs[rB] + 1) & 0x07) == true) << 2 ) & fP;
    else {
        regF &= ~fH;
        uint8_t x = regs[rB] & 0x0f;
        if( sDI & 0x80 ) { if( x == 0x00 ) regF |= fH; }
        else             { if( x == 0x0f ) regF |= fH; }
        regF ^= ( (parity((regs[rB] + 1) & 0x07) == true) << 2 ) & fP;
    }
}

// PREFIX CB - switch to second instruction set
// If it is used without prefix IX or IY it only switches instruction set.
// It is is used with prefix IX or IY it is necessary to read offset byte (machine cycle 6) and folowed instruction op code (machine cycle 7)
// The machine cycle 7 is handled at individual instructions because the instruction set is changed during machine cycle 7
// and this function is not called, becuase it is from basic instruction set.
// M1 - set prefix CB and switch to machine cycle 6 in case of register IX or IY
// M6 - read offset from memory and calculate memory pointer WZ
void Z80Core::prefix_cb()
{
    switch(sm_MCycle) {
    case 1: mc_prefix = prefixCB;                           // set prefix flag
            if (XYState != rHL) {                           // test if it is used register IX or IY
                mc_StateMachine = sXYOffsetFetch;           // set address offset fetch flag
                mc_busOp = oMemRead;                        // next byte is read from memory
                PC++;                                    // increase program counter PC
                mc_MCycles = 2;                             // set number of machine cycles to 2 to prevent change TStates in function runMCode()
            }
            break;
            // Address offset fetch
    case 6: mc_prefix = prefixCB;                           // set prefix flag
            mc_busOp = oMemRead;                     // opcode followed offset is read from memory
            regWZ = ( (regs[XYState] << 8) | regs[XYState + 1] ) + static_cast<signed char>(sDI);   // calculation memory pointer
            mc_TStates = 5;                                 // set number of TStates for machine cycle 7
            break;
    }
}

void Z80Core::setProducer( QString producer )
{
    //int p = getEnumIndex( producer );
    //m_producer = (eProducer)p;
}
void Z80Core::setCmos( bool cmos ) { m_cmos = cmos; }// Setter for CMOS or NMOS version
void Z80Core::setIoWait( bool ioWait ) { m_ioWait = !ioWait; } // Setter for single wait I/O operation
void Z80Core::setIntVector( bool intVector ) { m_intVector = intVector; } // Setter for force interrupt vector 0xff

void Z80Core::writeMem( uint16_t addr, uint8_t data )
{
    mc_busOp = oMemWrite;                   // next bus op. is write byte to stack
    sAO = addr;                             // set address for address bus
    sDO = data;                             // set data for data bus
}

void Z80Core::flagsZP( uint8_t &reg )
{
    if( reg == 0 ) regF |= fZ;             // set flag Z if accumulator is zero
    if( parity(reg) == false ) regF |= fP; // set flag P if accumulator parity is even
    regFChanged = true;                     // set flag change of register F - needed for instructions SCF and CCF
}

void Z80Core::flagsCHP( uint8_t reg )
{
    if ( (reg & 0x80) != 0x00 ) regF |= fC;         // set flag C if borrow from 8. bit of high byte
    if ( (reg & 0x08) != 0x00 ) regF |= fH;         // set flag H if borrow from 4. bit of high byte
    if ( (reg & 0xC0) == 0x80 || (reg & 0xC0) == 0x40 )  // set flag P if 7. or 6. bit borrow, but not simultaneously
        regF |= fP;
    regFChanged = true;                             // set flag change of register F - needed for instructions SCF and CCF
}
