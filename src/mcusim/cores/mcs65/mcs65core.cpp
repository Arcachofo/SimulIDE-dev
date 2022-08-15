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

//#include <QDebug>

#include "mcs65core.h"
#include "extmem.h"

Mcs65Core::Mcs65Core( eMcu* mcu )
         : McuCore( mcu )
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

    // Control Pins
    m_phi0Pin = mcu->getPin("P0");
    m_phi1Pin = mcu->getPin("P1");   m_phi1Pin->setPinMode( output );
    m_phi2Pin = mcu->getPin("P2");   m_phi2Pin->setPinMode( output );
    m_syncPin = mcu->getPin("SYNC"); m_syncPin->setPinMode( output );
    // Interrupt Pins
    m_irqPin = mcu->getPin("IRQ");
    m_nmiPin = mcu->getPin("NMI");
    // User Pins
    m_rdyPin = mcu->getPin("RDY");
    m_soPin  = mcu->getPin("SO");
    /// m_dbePin = McuPort::getPin("DBE"););
}
Mcs65Core::~Mcs65Core() {}

void Mcs65Core::reset()
{
    McuCore::reset();

    m_P = 0b00110100;
    m_cycle = 0;
    m_incPC = false;
    m_cpuState = cpu_RESET;

    m_phi1Pin->controlPin( true, true );
    m_phi2Pin->controlPin( true, true );
    m_syncPin->controlPin( true, true );

    m_phi1Pin->setDirection( true );
    m_phi2Pin->setDirection( true );
    m_syncPin->setDirection( true );

    m_psStep = m_mcu->psCycle(); // Full Clock cycle duration in ps
    m_tP0   = 10*1e3;            // 10 ns
    m_tSync = m_tP0*2; //35*m_psStep/100;
    //m_tMds  = 150*m_psStep/100;
    //m_tAds  = 100*1e3;           // 100 ns
    m_tAcc  = 55*m_psStep/100;   //

    m_mcu->extMem->setAddrSetTime(  m_tP0*2 );  // Addr pins: 100 ns
    m_mcu->extMem->setReadSetTime(  m_tP0*2+m_tAcc );  //
    m_mcu->extMem->setWriteSetTime( m_psStep/2);  //
    m_mcu->extMem->setReadBusTime(  m_psStep );  //
}

void Mcs65Core::setNZ( uint8_t val )
{
    SET_NEGATIVE( val & 0x80 );
    SET_ZERO( val == 0 );
}

void Mcs65Core::Read( uint16_t addr )
{
    m_cpuState = cpu_EXEC; // Default

    if( m_cycle == 1 ) { readMem( addr ); } // Read first operand
    else{
        switch( m_addrMode ){
            case a_NON: break; // Instruction with no operand
            case a_ABS:{       // Absolute: address From Mem 2 bytes: LLHH
                switch( m_cycle ){
                    case 2: m_op0 = readDataBus(); readMem( addr );   break; //
                    case 3:{
                        m_opAddr = (readDataBus() << 8) | m_op0 ;
                        if( m_addrIndx ){
                            if( m_addrIndx & i_C ) if( STATUS(C) ) m_opAddr++;
                            if( m_addrIndx & i_X ) m_opAddr += m_rX;
                            if( m_addrIndx & i_Y ) m_opAddr += m_rY;
                            readMem( m_opAddr );
                        }
                    }break;
                    case 4: m_op0 = readDataBus(); break;
                }
            }break;
            case a_ACC: m_op0 = m_Ac;         break; // Value = m_Acumulator
            case a_IMM: m_op0 = readDataBus(); break; // Immediate: From Mem 1 byte
            case a_IND: break;
            case a_REL: break;
    }   }
}

void Mcs65Core::readMem( uint16_t addr )
{
    m_cpuState = cpu_READ;
    m_mcu->extMem->read( addr );
    m_incPC = m_addrMode > a_ACC;
}

uint8_t Mcs65Core::readDataBus()
{
    uint8_t  data = m_mcu->extMem->getData();
    //uint16_t addr = m_mcu->extMem->getAddress();
    //m_progMem[addr] = data;          // Used by mpu Monitor
    return data;
}

void Mcs65Core::Write( uint16_t addr, uint8_t val )
{
    switch( m_addrMode )
    {
        //case a_NON:              break; /// ERROR
        case a_ACC: m_Ac = val; break;
        default:
            m_mcu->extMem->write( addr, val );
            //m_progMem[addr] = val;     // Used by mpu Monitor
            m_cpuState = cpu_WRITE;
}   }

void Mcs65Core::pushStack8( uint8_t byte ) { Write( 0x0100 + m_SP, byte ); m_SP -= 1; }
void Mcs65Core::popStack8()                { m_SP += 1; readMem( 0x0100 + m_SP ); m_cpuState = cpu_EXEC; }

void Mcs65Core::ADC()
{
    uint8_t m = m_op0;
    unsigned int tmp = m + m_Ac + (STATUS(C) ? 1 : 0);
    SET_ZERO( (tmp & 0xFF) == 0 );

    if( STATUS(D) )
    {
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

void Mcs65Core::AND() { m_Ac &= m_op0; setNZ( m_Ac ); }

void Mcs65Core::ASL()
{
    SET_CARRY( m_op0 & 0x80 );
    m_op0 <<= 1;
    setNZ( m_op0 );
    Write( m_opAddr, m_op0 );
}

void Mcs65Core::BIT()
{
    uint8_t res = m_op0 & m_Ac;
    SET_NEGATIVE( res );
    *m_STATUS = (*m_STATUS & 0x3F) | (m_op0 & 0xC0) | CONSTANT | BREAK;
    SET_ZERO( res == 0 );
}

void Mcs65Core::BRK() // Break/Interrupt : 1 byte, 7 cycles
{SET_INTERRUPT( 1 );
    switch( m_cycle ){
        case 2: pushStack8( ++PC >> 8 ); m_cpuState = cpu_EXEC; break; // PCH    -> Stack
        case 3: pushStack8( PC & 0xFF ); m_cpuState = cpu_EXEC; break; // PCL    -> Stack
        case 4: pushStack8( *m_STATUS ); m_cpuState = cpu_EXEC; break; // Status -> Stack
        case 5: Read( m_IsrH );          m_cpuState = cpu_EXEC; break; // Read IsrH
        case 6: Read( m_IsrL ); m_op1 = m_op0; m_cpuState = cpu_EXEC; break; // Read IsrL
        case 7: PC = (m_op1 << 8) + m_op0;            // Jump to Isr
}   }

void Mcs65Core::CLC() { SET_CARRY(0);     }
void Mcs65Core::CLD() { SET_DECIMAL(0);   }
void Mcs65Core::CLI() { SET_INTERRUPT(0); }
void Mcs65Core::CLV() { SET_OVERFLOW(0);  }

void Mcs65Core::CMP() { SET_CARRY( m_Ac >= m_op0 ); setNZ( m_Ac - m_op0 ); }
void Mcs65Core::CPX() { SET_CARRY( m_rX >= m_op0 ); setNZ( m_rX - m_op0 ); }
void Mcs65Core::CPY() { SET_CARRY( m_rY >= m_op0 ); setNZ( m_rY - m_op0 ); }

void Mcs65Core::DEC() { m_op0 -= 1; setNZ( m_op0 );  Write( m_opAddr, m_op0 ); }
void Mcs65Core::DEX() { m_rX  -= 1; setNZ( m_rX ); }
void Mcs65Core::DEY() { m_rY  -= 1; setNZ( m_rY ); }

void Mcs65Core::EOR() { m_Ac = m_Ac ^ m_op0; setNZ( m_op0 ); }

void Mcs65Core::INC() { m_op0 += 1; setNZ( m_op0 ); Write( m_opAddr, m_op0 ); }
void Mcs65Core::INX() { m_rX  += 1; setNZ( m_rX ); }
void Mcs65Core::INY() { m_rY  += 1; setNZ( m_rY ); }

void Mcs65Core::JMP() { PC = m_opAddr; }

void Mcs65Core::JSR()
{
    switch( m_cycle ){
        case 3: pushStack8( (PC+1) >> 8 );   m_cpuState = cpu_EXEC; break;  // PCH -> Stack
        case 4: pushStack8( (PC+1) & 0xFF ); m_cpuState = cpu_EXEC; break;  // PCL -> Stack
        case 5: PC = m_opAddr;                                              // Jump to Subroutine
}   }

void Mcs65Core::LDA() { m_Ac = m_op0; setNZ( m_op0 ); }
void Mcs65Core::LDX() { m_rX = m_op0; setNZ( m_op0 ); }
void Mcs65Core::LDY() { m_rY = m_op0; setNZ( m_op0 ); }

void Mcs65Core::LSR()
{
    SET_CARRY( m_op0 & 0x01 );
    m_op0 >>= 1;
    setNZ( m_op0 );
    Write( m_opAddr, m_op0 );
}

void Mcs65Core::NOP() { return; }

void Mcs65Core::ORA() { m_Ac |= m_op0; setNZ( m_Ac ); }

void Mcs65Core::PHA() { pushStack8( m_Ac ); }
void Mcs65Core::PHP() { pushStack8( STATUS(C) | STATUS(B) ); }

void Mcs65Core::PLA()
{
    if( m_cycle == 2 ) popStack8();
    else             { m_Ac = readDataBus(); setNZ( m_Ac ); PC++;}
}

void Mcs65Core::PLP()
{
    if( m_cycle == 2 ) popStack8();
    else              { *m_STATUS = readDataBus() | CONSTANT | BREAK; PC++; }
}

void Mcs65Core::ROL()
{
    bool carry = m_op0 & 0x80;
    m_op0 <<= 1;
    if( STATUS(C) ) m_op0 |= 0x01;
    SET_CARRY( carry );
    setNZ( m_op0 );
    Write( m_opAddr, m_op0 );
}

void Mcs65Core::ROR()
{
    bool carry = m_op0 & 1;
    m_op0 >>= 1;
    if( STATUS(C)  ) m_op0 |= 0x80;
    SET_CARRY( carry );
    setNZ( m_op0 );
    Write( m_opAddr, m_op0 );
}

void Mcs65Core::RTI() // Return from Interrupt ////
{
    switch( m_cycle ){
        case 2: popStack8();                                               break;
        case 3: popStack8(); *m_STATUS = readDataBus() | CONSTANT | BREAK; break;
        case 4: m_op0 = readDataBus(); popStack8();                        break;
        case 5: PC = (readDataBus() << 8) | m_op0;
}   }

void Mcs65Core::RTS() // Return from Subroutine
{
    switch( m_cycle ){
        case 2: popStack8();                        break;
        case 3: m_op0 = readDataBus(); popStack8(); break;
        case 4: PC = (readDataBus() << 8) | m_op0;
}   }

void Mcs65Core::SBC()
{
    uint8_t m = m_op0;
    uint tmp = m_Ac - m - (STATUS(C) ? 0 : 1);
    SET_NEGATIVE( tmp & 0x80 );
    SET_ZERO( (tmp & 0xFF) == 0 );
    SET_OVERFLOW( ((m_Ac ^ tmp) & 0x80) && ((m_Ac ^ m) & 0x80) );

    if( STATUS(D) )
    {
        if( ((m_Ac & 0x0F) - (STATUS(C) ? 0 : 1)) < (m & 0x0F)) tmp -= 6;
        if( tmp > 0x99) tmp -= 0x60;
    }
    SET_CARRY( tmp < 0x100 );
    m_Ac = ( tmp & 0xFF );
}

void Mcs65Core::SEC() { SET_CARRY(1); }
void Mcs65Core::SED() { SET_DECIMAL(1); }
void Mcs65Core::SEI() { SET_INTERRUPT(1); }

void Mcs65Core::STA() { Write( m_opAddr, m_Ac ); }
void Mcs65Core::STX() { Write( m_opAddr, m_rX ); }
void Mcs65Core::STY() { Write( m_opAddr, m_rY ); }

void Mcs65Core::TAX() { m_rX = m_Ac; setNZ( m_Ac ); }
void Mcs65Core::TAY() { m_rY = m_Ac; setNZ( m_Ac ); }

void Mcs65Core::TXA() { m_Ac = m_rX; setNZ( m_Ac ); }
void Mcs65Core::TYA() { m_Ac = m_rY; setNZ( m_Ac ); }

void Mcs65Core::TSX() { m_rX = m_SP; setNZ( m_SP ); }
void Mcs65Core::TXS() { m_SP = m_rX; }

void  Mcs65Core::BXX( uint8_t flag, uint8_t y ) /// +1 cycle same page +2 different page
{
    uint8_t bit = 0;
    switch( flag ) {
        case 0: bit = N; break;
        case 1: bit = O; break;
        case 2: bit = C; break;
        case 3: bit = Z; break;
    }
    uint8_t flagVal = STATUS( bit ) ? 1 : 0;
    if( flagVal == y ) PC = m_op0;
}

void Mcs65Core::runDecoder()
{
    if( m_incPC ) { PC++; m_incPC = false; }
    m_mcu->cyclesDone = 1;
    m_cycle++;

    m_phi2Pin->sheduleState( false, 0 );
    m_phi2Pin->sheduleState( true, m_psStep/2 ); // ClkPin2 High at half cycle
    if( m_syncPin->getOutState() ) m_syncPin->sheduleState( false, m_tSync );

    if( m_cpuState == cpu_RESET )              //Reset proccess (8 cycles)
    {
        switch( m_cycle ){
            case 1:                                      return;
            case 2: m_mcu->extMem->setAddrBus( 0x0001 ); return;
            case 3: m_mcu->extMem->setAddrBus( 0x0100 ); return;
            case 4:                                      return;
            case 5:                                      return;
            case 6: m_mcu->extMem->read( 0XFFFC );       return;
            case 7:{
                PC = readDataBus();
                m_mcu->extMem->read( 0XFFFD );           return;
            }
            case 8: // Reset sequence finished, go fetch first instruction.
            {
                uint32_t data = readDataBus();
                PC += data << 8;
                m_cpuState = cpu_FETCH;
            }
        }
    }

    if( m_cpuState == cpu_DECODE )            // Decode Instruction
    {
        m_ctrlPC   = false;    // Increment PC at execution
        m_cpuState = cpu_READ;
        m_addrMode = a_NON;
        m_addrIndx = 0;
        m_syncPin->sheduleState( true, m_tSync );

        m_IR = readDataBus();
        switch( m_IR ) {                   // Irregular Instructions
            case 0x00:  m_ctrlPC = true; break; // BRK
            case 0x08:  break; // PHP
            case 0x18:  break; // CLC
            case 0x20:  m_addrMode = a_ABS; m_ctrlPC = true; break; // JSR abs Execution controls PC
            case 0x28:  m_ctrlPC = true; break; // PLP
            case 0x38:  break; // SEC
            case 0x40:  m_ctrlPC = true; break; // RTI
            case 0x48:  break; // PHA
            case 0x58:  break; // CLI
            case 0x60:  m_ctrlPC = true; break; // RTS
            case 0x68:  m_ctrlPC = true; break; // PLA
            case 0x78:  break; // SEI
            case 0x88:  break; // DEY
            case 0x8A:  break; // TXA
            case 0x98:  break; // TYA
            case 0x9A:  break; // TXS
            case 0xA8:  break; // TAY
            case 0xAA:  break; // TAX
            case 0xB8:  break; // CLV
            case 0xBA:  break; // TSX
            case 0xC8:  break; // INY
            case 0xCA:  break; // DEX
            case 0xD8:  break; // CLD
            case 0xE8:  break; // INX
            case 0xEA:  break; // NOP
            case 0xF8:  break; // SED
            default:                          // Regular Instructions
            {                                 // OOOAAAGG , O = Opcode, A = Addr mode, G = Group
                m_group =  m_IR & 0b00000011;
                m_Amode = (m_IR & 0b00011100) >> 2;
                m_Ocode = (m_IR & 0b11100000) >> 5;

                if( m_group == 3 ) { m_cpuState = cpu_FETCH; return; }/// Not valid (by now)

                switch( m_Amode ){
                    case 0: {
        if( m_group == 1 ){ m_addrMode = a_IND; m_addrIndx = i_X | i_Z; break; }   // Group 1
                      else{ m_addrMode = a_IMM;                         break; }   // Groups 2, 4
                            }
                    case 1: m_addrMode = a_ABS; m_addrIndx = i_Z;       break;     // All
                    case 2: {
        if( m_group == 1 ){ m_addrMode = a_IMM;                         break; }   // Group 1
        if( m_group == 2 ){ m_addrMode = a_ACC;                         break; }   // Group 2
                            }
                    case 3: m_addrMode = a_ABS;                         break;     // All
                    case 4:
        if( m_group == 1 ){ m_addrMode = a_IND; m_addrIndx = i_Y;       break; }   // Group 1

                    case 5: m_addrMode = a_REL; m_addrIndx = i_X | i_Z; break;     // same
                    case 6:
        if( m_group == 1 ){ m_addrMode = a_ABS; m_addrIndx = i_Y | i_C; m_ctrlPC = true; break; }   // Group 1

                    case 7: m_addrMode = a_ABS; m_addrIndx = i_X | i_C; m_ctrlPC = true; break;     // All
                }
            }
        }
    }
    if( m_cpuState == cpu_READ ) Read( PC+1 );

    if( m_cpuState == cpu_EXEC )             // We are executing Instruction
    {
        m_cpuState = cpu_FETCH; // Default, Ops can change this
        if( !m_ctrlPC ) PC++;

        switch( m_IR ) {                   // Irregular Instruction
            case 0x00: BRK(); break; // BRK
            case 0x08: PHP(); break; // PHP
            case 0x18: CLC(); break; // CLC
            case 0x20: JSR(); break; // JSR abs
            case 0x28: PLP(); break; // PLP
            case 0x38: SEC(); break; // SEC
            case 0x40: RTI(); break; // RTI
            case 0x48: PHA(); break; // PHA
            case 0x58: CLI(); break; // CLI
            case 0x60: RTS(); break; // RTS
            case 0x68: PLA(); break; // PLA
            case 0x78: SEI(); break; // SEI
            case 0x88: DEY(); break; // DEY
            case 0x8A: TXA(); break; // TXA
            case 0x98: TYA(); break; // TYA
            case 0x9A: TXS(); break; // TXS
            case 0xA8: TAY(); break; // TAY
            case 0xAA: TAX(); break; // TAX
            case 0xB8: CLV(); break; // CLV
            case 0xBA: TSX(); break; // TSX
            case 0xC8: INY(); break; // INY
            case 0xCA: DEX(); break; // DEX
            case 0xD8: CLD(); break; // CLD
            case 0xE8: INX(); break; // INX
            case 0xEA: NOP(); break; // NOP
            case 0xF8: SED(); break; // SED
            default:{
                switch( m_group ){               // Regular Instruction
                    case 0:{
                        if( m_Amode == 4 ) // xxy 100 00 conditional branch
                        {
                            uint8_t flag = (m_IR & 0b11000000) >> 6; // 0=N, 1=O, 2=C, 3=Z
                            uint8_t y    = (m_IR & 0b00100000) >> 5;
                            BXX( flag, y ); // BPL BMI BVC BVS BCC BCS BNE BEQ
                        }else{
                            switch( m_Ocode ){
                                case 0: break;
                                case 1: BIT(); break;
                                case 2: //JMP();
                                case 3: JMP(); break;
                                case 4: STY(); break;
                                case 5: LDY(); break;
                                case 6: CPY(); break;
                                case 7: CPX(); break;
                        }   }
                    } break;
                    case 1:{ switch( m_Ocode ){
                            case 0: ORA(); break;
                            case 1: AND(); break;
                            case 2: EOR(); break;
                            case 3: ADC(); break;
                            case 4: STA(); break;
                            case 5: LDA(); break;
                            case 6: CMP(); break;
                            case 7: SBC(); break;
                    } }break;
                    case 2:{ switch( m_Ocode ){
                            case 0: ASL(); break;
                            case 1: ROL(); break;
                            case 2: LSR(); break;
                            case 3: ROR(); break;
                            case 4: STX(); break;
                            case 5: LDX(); break;
                            case 6: DEC(); break;
                            case 7: INC(); break;
                    } }break;
                    case 3: break;
    }   }   }   }

    if( m_cpuState == cpu_FETCH ) // Start m_IR cycle
    {
        m_cycle = 0;
        m_cpuState = cpu_DECODE;
        m_addrMode = a_NON;

        m_mcu->extMem->read( PC ); // Fetch Instruction
    }
    else if( m_cpuState == cpu_WRITE ) m_cpuState = cpu_FETCH;
}
