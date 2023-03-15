/***************************************************************************
 *   Copyright (C) 2022 by Santiago González                               *
 *                                                                         *
 ***( see copyright.txt file at root folder )*******************************/

#include <QDebug>

#include "mcs65core.h"
#include "e_mcu.h"
#include "simulator.h"
#include "ioport.h"
#include "watcher.h"

Mcs65Cpu::Mcs65Cpu( eMcu* mcu )
        : Mcs65Interface( mcu )
{
    // CPU registers to show in Monitor

    m_cpuRegs.insert("P" , &m_P  );
    m_cpuRegs.insert("S" , &m_SP );
    m_cpuRegs.insert("A" , &m_Ac );
    m_cpuRegs.insert("IR", &m_IR );
    m_cpuRegs.insert("X" , &m_rX );
    m_cpuRegs.insert("Y" , &m_rY );
    //m_rI = NULL;
    mcu->getCpuTable()->setRegisters( m_cpuRegs.keys() );

    // <register name="P"  addr="0x05" bits="C,Z,I,D,B,1,V,N" reset="00110100" mask="11011111" />
    m_STATUS = &m_P;
    mcu->setStatusBits({"C","Z","I","D","B","1","V","N"});

    // Buses
    m_dataBus = mcu->getIoPort("PORTD");
    m_addrBus = mcu->getIoPort("PORTA");
    m_rwPin   = mcu->getIoPin("RW");

    // Control Pins
    m_ctrlPort = mcu->getIoPort("CPORT0");
    m_phi0Pin = mcu->getIoPin("P0");
    m_phi1Pin = mcu->getIoPin("P1");   m_phi1Pin->setPinMode( output );
    m_phi2Pin = mcu->getIoPin("P2");   m_phi2Pin->setPinMode( output );
    m_syncPin = mcu->getIoPin("SYNC"); m_syncPin->setPinMode( output );

    // Interrupt Pins
    m_irqPin = mcu->getIoPin("IRQ");
    m_nmiPin = mcu->getIoPin("NMI");

    // User Pins
    m_rdyPin = mcu->getIoPin("RDY");
    m_soPin  = mcu->getIoPin("SO");
    /// m_dbePin = McuPort::getPin("DBE"););

    /*mcu->component()->addPropGroup( {"Cpu", {
new BoolProp<Mcu>( "Ext_Osc", tr("External Clock"),"", this, &Mcu::extOscEnabled, &Mcu::enableExtOsc ),
     }} );*/
}
Mcs65Cpu::~Mcs65Cpu() {}

QString Mcs65Cpu::getStrReg( QString reg ) // Called by Mcu Monitor to get String values
{
    QString value = "";

    if( reg == "Instruction") value = getStrInst( m_IR );


    return value;
}

void Mcs65Cpu::reset()
{
    CpuBase::reset();

    *m_STATUS = 0b00110100; // Status

    m_state = cRESET;
    m_EXEC = NULL;
    m_debugPC = 0;
    m_cycle = 0;
    m_SP = 0;

    m_IsrH = 0xFF;
    m_IsrL = 0;

    m_dataMode = input;
    m_nextClock = true;
    m_halt = false;

    stamp();
}

void Mcs65Cpu::stamp()
{
    m_dataBus->reset();
    m_addrBus->reset();
    m_ctrlPort->reset();

    m_addrBus->setPinMode( output );
    m_dataBus->setPinMode( input );

    m_rwPin->setPinMode( output );
    m_rwPin->setOutState( true );

    m_phi1Pin->setPinMode( output );
    m_phi2Pin->setPinMode( output );
    m_syncPin->setPinMode( output );

    m_irqPin->setPinMode( input );
    m_nmiPin->setPinMode( input );

    // User Pins
    m_rdyPin->setPinMode( input );
    m_soPin->setPinMode( input );
}

void Mcs65Cpu::runEvent()
{
    if( m_nextClock ) // Clock Falling delayed: Set Address Bus
    {
        m_addrBus->setOutState( m_busAddr );

        if( m_dataMode == output ){
            m_dataMode = input;
            m_dataBus->setPinMode( input ); // Data Bus to input if last Write
        }
    }else            // Clock Rising delayed: Set Data Bus
    {
        if( m_dataMode == input ){
            m_dataMode = output;
            m_dataBus->setPinMode( output );
        }
        m_dataBus->setOutState( m_op0 );
    }
}

void Mcs65Cpu::extClock( bool clkState )
{
    if( clkState == m_nextClock ) runStep();
}

void Mcs65Cpu::runStep()
{
    if( m_nextClock ) clkRisingEdge();
    else              clkFallingEdge();

    m_phi2Pin->sheduleState( m_nextClock, 5000 );
    m_nextClock = !m_nextClock;
}

void Mcs65Cpu::clkRisingEdge()
{
    if( m_halt ) return;
    if( !m_soPin->getInpState() ) SET_OVERFLOW( 1 );

    if( m_state != cWRITE ) return;
    m_state = m_nextState;

    Simulator::self()->addEvent( m_tHW, this ); // Set Data Port
}

void Mcs65Cpu::clkFallingEdge()
{
     m_halt = !m_rdyPin->getInpState();
    if( m_halt ) return;

    m_mcu->cyclesDone = 1;
    m_cycle++;

    if( m_state == cRESET )  // Reset Sequence: 8 cycles, then state changes to FETCH
    {
        resetSeq();
        if( m_state == cREAD ) { m_state = cRESET; return; } // Keep sequence until cFETCH
    }
    if( m_state == cDECODE )  // Decode Instruction, then state changes to READ
    {
        //m_debugPC = m_PC; // Don't update m_debugPC until last Instruction fully executed

        m_syncPin->sheduleState( false, m_tHA ); // Reset SYNC Signal
        m_IR = readDataBus();
        if( !m_IsrL && !m_nmiPin->getInpState() ) // NMI
        {
            m_IsrL = 0xFA;
            m_EXEC = &Mcs65Cpu::BRK;
        }
        else if( !STATUS(I) && !m_IsrL && !m_irqPin->getInpState() ) // IRQ
        {
            m_IsrL = 0xFE;
            m_EXEC = &Mcs65Cpu::BRK;
        }
        else decode();
    }
    if( m_state == cREAD ) // Read Operand: When all Operands ready state changes to EXEC
    {
        Read();
    }
    if( m_state == cEXEC )         // Execute Instruction: when finished state changes to FETCH
    {
        m_state = cFETCH;       // Default: Execute in 1 step, Instructions can add Execute cycles

        if( m_EXEC ) (this->*m_EXEC)();
        //else qDebug() << "ERROR: Instruction not implemented: 0x"+QString::number( m_IR, 16 ).toUpper(); //
    }
    if( m_state == cWRITE ) m_rwPin->sheduleState( false, m_tHA ); // Write result and fetch at next cycle //m_busAddr = m_opAddr Done in instruction
    else{
        m_rwPin->sheduleState( true, m_tHA );

        if( m_state == cFETCH )  // If no Write op. fetch next inst. at execute cycle
        {
            if( m_aMode <= aACCU ) m_PC--;           // If dummy Read then read the same PC
            m_debugPC = m_PC;
            readPGM();                               // Read opCode
            m_cycle = 0;
            m_state = cDECODE;

            m_syncPin->sheduleState( true, m_tHA );  // Set SYNC Signal
        }
    }
}

void Mcs65Cpu::resetSeq() // Reset Sequence: 8 cycles (including fetch first instruction)
{
    switch( m_cycle ){
        case 1:                    return;
        case 2: readMem( 0x0001 ); return;
        case 3: readMem( 0x0100 ); return;
        case 4:                    return;
        case 5:                    return;
        case 6: readMem( 0XFFFC ); return;
        case 7: readMem( 0XFFFD ); m_op0 = readDataBus(); return;
        case 8:{
            uint32_t data = readDataBus();
            m_PC = m_op0 | (data << 8);
            m_state = cFETCH;         // Reset sequence finished, Fetch first instruction.
            m_aMode = aIMME;          // Avoid PC decrement at cFETCH case
}   }   }

void Mcs65Cpu::decode()
{
    m_state = cREAD;    // Read next operand in any case
    m_aMode = aNONE;
    m_aFlags = 0;
    m_EXEC = NULL;

    switch( m_IR )                     // Irregular Instructions
    {
        case 0x00: m_EXEC = &Mcs65Cpu::BRK; m_IsrL = 0xFE;   return; // BRK
        case 0x08: m_EXEC = &Mcs65Cpu::PHP;                  return; // PHP
        case 0x18: m_EXEC = &Mcs65Cpu::CLC; return; // CLC
        case 0x20: m_EXEC = &Mcs65Cpu::JSR; m_aMode = aABSO; return; // JSR abs Execution controls PC
        case 0x28: m_EXEC = &Mcs65Cpu::PLP;                  return; // PLP
        case 0x38: m_EXEC = &Mcs65Cpu::SEC; return; // SEC
        case 0x40: m_EXEC = &Mcs65Cpu::RTI;                  return; // RTI
        case 0x48: m_EXEC = &Mcs65Cpu::PHA; return; // PHA
        case 0x58: m_EXEC = &Mcs65Cpu::CLI; return; // CLI
        case 0x60: m_EXEC = &Mcs65Cpu::RTS;                  return; // RTS
        case 0x68: m_EXEC = &Mcs65Cpu::PLA;                  return; // PLA
        case 0x78: m_EXEC = &Mcs65Cpu::SEI; return; // SEI
        case 0x88: m_EXEC = &Mcs65Cpu::DEY; return; // DEY
        case 0x8A: m_EXEC = &Mcs65Cpu::TXA; return; // TXA
        case 0x98: m_EXEC = &Mcs65Cpu::TYA; return; // TYA
        case 0x9A: m_EXEC = &Mcs65Cpu::TXS; return; // TXS
        case 0xA8: m_EXEC = &Mcs65Cpu::TAY; return; // TAY
        case 0xAA: m_EXEC = &Mcs65Cpu::TAX; return; // TAX
        case 0xB8: m_EXEC = &Mcs65Cpu::CLV; return; // CLV
        case 0xBA: m_EXEC = &Mcs65Cpu::TSX; return; // TSX
        case 0xC8: m_EXEC = &Mcs65Cpu::INY; return; // INY
        case 0xCA: m_EXEC = &Mcs65Cpu::DEX; return; // DEX
        case 0xD8: m_EXEC = &Mcs65Cpu::CLD; return; // CLD
        case 0xE8: m_EXEC = &Mcs65Cpu::INX; return; // INX
        case 0xEA: m_EXEC = &Mcs65Cpu::NOP; return; // NOP
        case 0xF8: m_EXEC = &Mcs65Cpu::SED; return; // SED
    }
    /// A big switch case would be faster (with setters for Addressing Mode)
    // Regular Instructions:             OOOAAAGG , O = Opcode, A = Addr mode, G = Group
    uint8_t group =  m_IR & 0b00000011;
    uint8_t Ocode = (m_IR & 0b11100000) >> 5;
    uint8_t Atype = (m_IR & 0b00011100) >> 2;

    switch( Atype ){                // Set Addressing Mode
    case 0: {
        if( group == 1 ){ m_aMode = aINDI; m_aFlags = iX; break; } // Group 1
        else{             m_aMode = aIMME;                break; } // Groups 2, 0
    }
    case 1:               m_aMode = aZERO;                break;   // All
    case 2: {
        if( group == 1 ){ m_aMode = aIMME;                break; } // Group 1
        if( group == 2 ){ m_aMode = aACCU;                break; } // Group 2
    }
    case 3:               m_aMode = aABSO;                break;   // All
    case 4:
        if( group == 0 ){ m_aMode = aIMME;                break; } // Group 0 Conditional Branch
        if( group == 1 ){ m_aMode = aINDI; m_aFlags = iY; break; } // Group 1

    case 5:               m_aMode = aZEDX;                break;   // same
    case 6:
        if( group == 1 ){ m_aMode = aABSO; m_aFlags = iY; break; } // Group 1

    case 7:               m_aMode = aABSO; m_aFlags = iX; break;   // All
    }
    switch( group ){               // Set Executable
        case 0:{
        if( Atype == 4 ) // xxy 100 00 Conditional Branch
        {
            m_u8Tmp0 = (m_IR & 0b11000000) >> 6; // flag: 0=N, 1=O, 2=C, 3=Z
            m_u8Tmp1 = (m_IR & 0b00100000) >> 5; // y
            m_EXEC = &Mcs65Cpu::BXX; // BPL BMI BVC BVS BCC BCS BNE BEQ
        }else{
            switch( Ocode ){
                case 0: break;
                case 1: m_EXEC = &Mcs65Cpu::BIT;; break;
                case 2:                   //JMP;
                case 3: m_EXEC = &Mcs65Cpu::JMP; break;
                case 4: m_EXEC = &Mcs65Cpu::STY; break;
                case 5: m_EXEC = &Mcs65Cpu::LDY;; break;
                case 6: m_EXEC = &Mcs65Cpu::CPY;; break;
                case 7: m_EXEC = &Mcs65Cpu::CPX;; break;
        } } } break;
        case 1:{ switch( Ocode ){
                case 0: m_EXEC = &Mcs65Cpu::ORA;; break;
                case 1: m_EXEC = &Mcs65Cpu::AND;; break;
                case 2: m_EXEC = &Mcs65Cpu::EOR;; break;
                case 3: m_EXEC = &Mcs65Cpu::ADC;; break;
                case 4: m_EXEC = &Mcs65Cpu::STA; break;
                case 5: m_EXEC = &Mcs65Cpu::LDA;; break;
                case 6: m_EXEC = &Mcs65Cpu::CMP;; break;
                case 7: m_EXEC = &Mcs65Cpu::SBC;; break;
        } }break;
        case 2:{ switch( Ocode ){
                case 0: m_EXEC = &Mcs65Cpu::ASL;; break;
                case 1: m_EXEC = &Mcs65Cpu::ROL;; break;
                case 2: m_EXEC = &Mcs65Cpu::LSR;; break;
                case 3: m_EXEC = &Mcs65Cpu::ROR;; break;
                case 4: m_EXEC = &Mcs65Cpu::STX; break;
                case 5: m_EXEC = &Mcs65Cpu::LDX;; break;
                case 6: m_EXEC = &Mcs65Cpu::DEC;; break;
                case 7: m_EXEC = &Mcs65Cpu::INC;; break;
        } }break;
        case 3: { break; }  /// Not valid (by now)
    }
    if( !m_EXEC )
        qDebug() << "ERROR: Instruction not implemented: 0x"
                    +QString::number( m_IR, 16 ).toUpper()
                 << "PC:0x"+QString::number( m_PC, 16 ).toUpper();
}

void Mcs65Cpu::Read()
{
    if( m_cycle == 1 ){ readPGM(); return; } // Always Read 1º Operand
    m_state = cEXEC; // Default

    switch( m_aMode ){
        case aNONE:                        break;/// Dummy Read
        case aACCU: m_op0 = m_Ac;          break;/// Value = m_Acumulator
        case aIMME: m_op0 = readDataBus(); break;/// Immediate: From Mem 1 byte
        case aABSO:{                             /// Absolute: address From Mem 2 bytes: LLHH (Little Endian)
            switch( m_cycle ){
                case 2:{
                    m_opAddr = readDataBus();     // Read addrL
                    readPGM();                    // Read 2º Operand
                }break;
                case 3:{
                    if( m_aFlags )               /// Absolute Indexed
                    {
                        if     ( m_aFlags & iX ) m_opAddr += m_rX;
                        else if( m_aFlags & iY ) m_opAddr += m_rY;
                        if( m_opAddr > 255 ){
                            m_opAddr &= 0xFF;     // clear High byte
                            m_aFlags |= iC;       // carry
                        }
                    }
                    m_opAddr |= (readDataBus() << 8); // Read addrH
                    readMem( m_opAddr );
                }break;
                case 4:{
                    if( m_aFlags & iC ){          // Add carry and read
                        m_opAddr += 256;
                        readMem( m_opAddr );
                    }
                    else m_op0 = readDataBus();   // Read operation finished
                }break;
                case 5: m_op0 = readDataBus();
            }
        }break;
        case aZERO:{                             /// Zero Page
            switch( m_cycle ){
                case 2:{
                    m_opAddr = readDataBus();     // Read addrL
                    readMem( m_opAddr );          // Read 2º Operand
                }break;
                case 3: m_op0 = readDataBus();    // Finish read operation
            }
        }break;
        case aZEDX:{                             /// Zero Page indexed
            switch( m_cycle ){
                case 2:{
                    m_u8Tmp0 = readDataBus();
                    readMem( m_u8Tmp0 );
                }break;
                case 3:{             // Discard reading
                    if     ( m_aFlags & iX ) m_u8Tmp0 += m_rX;
                    else if( m_aFlags & iY ) m_u8Tmp0 += m_rY;
                    m_opAddr = m_u8Tmp0;
                    readMem( m_opAddr );
                }break;
                case 4: m_op0 = readDataBus();    // Finish read operation
            }
        }break;
        case aINDI:{                             /// Indirect: 1 byte from Mem + RX(ZP wrap) or RY
            if( m_aFlags & iX ){                 /// Indirect Pre-indexed
                switch( m_cycle ){
                    case 2:{
                        m_u8Tmp0 = readDataBus();
                        readMem( m_u8Tmp0 );          // Read addrL
                    }break;
                    case 3:{
                        m_u8Tmp0 += m_rX;             // Read addrL+X
                        readMem( m_u8Tmp0 );
                    }break;
                    case 4:{
                        m_u8Tmp0 = readDataBus()+1;
                        readMem( m_u8Tmp0 );          // Read addrH
                    }break;
                    case 5:{
                        m_opAddr = m_u8Tmp0 | readDataBus()<<8;
                        readMem( m_opAddr );
                    }break;
                    case 6: m_op0 = readDataBus();    // Finish read operation
                }
            }else{                                   /// Indirect Post-indexed
                switch( m_cycle ){
                    case 2:{
                        m_u8Tmp0 = readDataBus();
                        readMem( m_u8Tmp0 );          // Read addrL
                    }break;
                    case 3:{
                        m_u8Tmp1 = readDataBus();     // addrL
                        readMem( m_u8Tmp0+1 );        // Read addrH
                    }break;
                    case 4:{
                        m_opAddr = m_u8Tmp1 + m_rY;   // addrL+Y
                        if( m_opAddr > 255 ){
                            m_opAddr &= 0xFF;         // clear High byte
                            m_aFlags |= iC;           // carry
                        }
                        m_opAddr |= readDataBus()<<8; // Read addrH
                        readMem( m_opAddr );
                    }break;
                    case 5:{
                        if( m_aFlags & iC ){          // Add carry and read
                            m_opAddr += 256;
                            readMem( m_opAddr );
                        }
                        else m_op0 = readDataBus();   // Read operation finished
                    }break;
                    case 6: m_op0 = readDataBus();    // Finish read operation
                }
            }
        }
    }
}

void Mcs65Cpu::readPGM(){ readMem( m_PC++ ); }

void Mcs65Cpu::readMem( uint16_t addr )
{
    m_busAddr = addr;
    m_state = cREAD;
    Simulator::self()->addEvent( m_tHA, this ); // Buses managed at runEvent()
}

uint8_t Mcs65Cpu::readDataBus() { return m_dataBus->getInpState(); }

void Mcs65Cpu::writeMem( uint16_t addr ) {
    m_busAddr = addr; m_state = cWRITE; m_nextState = cFETCH;
    Simulator::self()->addEvent( m_tHA, this ); // Buses managed at runEvent()
}

void Mcs65Cpu::pushStack8( uint8_t byte ) { m_op0 = byte; writeMem( 0x0100 + m_SP-- ); }

void Mcs65Cpu::popStack8() { readMem( 0x0100 + (++m_SP) ); /*m_state = cEXEC;*/ }

void Mcs65Cpu::setNZ( uint8_t val ) { SET_NEGATIVE( val & 0x80 ); SET_ZERO( val == 0 ); }

void Mcs65Cpu::writeWflags( bool c )
{
    SET_CARRY( c );
    setNZ( m_op0 );
    if( m_aMode == aACCU ) m_Ac = m_op0;         // Write to Acc
    else                   writeMem( m_opAddr ); // Write to MEM
}

void Mcs65Cpu::ADC()
{
    uint8_t m = m_op0;
    uint tmp = m + m_Ac + (STATUS(C) ? 1 : 0);
    SET_ZERO( (tmp & 0xFF) == 0 );

    if( STATUS(D) ){
        if( ((m_Ac & 0xF) + (m & 0xF) + (STATUS(C) ? 1 : 0)) > 9) tmp += 6;

        SET_NEGATIVE( tmp & 0x80 );
        SET_OVERFLOW( !((m_Ac ^ m) & 0x80) && ((m_Ac ^ tmp) & 0x80) );

        if( tmp > 0x99) tmp += 96;
        SET_CARRY(tmp > 0x99);
    }else{
        SET_NEGATIVE( tmp & 0x80 );
        SET_OVERFLOW(!((m_Ac ^ m) & 0x80) && ((m_Ac ^ tmp) & 0x80));
        SET_CARRY(tmp > 0xFF);
    }
    m_Ac = tmp & 0xFF;
}

void Mcs65Cpu::AND() { m_Ac &= m_op0; setNZ( m_Ac ); }

void Mcs65Cpu::ASL()
{
    bool carry = m_op0 & 0x80;
    m_op0 <<= 1;
    writeWflags( carry );
}

void Mcs65Cpu::BIT()
{
    uint8_t res = m_op0 & m_Ac;
    SET_NEGATIVE( res );
    *m_STATUS = (*m_STATUS & 0x3F) | (m_op0 & 0xC0) | CONSTANT | BREAK;
    SET_ZERO( res == 0 );
}

void Mcs65Cpu::BRK() // Break/Interrupt : 1 byte, 7 cycles
{
    SET_INTERRUPT( 1 ); /// ???
    switch( m_cycle ){
        case 2: pushStack8( m_PC >> 8 );       m_nextState = cEXEC; break; // PCH    -> Stack
        case 3: pushStack8( m_PC /*& 0xFF*/ ); m_nextState = cEXEC; break; // PCL    -> Stack
        case 4: pushStack8( *m_STATUS );       m_nextState = cEXEC; break; // Status -> Stack
        case 5: readMem( m_IsrH );                 m_state = cEXEC; break; // Read IsrH
        case 6: readMem( m_IsrL ); m_u8Tmp0 = m_op0; m_state = cEXEC; break; // Read IsrL
        case 7: m_PC = (m_u8Tmp0 << 8) + m_op0; m_IsrL = 0;                  // Jump to Isr
}   }

void Mcs65Cpu::CLC() { SET_CARRY(0);     }
void Mcs65Cpu::CLD() { SET_DECIMAL(0);   }
void Mcs65Cpu::CLI() { SET_INTERRUPT(0); }
void Mcs65Cpu::CLV() { SET_OVERFLOW(0);  }

void Mcs65Cpu::CMP() { SET_CARRY( m_Ac >= m_op0 ); setNZ( m_Ac - m_op0 ); }
void Mcs65Cpu::CPX() { SET_CARRY( m_rX >= m_op0 ); setNZ( m_rX - m_op0 ); }
void Mcs65Cpu::CPY() { SET_CARRY( m_rY >= m_op0 ); setNZ( m_rY - m_op0 ); }

void Mcs65Cpu::DEC() { m_op0 -= 1; setNZ( m_op0 ); writeMem( m_opAddr ); }
void Mcs65Cpu::DEX() { m_rX  -= 1; setNZ( m_rX ); }
void Mcs65Cpu::DEY() { m_rY  -= 1; setNZ( m_rY ); }

void Mcs65Cpu::EOR() { m_Ac = m_Ac ^ m_op0; setNZ( m_Ac ); }

void Mcs65Cpu::INC() { m_op0 += 1; setNZ( m_op0 ); writeMem( m_opAddr ); }
void Mcs65Cpu::INX() { m_rX  += 1; setNZ( m_rX ); }
void Mcs65Cpu::INY() { m_rY  += 1; setNZ( m_rY ); }

void Mcs65Cpu::JMP() { m_PC = m_opAddr; }
void Mcs65Cpu::JSR()
{
    switch( m_cycle ){
        case 4: pushStack8( m_PC >> 8 );   m_nextState = cEXEC; break;  // PCH -> Stack
        case 5: pushStack8( m_PC & 0xFF ); m_nextState = cEXEC; break;  // PCL -> Stack
        case 6: m_PC = m_opAddr;                                        // Jump to Subroutine
}   }

void Mcs65Cpu::LDA() { m_Ac = m_op0; setNZ( m_Ac ); }
void Mcs65Cpu::LDX() { m_rX = m_op0; setNZ( m_rX ); }
void Mcs65Cpu::LDY() { m_rY = m_op0; setNZ( m_rY ); }

void Mcs65Cpu::LSR()
{
    bool carry = m_op0 & 1;
    m_op0 >>= 1;
    writeWflags( carry );
}

void Mcs65Cpu::NOP() { return; }

void Mcs65Cpu::ORA() { m_Ac |= m_op0; setNZ( m_Ac ); } // OR ACCU

void Mcs65Cpu::PHA() { pushStack8( m_Ac ); }
void Mcs65Cpu::PHP() { pushStack8( STATUS(C) | STATUS(B) ); }

void Mcs65Cpu::PLA()
{
    if( m_cycle == 2 ) popStack8();
    else             { m_Ac = readDataBus(); setNZ( m_Ac ); }
}

void Mcs65Cpu::PLP()
{
    if( m_cycle == 2 ) popStack8();
    else              { *m_STATUS = readDataBus() | CONSTANT | BREAK;  }
}

void Mcs65Cpu::ROL()
{
    bool carry = m_op0 & 0x80;
    m_op0 <<= 1;
    if( STATUS(C) ) m_op0 |= 0x01;
    writeWflags( carry );
}

void Mcs65Cpu::ROR()
{
    bool carry = m_op0 & 1;
    m_op0 >>= 1;
    if( STATUS(C) ) m_op0 |= 0x80;
    writeWflags( carry );
}

void Mcs65Cpu::RTI() // Return from Interrupt ////
{
    switch( m_cycle ){
        case 2: popStack8();                                               break;
        case 3: popStack8(); *m_STATUS = readDataBus() | CONSTANT | BREAK; break;
        case 4: popStack8(); m_op0 = readDataBus();                        break;
        case 5: m_PC = (readDataBus() << 8) | m_op0; m_aMode = aIMME; // Avoid PC decrement at cFETCH case
}   }

void Mcs65Cpu::RTS() // Return from Subroutine
{
    switch( m_cycle ){
        case 2: popStack8();                        break;
        case 3: popStack8(); m_op0 = readDataBus(); break;
        case 4: m_PC = (readDataBus() << 8) | m_op0; m_aMode = aIMME; // Avoid PC decrement at cFETCH case
}   }

void Mcs65Cpu::SBC()
{
    uint8_t m = m_op0;
    uint tmp = m_Ac - m - (STATUS(C) ? 0 : 1);
    SET_NEGATIVE( tmp & 0x80 );
    SET_ZERO( (tmp & 0xFF) == 0 );
    SET_OVERFLOW( ((m_Ac ^ tmp) & 0x80) && ((m_Ac ^ m) & 0x80) );

    if( STATUS(D) ){
        if( ((m_Ac & 0x0F) - (STATUS(C) ? 0 : 1)) < (m & 0x0F)) tmp -= 6;
        if( tmp > 0x99) tmp -= 0x60;
    }
    SET_CARRY( tmp < 0x100 );
    m_Ac = ( tmp /*& 0xFF*/ );
}

void Mcs65Cpu::SEC() { SET_CARRY(1); }
void Mcs65Cpu::SED() { SET_DECIMAL(1); }
void Mcs65Cpu::SEI() { SET_INTERRUPT(1); }

void Mcs65Cpu::STA() { m_op0 = m_Ac; writeMem( m_opAddr ); }
void Mcs65Cpu::STX() { m_op0 = m_rX; writeMem( m_opAddr ); }
void Mcs65Cpu::STY() { m_op0 = m_rY; writeMem( m_opAddr ); }

void Mcs65Cpu::TAX() { m_rX = m_Ac; setNZ( m_rX ); }
void Mcs65Cpu::TAY() { m_rY = m_Ac; setNZ( m_rY ); }
void Mcs65Cpu::TXA() { m_Ac = m_rX; setNZ( m_Ac ); }
void Mcs65Cpu::TYA() { m_Ac = m_rY; setNZ( m_Ac ); }
void Mcs65Cpu::TSX() { m_rX = m_SP; setNZ( m_rX ); }
void Mcs65Cpu::TXS() { m_SP = m_rX; }

void  Mcs65Cpu::BXX() /// +1 cycle same page +2 different page
{                         // m_u8Tmp0 = Flag , m_u8Tmp1 = y
    uint8_t bit = 0;
    switch( m_u8Tmp0 ) {
        case 0: bit = N; break;
        case 1: bit = O; break;
        case 2: bit = C; break;
        case 3: bit = Z; break;
    }
    uint8_t flagVal = STATUS( bit ) ? 1 : 0;
    if( flagVal == m_u8Tmp1 ) m_PC += (int8_t)m_op0;
}
